#include <unity.h>
#include "nfl/nfl_api_client.h"
#include "common/http_fetch.h"

void setUp() {}
void tearDown() {}

// Minimal fixtures using only the fields parseNflSchedule() actually reads -
// field paths/shape confirmed against a real site.api.espn.com response
// (see NFL_SUPPORT_ROADMAP.md), not guessed at.

static const char* JSON_HOME_FAVORED = R"JSON(
{
  "week": {"number": 1},
  "events": [{
    "id": "401872656",
    "date": "2026-09-10T00:20Z",
    "competitions": [{
      "competitors": [
        {"homeAway": "home", "team": {"abbreviation": "SEA"}},
        {"homeAway": "away", "team": {"abbreviation": "NE"}}
      ],
      "odds": [{
        "spread": -3.5,
        "overUnder": 44.5,
        "homeTeamOdds": {"favorite": true},
        "awayTeamOdds": {"favorite": false}
      }]
    }]
  }]
}
)JSON";

void test_parses_week_number() {
    JsonDocument doc;
    deserializeJson(doc, JSON_HOME_FAVORED);
    NflWeekSchedule result = parseNflSchedule(doc);
    TEST_ASSERT_EQUAL_INT(1, result.weekNumber);
}

void test_parses_home_and_away_abbreviations() {
    JsonDocument doc;
    deserializeJson(doc, JSON_HOME_FAVORED);
    NflWeekSchedule result = parseNflSchedule(doc);
    TEST_ASSERT_EQUAL_INT(1, result.count);
    TEST_ASSERT_EQUAL_STRING("SEA", result.games[0].homeAbbr);
    TEST_ASSERT_EQUAL_STRING("NE", result.games[0].awayAbbr);
}

void test_home_favorite_spread_is_positive_magnitude() {
    // ESPN's raw spread is negative when the home team is favored (-3.5) -
    // parseNflSchedule() should store the magnitude only, favorite tracked
    // separately via favoriteAbbr.
    JsonDocument doc;
    deserializeJson(doc, JSON_HOME_FAVORED);
    NflWeekSchedule result = parseNflSchedule(doc);
    TEST_ASSERT_TRUE(result.games[0].hasOdds);
    TEST_ASSERT_EQUAL_STRING("SEA", result.games[0].favoriteAbbr);
    TEST_ASSERT_EQUAL_FLOAT(3.5f, result.games[0].spread);
    TEST_ASSERT_EQUAL_FLOAT(44.5f, result.games[0].overUnder);
}

static const char* JSON_AWAY_FAVORED = R"JSON(
{
  "week": {"number": 3},
  "events": [{
    "id": "1",
    "date": "2026-09-20T17:00Z",
    "competitions": [{
      "competitors": [
        {"homeAway": "home", "team": {"abbreviation": "CHI"}},
        {"homeAway": "away", "team": {"abbreviation": "GB"}}
      ],
      "odds": [{
        "spread": 6.0,
        "overUnder": 41.5,
        "homeTeamOdds": {"favorite": false},
        "awayTeamOdds": {"favorite": true}
      }]
    }]
  }]
}
)JSON";

void test_away_favorite_resolves_correctly() {
    JsonDocument doc;
    deserializeJson(doc, JSON_AWAY_FAVORED);
    NflWeekSchedule result = parseNflSchedule(doc);
    TEST_ASSERT_EQUAL_STRING("GB", result.games[0].favoriteAbbr);
    TEST_ASSERT_EQUAL_FLOAT(6.0f, result.games[0].spread);
}

static const char* JSON_NO_ODDS = R"JSON(
{
  "week": {"number": 1},
  "events": [{
    "id": "2",
    "date": "2026-09-07T20:25Z",
    "competitions": [{
      "competitors": [
        {"homeAway": "home", "team": {"abbreviation": "DAL"}},
        {"homeAway": "away", "team": {"abbreviation": "SF"}}
      ]
    }]
  }]
}
)JSON";

void test_missing_odds_key_sets_has_odds_false() {
    // Confirmed real behavior: odds aren't always posted (e.g. far-out
    // games) - the odds array/key can be entirely absent, not just empty.
    JsonDocument doc;
    deserializeJson(doc, JSON_NO_ODDS);
    NflWeekSchedule result = parseNflSchedule(doc);
    TEST_ASSERT_EQUAL_INT(1, result.count);
    TEST_ASSERT_FALSE(result.games[0].hasOdds);
    TEST_ASSERT_EQUAL_STRING("DAL", result.games[0].homeAbbr);
}

static const char* JSON_TWO_GAMES = R"JSON(
{
  "week": {"number": 1},
  "events": [
    {"id": "1", "date": "d1", "competitions": [{"competitors": [
      {"homeAway": "home", "team": {"abbreviation": "PIT"}},
      {"homeAway": "away", "team": {"abbreviation": "NYJ"}}
    ]}]},
    {"id": "2", "date": "d2", "competitions": [{"competitors": [
      {"homeAway": "home", "team": {"abbreviation": "KC"}},
      {"homeAway": "away", "team": {"abbreviation": "LAC"}}
    ]}]}
  ]
}
)JSON";

void test_multiple_events_all_parsed_in_order() {
    JsonDocument doc;
    deserializeJson(doc, JSON_TWO_GAMES);
    NflWeekSchedule result = parseNflSchedule(doc);
    TEST_ASSERT_EQUAL_INT(2, result.count);
    TEST_ASSERT_EQUAL_STRING("PIT", result.games[0].homeAbbr);
    TEST_ASSERT_EQUAL_STRING("KC", result.games[1].homeAbbr);
}

void test_empty_document_yields_zero_games() {
    JsonDocument doc;
    NflWeekSchedule result = parseNflSchedule(doc);
    TEST_ASSERT_EQUAL_INT(0, result.count);
}

// Shaped like the real ESPN response as of 2026-09-26 (week 3): a finished
// game (no odds any more) first, then an upcoming one with odds, each wrapped
// in the kind of extra fields the real ~280 KB response is mostly made of.
static const char* JSON_REALISTIC_NOISY = R"JSON(
{
  "leagues": [{"id": "28", "name": "National Football League", "calendar": [{"label": "Regular Season", "entries": [{"label": "Week 1", "value": "1"}, {"label": "Week 2", "value": "2"}]}]}],
  "season": {"type": 2, "year": 2026},
  "week": {"number": 3},
  "events": [
    {
      "id": "401872700",
      "uid": "s:20~l:28~e:401872700",
      "date": "2026-09-25T00:15Z",
      "name": "Atlanta Falcons at Green Bay Packers",
      "shortName": "ATL @ GB",
      "status": {"clock": 0.0, "displayClock": "0:00", "period": 4, "type": {"id": "3", "name": "STATUS_FINAL", "state": "post", "completed": true, "description": "Final"}},
      "competitions": [{
        "id": "401872700",
        "venue": {"fullName": "Lambeau Field", "address": {"city": "Green Bay", "state": "WI"}},
        "competitors": [
          {"id": "9", "homeAway": "home", "winner": true, "score": "27", "team": {"id": "9", "abbreviation": "GB", "displayName": "Green Bay Packers", "color": "203731", "logo": "https://a.espncdn.com/i/teamlogos/nfl/500/gb.png"}, "records": [{"name": "overall", "summary": "2-0"}],
           "leaders": [{"name": "passingYards", "leaders": [{"displayValue": "250 YDS", "athlete": {"id": "1", "fullName": "A Quarterback", "links": [{"rel": ["playercard", "desktop", "athlete"], "href": "https://www.espn.com/nfl/player/_/id/1"}]}}]}]},
          {"id": "1", "homeAway": "away", "winner": false, "score": "20", "team": {"id": "1", "abbreviation": "ATL", "displayName": "Atlanta Falcons", "color": "a71930", "logo": "https://a.espncdn.com/i/teamlogos/nfl/500/atl.png"}, "records": [{"name": "overall", "summary": "1-1"}]}
        ],
        "notes": [],
        "broadcasts": [{"market": "national", "names": ["Prime Video"]}]
      }],
      "links": [{"rel": ["summary"], "href": "https://www.espn.com/nfl/game/_/gameId/401872700"}]
    },
    {
      "id": "401872701",
      "date": "2026-09-27T17:00Z",
      "name": "Los Angeles Chargers at Buffalo Bills",
      "status": {"type": {"id": "1", "name": "STATUS_SCHEDULED", "state": "pre", "completed": false}},
      "competitions": [{
        "competitors": [
          {"homeAway": "home", "team": {"abbreviation": "BUF", "displayName": "Buffalo Bills"}, "records": [{"summary": "2-0"}]},
          {"homeAway": "away", "team": {"abbreviation": "LAC", "displayName": "Los Angeles Chargers"}, "records": [{"summary": "1-1"}]}
        ],
        "odds": [{
          "provider": {"id": "58", "name": "ESPN BET"},
          "details": "BUF -7",
          "overUnder": 50.5,
          "spread": -7.0,
          "awayTeamOdds": {"favorite": false, "underdog": true, "moneyLine": 250},
          "homeTeamOdds": {"favorite": true, "underdog": false, "moneyLine": -320}
        }]
      }]
    }
  ]
}
)JSON";

void test_finished_game_is_flagged_and_has_no_odds() {
    JsonDocument doc;
    deserializeJson(doc, JSON_REALISTIC_NOISY, DeserializationOption::NestingLimit(JSON_NESTING_LIMIT));
    NflWeekSchedule result = parseNflSchedule(doc);
    TEST_ASSERT_EQUAL_INT(2, result.count);

    TEST_ASSERT_EQUAL_STRING("ATL", result.games[0].awayAbbr);
    TEST_ASSERT_EQUAL_STRING("GB", result.games[0].homeAbbr);
    TEST_ASSERT_TRUE(result.games[0].isFinal);
    TEST_ASSERT_FALSE(result.games[0].hasOdds);

    TEST_ASSERT_FALSE(result.games[1].isFinal);
    TEST_ASSERT_TRUE(result.games[1].hasOdds);
    TEST_ASSERT_EQUAL_STRING("BUF", result.games[1].favoriteAbbr);
    TEST_ASSERT_EQUAL_FLOAT(7.0f, result.games[1].spread);
    TEST_ASSERT_EQUAL_FLOAT(50.5f, result.games[1].overUnder);
}

void test_missing_status_is_not_final() {
    JsonDocument doc;
    deserializeJson(doc, JSON_HOME_FAVORED);  // has no status block at all
    NflWeekSchedule result = parseNflSchedule(doc);
    TEST_ASSERT_FALSE(result.games[0].isFinal);
}

static void assertSameSchedule(const NflWeekSchedule& a, const NflWeekSchedule& b) {
    TEST_ASSERT_EQUAL_INT(a.weekNumber, b.weekNumber);
    TEST_ASSERT_EQUAL_INT(a.count, b.count);
    for (int i = 0; i < a.count; i++) {
        TEST_ASSERT_EQUAL_STRING(a.games[i].eventId, b.games[i].eventId);
        TEST_ASSERT_EQUAL_STRING(a.games[i].homeAbbr, b.games[i].homeAbbr);
        TEST_ASSERT_EQUAL_STRING(a.games[i].awayAbbr, b.games[i].awayAbbr);
        TEST_ASSERT_EQUAL_STRING(a.games[i].kickoffIso, b.games[i].kickoffIso);
        TEST_ASSERT_EQUAL_STRING(a.games[i].favoriteAbbr, b.games[i].favoriteAbbr);
        TEST_ASSERT_EQUAL(a.games[i].isFinal, b.games[i].isFinal);
        TEST_ASSERT_EQUAL(a.games[i].hasOdds, b.games[i].hasOdds);
        TEST_ASSERT_EQUAL_FLOAT(a.games[i].spread, b.games[i].spread);
        TEST_ASSERT_EQUAL_FLOAT(a.games[i].overUnder, b.games[i].overUnder);
    }
}

// The real bug this guards against: the live response is ~280 KB, the ESP32
// ran out of memory parsing it whole, and half a game reached the screen.
// The filter is what makes the real fetch fit - so it must never drop a field
// parseNflSchedule() reads. Parse every fixture both ways and require identical
// results, rather than trusting the filter matches the parser by eye.
void test_filter_never_changes_what_the_parser_produces() {
    const char* fixtures[] = {
        JSON_HOME_FAVORED, JSON_AWAY_FAVORED, JSON_NO_ODDS,
        JSON_TWO_GAMES, JSON_REALISTIC_NOISY,
    };

    JsonDocument filter;
    buildNflScheduleFilter(filter);

    for (const char* json : fixtures) {
        JsonDocument full;
        deserializeJson(full, json, DeserializationOption::NestingLimit(JSON_NESTING_LIMIT));

        JsonDocument filtered;
        DeserializationError err = deserializeJson(filtered, json, DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(JSON_NESTING_LIMIT));
        TEST_ASSERT_TRUE(err == DeserializationError::Ok);

        assertSameSchedule(parseNflSchedule(full), parseNflSchedule(filtered));
    }
}

// Reproduces the real on-device failure. ArduinoJson 7 defaults to a nesting
// limit of 10, and the live ESPN response nests 15 deep (measured 2026-09-26,
// deepest path events[].competitions[].competitors[].leaders[].leaders[]
// .athlete.links[].rel). Every fixture above nests far shallower, which is why
// nothing caught this before the board did. Skipped/filtered-out values still
// count toward the limit, so the filter alone doesn't help: parsing aborted
// inside the first game, leaving the board with half a matchup.
void test_the_devices_default_nesting_limit_cannot_parse_the_real_response() {
    JsonDocument filter;
    buildNflScheduleFilter(filter);

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, JSON_REALISTIC_NOISY,
        DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(10));
    TEST_ASSERT_TRUE(err == DeserializationError::TooDeep);
}

void test_production_nesting_limit_covers_the_real_response() {
    TEST_ASSERT_TRUE(JSON_NESTING_LIMIT >= 15);
}

void test_filter_actually_drops_the_bulk_of_the_response() {
    JsonDocument filter;
    buildNflScheduleFilter(filter);

    JsonDocument full;
    deserializeJson(full, JSON_REALISTIC_NOISY, DeserializationOption::NestingLimit(JSON_NESTING_LIMIT));

    JsonDocument filtered;
    deserializeJson(filtered, JSON_REALISTIC_NOISY, DeserializationOption::Filter(filter), DeserializationOption::NestingLimit(JSON_NESTING_LIMIT));

    TEST_ASSERT_TRUE(filtered["events"][0]["name"].isNull());
    TEST_ASSERT_TRUE(filtered["events"][0]["competitions"][0]["venue"].isNull());
    TEST_ASSERT_TRUE(filtered["leagues"].isNull());
    // Serialized size of what's retained (JsonDocument::memoryUsage() is
    // deprecated and always 0 in ArduinoJson 7).
    TEST_ASSERT_TRUE(measureJson(filtered) * 2 < measureJson(full));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_parses_week_number);
    RUN_TEST(test_parses_home_and_away_abbreviations);
    RUN_TEST(test_home_favorite_spread_is_positive_magnitude);
    RUN_TEST(test_away_favorite_resolves_correctly);
    RUN_TEST(test_missing_odds_key_sets_has_odds_false);
    RUN_TEST(test_multiple_events_all_parsed_in_order);
    RUN_TEST(test_empty_document_yields_zero_games);
    RUN_TEST(test_finished_game_is_flagged_and_has_no_odds);
    RUN_TEST(test_missing_status_is_not_final);
    RUN_TEST(test_filter_never_changes_what_the_parser_produces);
    RUN_TEST(test_the_devices_default_nesting_limit_cannot_parse_the_real_response);
    RUN_TEST(test_production_nesting_limit_covers_the_real_response);
    RUN_TEST(test_filter_actually_drops_the_bulk_of_the_response);

    return UNITY_END();
}
