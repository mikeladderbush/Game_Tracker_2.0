#pragma once
#include <ArduinoJson.h>

// Confirmed via a live regular-season-week response (2026-08, week 1):
// odds come embedded directly in the scoreboard response
// (competitions[0].odds[]), so no separate per-game odds call is needed at
// all - this resolves NFL_SUPPORT_ROADMAP.md steps 1/2. One game per
// NflMatchup, combining schedule + odds since they arrive together.
struct NflMatchup {
    char eventId[16] = {0};
    char homeAbbr[4] = {0};
    char awayAbbr[4] = {0};
    char kickoffIso[24] = {0};   // raw ISO datetime from the API, e.g. "2026-09-10T00:20Z"

    bool isFinal = false;        // game already played - ESPN drops its odds afterwards
    bool hasOdds = false;
    char favoriteAbbr[4] = {0};
    float spread = 0.0f;         // magnitude only (always shown as "-X.X" next to the favorite)
    float overUnder = 0.0f;
};

// Up to ~16 games/week.
struct NflWeekSchedule {
    NflMatchup games[16];
    int count = 0;
    int weekNumber = 0;
};

// GET site.api.espn.com/apis/site/v2/sports/football/nfl/scoreboard
//     [?week={week}&seasontype={seasonType}&dates={year}]
// seasonType: 1=preseason, 2=regular, 3=postseason.
//
// week <= 0 (the default) omits the query params entirely and lets ESPN
// resolve "current week" server-side - confirmed live: a bare request
// returns whatever week is actually happening right now (preseason,
// regular season, or postseason, correctly). Deliberately not replicated
// locally - the NFL's week boundaries shift by a few days every season, so
// a hardcoded date table would need yearly upkeep and ESPN already solves
// this correctly. Pass an explicit week for a future "browse other weeks"
// feature; nothing needs that yet.
JsonDocument fetchNflWeekScheduleJson(int week = 0, int seasonType = 0, int year = 0);

// The full scoreboard response is ~280 KB (confirmed live, 2026-09-26) - far
// more than the ESP32 can hold as a parsed tree. This fills in an ArduinoJson
// filter naming only the fields parseNflSchedule() reads, so the fetch keeps
// a few KB instead. Kept next to the parser so the two can't drift apart.
void buildNflScheduleFilter(JsonDocument& filter);

// Parses schedule + embedded odds together. A game with no odds posted yet
// (or no market for that game) just gets hasOdds=false - not treated as an
// error.
NflWeekSchedule parseNflSchedule(const JsonDocument& doc);
