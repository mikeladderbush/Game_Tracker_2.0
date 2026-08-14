#include "nfl_api_client.h"
#include "../common/http_fetch.h"

static const char* NFL_SCOREBOARD_BASE =
    "https://site.api.espn.com/apis/site/v2/sports/football/nfl/scoreboard";
static const char* NFL_ODDS_BASE =
    "https://sports.core.api.espn.com/v2/sports/football/leagues/nfl/events";

JsonDocument fetchNflWeekScheduleJson(int week, int seasonType, int year) {
    JsonDocument doc;
    String url = String(NFL_SCOREBOARD_BASE) + "?week=" + week +
                 "&seasontype=" + seasonType + "&dates=" + year;
    fetchJsonPlain(url.c_str(), doc);
    return doc;
}

NflWeekSchedule parseNflSchedule(const JsonDocument& doc) {
    // TODO: see nfl_api_client.h - response shape for a real regular-season
    // week hasn't been confirmed yet, so this deliberately doesn't guess at
    // field paths. Once confirmed, mirror nba/nba_api_client.cpp's
    // parseGame() pattern: JsonArrayConst events = doc["events"]...
    (void)doc;
    NflWeekSchedule result;
    return result;
}

JsonDocument fetchNflOddsJson(const char* eventId) {
    JsonDocument doc;
    String url = String(NFL_ODDS_BASE) + "/" + eventId +
                 "/competitions/" + eventId + "/odds";
    fetchJsonSecure(url.c_str(), nullptr, doc);
    return doc;
}

void parseNflOdds(const JsonDocument& doc, NflMatchup& matchup) {
    // TODO: see nfl_api_client.h for confirmed field names. Left unfilled
    // (matchup.hasOdds stays false) rather than guessing at exact array/
    // object nesting not yet verified against a live response.
    (void)doc;
    (void)matchup;
}
