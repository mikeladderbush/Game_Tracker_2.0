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
//     ?week={week}&seasontype={seasonType}&dates={year}
// seasonType: 1=preseason, 2=regular, 3=postseason.
JsonDocument fetchNflWeekScheduleJson(int week, int seasonType, int year);

// Parses schedule + embedded odds together. A game with no odds posted yet
// (or no market for that game) just gets hasOdds=false - not treated as an
// error.
NflWeekSchedule parseNflSchedule(const JsonDocument& doc);
