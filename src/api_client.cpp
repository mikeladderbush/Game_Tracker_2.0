#include "api_client.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

static const char* NBA_SCOREBOARD_URL =
    "https://cdn.nba.com/static/json/liveData/scoreboard/todaysScoreboard_00.json";
static const char* BALLDONTLIE_BASE = "https://api.balldontlie.io/v1/games";
static const char* BALLDONTLIE_TOKEN = "7b02b2a9-0b96-4f6f-9ab1-1b14f14abb9f";
static const bool DAY_LIGHT_SAVINGS = true;

// Teams have different names and ids across APIs and within the same API at times. This list consolidates them.
struct TeamIdEntry { const char* name; int id; };
static const TeamIdEntry TEAM_IDS[] = {
    {"Hawks",1},{"Celtics",2},{"Nets",3},{"Hornets",4},{"Bulls",5},
    {"Cavaliers",6},{"Mavericks",7},{"Nuggets",8},{"Pistons",9},{"Warriors",10},
    {"Rockets",11},{"Pacers",12},{"Clippers",13},{"Lakers",14},{"Grizzlies",15},
    {"Heat",16},{"Bucks",17},{"Timberwolves",18},{"Pelicans",19},{"Knicks",20},
    {"Thunder",21},{"Magic",22},{"Sixers",23},{"Suns",24},{"Blazers",25},
    {"Kings",26},{"Spurs",27},{"Raptors",28},{"Jazz",29},{"Wizards",30},
};

static int teamNameToId(const char* name) {
    for (auto& entry : TEAM_IDS) {
        if (strcmp(entry.name, name) == 0) return entry.id;
    }
    return -1;
}

bool syncTime() {
    long gmtOffsetSec = -5 * 3600 + (DAY_LIGHT_SAVINGS ? 3600 : 0);
    configTime(gmtOffsetSec, 0, "time.cloudflare.com");

    struct tm timeinfo;
    for (int attempt = 0; attempt < 3; attempt++) {
        if (getLocalTime(&timeinfo, 5000)) return true;
        Serial.println("NTP sync failed, retrying...");
        delay(2000);
    }
    return false;
}

String getCurrentDate() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 100)) return "0000-00-00";
    char buf[11];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d",
              timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday);
    return String(buf);
}

String convertUtcToEst(const String& timeStrHHMM) {
    int colonIdx = timeStrHHMM.indexOf(':');
    if (colonIdx < 0) return timeStrHHMM;

    int hours = timeStrHHMM.substring(0, colonIdx).toInt();
    String minutes = timeStrHHMM.substring(colonIdx + 1);

    int offset = DAY_LIGHT_SAVINGS ? -4 : -5;
    hours = ((hours + offset) % 24 + 24) % 24;

    const char* period = (hours < 12) ? "AM" : "PM";
    int hours12 = hours % 12;
    if (hours12 == 0) hours12 = 12;

    char buf[16];
    snprintf(buf, sizeof(buf), "%d:%s %s", hours12, minutes.c_str(), period);
    return String(buf);
}

FetchResult fetchGame(const char* teamName) {
    FetchResult result;

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    if (!http.begin(client, NBA_SCOREBOARD_URL)) {
        Serial.println("fetchGame: http.begin failed");
        return result;
    }
    http.useHTTP10(true);
    http.addHeader("User-Agent", "Mozilla/5.0 (compatible; ESP32)");

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("fetchGame: GET failed, code %d\n", httpCode);
        http.end();
        return result;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, http.getStream());
    http.end();

    if (err) {
        Serial.printf("fetchGame: JSON parse error: %s\n", err.c_str());
        return result;
    }

    JsonArray games = doc["scoreboard"]["games"].as<JsonArray>();
    for (JsonObject game : games) {
        JsonObject home = game["homeTeam"];
        JsonObject away = game["awayTeam"];
        const char* homeTeam = home["teamName"] | "";
        const char* awayTeam = away["teamName"] | "";

        if (strcmp(teamName, homeTeam) != 0 && strcmp(teamName, awayTeam) != 0) {
            continue;
        }

        int homeScoreRaw = home["score"] | 0;
        int awayScoreRaw = away["score"] | 0;
        int period = game["period"] | 0;
        int gameStatus = game["gameStatus"] | 0;
        const char* gameClock = game["gameClock"] | "";
        const char* gameTimeText = game["gameStatusText"] | "";

        result.found = true;
        result.gameStatus = gameStatus;
        result.period = period;
        strlcpy(result.gameClock, gameClock, sizeof(result.gameClock));
        strlcpy(result.gameTimeText, gameTimeText, sizeof(result.gameTimeText));

        if (strcmp(homeTeam, teamName) == 0) {
            result.homeScore = homeScoreRaw;
            result.awayScore = awayScoreRaw;
            strlcpy(result.opponent, awayTeam, sizeof(result.opponent));
        } else {
            result.homeScore = awayScoreRaw;
            result.awayScore = homeScoreRaw;
            strlcpy(result.opponent, homeTeam, sizeof(result.opponent));
        }
        return result;
    }

    return result;
}

NextGameResult getNextGame(const char* teamName) {
    NextGameResult result;

    int teamId = teamNameToId(teamName);
    String startDate = getCurrentDate();

    String url = String(BALLDONTLIE_BASE) + "?team_ids[]=" + teamId + "&start_date=" + startDate;

    for (int attempt = 0; attempt < 5; attempt++) {
        WiFiClientSecure client;
        client.setInsecure();

        HTTPClient http;
        if (!http.begin(client, url)) {
            delay(1000);
            continue;
        }
        http.useHTTP10(true);
        http.addHeader("Authorization", String("Bearer ") + BALLDONTLIE_TOKEN);

        int httpCode = http.GET();
        if (httpCode != HTTP_CODE_OK) {
            http.end();
            delay(1000);
            continue;
        }

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, http.getStream());
        http.end();

        if (err) {
            Serial.printf("getNextGame: JSON parse error: %s\n", err.c_str());
            delay(1000);
            continue;
        }

        JsonArray games = doc["data"].as<JsonArray>();
        if (games.isNull() || games.size() == 0) {
            return result;
        }

        JsonObject game = games[0];
        const char* opponentName = game["visitor_team"]["full_name"] | "";
        const char* teamFullName = game["home_team"]["full_name"] | "";
        const char* dateStr = game["date"] | "";
        const char* datetimeStr = game["datetime"] | "";

        String hhmm = String(datetimeStr).substring(11, 16);

        result.found = true;
        strlcpy(result.dateStr, dateStr, sizeof(result.dateStr));
        strlcpy(result.teamFullName, teamFullName, sizeof(result.teamFullName));
        strlcpy(result.opponentFullName, opponentName, sizeof(result.opponentFullName));
        convertUtcToEst(hhmm).toCharArray(result.timeStr, sizeof(result.timeStr));

        return result;
    }

    return result;
}

int clockStrToSecs(const String& clockStr) {
    if (clockStr.length() == 0) return -1;

    if (clockStr.startsWith("PT")) {
        String s = clockStr.substring(2);
        int minutes = 0, seconds = 0;

        int mIdx = s.indexOf('M');
        String rest = s;
        if (mIdx >= 0) {
            minutes = s.substring(0, mIdx).toInt();
            rest = s.substring(mIdx + 1);
        }

        int sIdx = rest.indexOf('S');
        if (sIdx >= 0) {
            seconds = (int)rest.substring(0, sIdx).toFloat();
        }

        return minutes * 60 + seconds;
    }

    int colonIdx = clockStr.indexOf(':');
    if (colonIdx >= 0) {
        String minutesPart = clockStr.substring(0, colonIdx);
        String secondsPart = clockStr.substring(colonIdx + 1);

        int spaceIdx = secondsPart.indexOf(' ');
        if (spaceIdx >= 0) secondsPart = secondsPart.substring(0, spaceIdx);

        int dotIdx = secondsPart.indexOf('.');
        if (dotIdx >= 0) secondsPart = secondsPart.substring(0, dotIdx);

        int minutes = minutesPart.length() ? minutesPart.toInt() : 0;
        int seconds = secondsPart.length() ? secondsPart.toInt() : 0;
        return minutes * 60 + seconds;
    }

    return -1;
}

String secsToMMSS(int totalSeconds) {
    if (totalSeconds < 0) totalSeconds = 0;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);
    return String(buf);
}