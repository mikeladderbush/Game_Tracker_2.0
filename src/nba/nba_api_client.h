#pragma once
#include <ArduinoJson.h>
#include "../common/time_formatting.h"
#include "nba_team_ids.h"

// Results collected from the live game API are translated from JSON into this struct.
struct FetchResult {
    bool found = false;
    int homeScore = 0;
    int awayScore = 0;
    char opponent[32] = {0};
    char gameClock[16] = {0};
    char gameTimeText[32] = {0};
    int gameStatus = 0;
    int period = 0;
};

// Results for future scheduled games use this struct.
struct NextGameResult {
    bool found = false;
    char dateStr[16] = {0};
    char timeStr[16] = {0};
    char teamFullName[32] = {0};
    char opponentFullName[32] = {0};
};

// Each endpoint is split into a network fetch (returns the raw response as a
// JsonDocument, or an empty one on failure) and a pure parse (JsonDocument ->
// result struct, no network/HTTPClient involved). The parse half has no
// ESP32-specific dependency and can be driven natively in tests with a
// JsonDocument built from a fixture string - no mock HTTP server needed.
// fetchGame()/getNextGame() remain as fetch+parse convenience wrappers for
// normal (non-test) callers.
JsonDocument fetchScoreboardJson();
FetchResult parseGame(const JsonDocument& doc, const char* teamName);
FetchResult fetchGame(const char* teamName);

JsonDocument fetchNextGameJson(const char* teamName);
NextGameResult parseNextGame(const JsonDocument& doc);
NextGameResult getNextGame(const char* teamName);
