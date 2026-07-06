#include <Arduino.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "display.h"
#include "control_server.h"
#include "api_client.h"
#include "game_state.h"
#include "draw_tools.h"
#include "nba_teams.h"
#include "secrets.h"

static GameStateMachine gameStateMachine;
static const NbaTeam* selectedTeam = nullptr;

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

        gameStateMachine.update();
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

static void renderTask(void* pv) {
    for (;;) {
        matrix.fillScreen(0);

        if (gameStateMachine.isInGame()) {
            GameFrame frame = gameStateMachine.currentFrame();
            const NbaTeam* opp = teamFromString(frame.opponent);

            if (selectedTeam) drawLogo(*selectedTeam, 0, 0, 0);
            if (opp) drawLogo(*opp, 0, 0, 1);
            drawScore(frame.homeScore, frame.awayScore);
            drawGameClock(frame.clock);
            if (frame.period >= 1 && frame.period <= 4) drawQuarter(frame.period);
        } else {
            ScheduledGame sched = gameStateMachine.scheduledGame();
            const NbaTeam* opp = teamFromString(sched.opponentFullName);

            if (selectedTeam && opp) {
                drawFutureGame(sched.dateStr, sched.timeStr, *selectedTeam, *opp);
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

    bool menuActive = true;
    while (menuActive) {
        pollControlServer();

        if (appInput.teamPending) {
            selectedTeam = teamFromAbbr(appInput.team);
            appInput.teamPending = false;
            if (selectedTeam) menuActive = false;
        }
        if (!appInput.powerOn) {
            ESP.restart();
        }

        matrix.fillScreen(0);
        drawCityMenu();
        matrix.show();
        delay(50);
    }

    gameStateMachine.begin(selectedTeam->teamName);

    xTaskCreatePinnedToCore(renderTask, "render", 8192, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(stateTask, "state", 8192, nullptr, 2, nullptr, 0);
}

void loop() {
}