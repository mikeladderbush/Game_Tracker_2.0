#include "nba_api_client.h"
#include "../common/http_fetch.h"
#include "../common/ntp_time.h"
#include "../secrets.h"
#include <string.h>

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
