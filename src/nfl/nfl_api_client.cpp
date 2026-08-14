#include "nfl_api_client.h"
#include "../common/http_fetch.h"

#ifndef TEST_SERVER
#define TEST_SERVER 1
#endif

#if TEST_SERVER
// Update the IP to match whatever machine is running TestServer.py, same as
// nba/nba_api_client.cpp's NBA_SCOREBOARD_URL.
static const char* NFL_TEST_SERVER_URL = "http://192.168.1.165:4999/fake_nfl_week";
#else
static const char* NFL_SCOREBOARD_BASE =
    "https://site.api.espn.com/apis/site/v2/sports/football/nfl/scoreboard";
#endif

JsonDocument fetchNflWeekScheduleJson(int week, int seasonType, int year) {
    JsonDocument doc;
#if TEST_SERVER
    (void)week; (void)seasonType; (void)year;  // fixture ignores these, always returns the same week
    fetchJsonPlain(NFL_TEST_SERVER_URL, doc);
#else
    String url = String(NFL_SCOREBOARD_BASE) + "?week=" + week +
                 "&seasontype=" + seasonType + "&dates=" + year;
    fetchJsonPlain(url.c_str(), doc);
#endif
    return doc;
}
