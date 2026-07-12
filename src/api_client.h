#pragma once
#include <Arduino.h>
#include "api_formatting.h"

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

bool syncTime();
String getCurrentDate();
String convertUtcToEst(const String& timeStrHHMM);
FetchResult fetchGame(const char* teamName);
NextGameResult getNextGame(const char* teamName);
// clockStrToSecs, secsToMMSS, teamNameToId now live in api_formatting.h
// (included above) - re-exposed here unchanged so existing callers don't
// need to change their #include.