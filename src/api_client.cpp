#include "api_client.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "secrets.h"

#ifndef TEST_SERVER
#define TEST_SERVER 1
#endif

#if TEST_SERVER
static const char* NBA_SCOREBOARD_URL = "http://192.168.1.165:4999/fake_clock";
#else
static const char* NBA_SCOREBOARD_URL =
    "https://cdn.nba.com/static/json/liveData/scoreboard/todaysScoreboard_00.json";
#endif
static const char* BALLDONTLIE_BASE = "https://api.balldontlie.io/v1/games";

bool syncTime() {
    // TZ string encodes US Eastern + DST rules (2nd Sun Mar - 1st Sun Nov),
    // so tm_isdst is computed correctly by the library going forward —
    // no more hardcoded seasonal guess.
    configTzTime("EST5EDT,M3.2.0,M11.1.0", "time.cloudflare.com");

    struct tm timeinfo;
    for (int attempt = 0; attempt < 3; attempt++) {
        if (getLocalTime(&timeinfo, 5000)) return true;
        Serial.println("NTP sync failed, retrying...");
        delay(2000);
    }
    return false;
}

static bool isDaylightSavingNow() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo, 100)) return false;  // fallback: standard time
    return timeinfo.tm_isdst > 0;
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
    int offset = isDaylightSavingNow() ? -4 : -5;
    return convertUtcToEstWithOffset(timeStrHHMM, offset);
}

// Shared HTTP-GET-then-parse-JSON step used by every endpoint below. Only
// knows about a URL, which client to speak plaintext/TLS with, and an
// optional auth header - nothing NBA- or team-specific - so it's the seam to
// extend if another API/data source gets added later.
static bool fetchJson(HTTPClient& http, const char* authHeader, JsonDocument& outDoc) {
    http.useHTTP10(true);
    if (authHeader) {
        http.addHeader("Authorization", authHeader);
    } else {
        http.addHeader("User-Agent", "Mozilla/5.0 (compatible; ESP32)");
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("fetchJson: GET failed, code %d\n", httpCode);
        http.end();
        return false;
    }

    DeserializationError err = deserializeJson(outDoc, http.getStream());
    http.end();
    if (err) {
        Serial.printf("fetchJson: JSON parse error: %s\n", err.c_str());
        return false;
    }
    return true;
}

static bool fetchJsonPlain(const char* url, JsonDocument& outDoc) {
    WiFiClient client;
    HTTPClient http;
    if (!http.begin(client, url)) {
        Serial.println("fetchJson: http.begin failed");
        return false;
    }
    return fetchJson(http, nullptr, outDoc);
}

static bool fetchJsonSecure(const char* url, const char* authHeader, JsonDocument& outDoc) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    if (!http.begin(client, url)) {
        Serial.println("fetchJson: http.begin failed");
        return false;
    }
    return fetchJson(http, authHeader, outDoc);
}

JsonDocument fetchScoreboardJson() {
    JsonDocument doc;
#if TEST_SERVER
    fetchJsonPlain(NBA_SCOREBOARD_URL, doc);
#else
    fetchJsonSecure(NBA_SCOREBOARD_URL, nullptr, doc);
#endif
    return doc;
}

FetchResult parseGame(const JsonDocument& doc, const char* teamName) {
    FetchResult result;

    JsonArrayConst games = doc["scoreboard"]["games"].as<JsonArrayConst>();
    for (JsonObjectConst game : games) {
        JsonObjectConst home = game["homeTeam"];
        JsonObjectConst away = game["awayTeam"];
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

FetchResult fetchGame(const char* teamName) {
    JsonDocument doc = fetchScoreboardJson();
    return parseGame(doc, teamName);
}

JsonDocument fetchNextGameJson(const char* teamName) {
    JsonDocument doc;

    int teamId = teamNameToId(teamName);
    String startDate = getCurrentDate();
    String url = String(BALLDONTLIE_BASE) + "?team_ids[]=" + teamId + "&start_date=" + startDate;
    String authHeader = String("Bearer ") + BALLDONTLIE_TOKEN;

    for (int attempt = 0; attempt < 5; attempt++) {
        if (fetchJsonSecure(url.c_str(), authHeader.c_str(), doc)) {
            return doc;
        }
        delay(1000);
    }

    return doc;
}

NextGameResult parseNextGame(const JsonDocument& doc) {
    NextGameResult result;
    JsonArrayConst games = doc["data"].as<JsonArrayConst>();
    if (games.isNull() || games.size() == 0) {
        return result;
    }

    JsonObjectConst game = games[0];
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

NextGameResult getNextGame(const char* teamName) {
    JsonDocument doc = fetchNextGameJson(teamName);
    return parseNextGame(doc);
}

// clockStrToSecs, secsToMMSS, teamNameToId moved to api_formatting.cpp -
// pure logic with no network/ESP32 dependency, kept separate so it can be
// compiled and unit tested natively.