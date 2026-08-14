#include <unity.h>
#include "nfl/nfl_api_client.h"

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

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_parses_week_number);
    RUN_TEST(test_parses_home_and_away_abbreviations);
    RUN_TEST(test_home_favorite_spread_is_positive_magnitude);
    RUN_TEST(test_away_favorite_resolves_correctly);
    RUN_TEST(test_missing_odds_key_sets_has_odds_false);
    RUN_TEST(test_multiple_events_all_parsed_in_order);
    RUN_TEST(test_empty_document_yields_zero_games);

    return UNITY_END();
}
