#pragma once
#include <ArduinoJson.h>

// Draft data shape for NFL_SUPPORT_ROADMAP.md step 7 ("design a data
// structure to hold a full week's matchups + odds"). Combines schedule +
// odds into one struct per game so the (not-yet-designed, step 8) display
// screen just reads one array. Field choices trace back to the "Confirmed
// real JSON fields" section of the roadmap - adjust once a regular-season
// (not preseason) response is checked, per the roadmap's still-open question
// about whether odds ever come back embedded in the schedule response.
struct NflMatchup {
    char eventId[16] = {0};      // needed to fetch this game's odds separately, if step 1/2 concludes that's required
    char homeAbbr[4] = {0};
    char awayAbbr[4] = {0};
    char kickoffIso[24] = {0};   // raw ISO datetime from the API - display formatting is step 8, not decided yet

    bool hasOdds = false;
    char favoriteAbbr[4] = {0};
    float spread = 0.0f;
    float overUnder = 0.0f;
};

// Up to ~16 games/week per the roadmap's own estimate.
struct NflWeekSchedule {
    NflMatchup games[16];
    int count = 0;
};

// GET site.api.espn.com/apis/site/v2/sports/football/nfl/scoreboard
//     ?week={week}&seasontype={seasonType}&dates={year}
// seasonType: 1=preseason, 2=regular, 3=postseason (per NFL_SUPPORT_ROADMAP.md).
JsonDocument fetchNflWeekScheduleJson(int week, int seasonType, int year);

// TODO (roadmap step 4): parse fetchNflWeekScheduleJson()'s response into
// the struct above. Not yet implemented - the exact response shape for a
// regular-season week (vs. the one preseason/Hall-of-Fame-game response
// checked so far) hasn't been confirmed.
NflWeekSchedule parseNflSchedule(const JsonDocument& doc);

// GET sports.core.api.espn.com/v2/sports/football/leagues/nfl/events/{eventId}
//     /competitions/{eventId}/odds
// Confirmed working (see roadmap) - only needed at all if step 1/2 concludes
// odds require a separate per-game call rather than coming embedded.
JsonDocument fetchNflOddsJson(const char* eventId);

// TODO (roadmap step 6): pull favorite/spread/over-under out of
// fetchNflOddsJson()'s response and fill them into the matching NflMatchup.
// Confirmed field names (see roadmap): awayTeamOdds.favorite /
// homeTeamOdds.underdog (bool), spread (numeric), overUnder (numeric).
void parseNflOdds(const JsonDocument& doc, NflMatchup& matchup);
