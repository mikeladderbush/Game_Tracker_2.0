#include <Arduino.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <string.h>
#include "common/display.h"
#include "common/draw_tools.h"
#include "common/ntp_time.h"
#include "nba/control_server.h"
#include "nba/game_state.h"
#include "nba/nba_menu.h"
#include "nba/nba_teams.h"
#include "nfl/nfl_menu.h"
#include "nfl/nfl_state.h"
#include "secrets.h"

static GameStateMachine gameStateMachine;
static NflScheduleState nflState;
static const TeamSprite* selectedTeam = nullptr;

static bool isNflMode() {
    return strcmp(appInput.sport, "NFL") == 0;
}

// Shared by the boot-time menu loop and stateTask, so switching sport works
// identically whether it's the very first selection or a live change later.
static void applyPendingSelection() {
    if (appInput.sportPending) {
        appInput.sportPending = false;
        if (isNflMode()) {
            nflState.begin();
        }
    }
    if (appInput.teamPending) {
        const TeamSprite* team = nbaTeamFromAbbr(appInput.team);
        appInput.teamPending = false;
        if (team) {
            selectedTeam = team;
            gameStateMachine.begin(selectedTeam->teamName);
        }
    }
}

static void stateTask(void* pv) {
    unsigned long lastSync = millis();
    const unsigned long SYNC_INTERVAL_MS = 3600000UL;
    const unsigned long FORCE_RESET_MS   = 14400000UL;

    for (;;) {
        if (millis() - lastSync > SYNC_INTERVAL_MS) {
            syncTime();
            lastSync = millis();
        }
        if (millis() - lastSync > FORCE_RESET_MS) {
            ESP.restart();
        }

        pollControlServer();
        if (!appInput.powerOn) {
            ESP.restart();
        }

        applyPendingSelection();

        if (isNflMode()) {
            nflState.update();
        } else if (selectedTeam) {
            gameStateMachine.update();
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void renderTask(void* pv) {
    for (;;) {
        matrix.fillScreen(0);

        if (isNflMode()) {
            drawScheduleList(nflState.schedule(), nflState.currentPage());
        } else if (selectedTeam) {
            if (gameStateMachine.isInGame()) {
                GameFrame frame = gameStateMachine.currentFrame();
                const TeamSprite* opp = nbaTeamFromString(frame.opponent);

                drawLogo(*selectedTeam, 0, 0, 0);
                if (opp) drawLogo(*opp, 0, 0, 1);
                drawScore(frame.homeScore, frame.awayScore);
                drawGameClock(frame.clock);
                if (frame.period >= 1) drawQuarter(frame.period);
            } else {
                ScheduledGame sched = gameStateMachine.scheduledGame();
                const TeamSprite* opp = nbaTeamFromString(sched.opponentFullName);

                if (opp) {
                    drawFutureGame(sched.dateStr, sched.timeStr, *selectedTeam, *opp);
                }
            }
        }

        matrix.show();
        vTaskDelay(pdMS_TO_TICKS(16));
    }
}

void setup() {
    Serial.begin(115200);
    initDisplay();

    Serial.println("Connecting to WiFi...");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (millis() - startAttempt > 15000) {
            Serial.println("\nWiFi failed to connect after 15s, restarting...");
            ESP.restart();
        }
    }
    Serial.print("Connected, IP: ");
    Serial.println(WiFi.localIP());

    if (!syncTime()) {
        Serial.println("Time sync failed - date/time will be unavailable");
    }

    beginControlServer();

    bool hibernating = true;
    while (hibernating) {
        pollControlServer();
        if (appInput.powerOn) hibernating = false;
        delay(50);
    }

    // Wait for either an NBA team pick or a switch to NFL mode (which needs
    // no team - it shows the whole week's games). Still shows the NBA city
    // menu while waiting, since that's the default/most common path.
    bool menuActive = true;
    while (menuActive) {
        pollControlServer();

        if (appInput.sportPending && strcmp(appInput.sport, "NFL") == 0) {
            appInput.sportPending = false;
            nflState.begin();
            menuActive = false;
        }
        if (appInput.teamPending) {
            selectedTeam = nbaTeamFromAbbr(appInput.team);
            appInput.teamPending = false;
            appInput.sportPending = false;  // already NBA by construction, nothing to re-apply
            if (selectedTeam) {
                gameStateMachine.begin(selectedTeam->teamName);
                menuActive = false;
            }
        }
        if (!appInput.powerOn) {
            ESP.restart();
        }

        matrix.fillScreen(0);
        drawCityMenu();
        matrix.show();
        delay(50);
    }

    xTaskCreatePinnedToCore(renderTask, "render", 8192, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(stateTask, "state", 8192, nullptr, 2, nullptr, 0);
}

void loop() {
}
