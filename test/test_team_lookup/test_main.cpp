#include <unity.h>
#include "nba/nba_teams.h"
#include "nba/nba_team_ids.h"

void setUp() {}
void tearDown() {}

// --- nbaTeamFromString ---

void test_from_string_lowercase_nickname() {
    TEST_ASSERT_EQUAL_PTR(&celtics, nbaTeamFromString("celtics"));
}

void test_from_string_mixed_case_full_name_last_word_match() {
    TEST_ASSERT_EQUAL_PTR(&celtics, nbaTeamFromString("Boston Celtics"));
}

void test_from_string_76ers_special_case() {
    TEST_ASSERT_EQUAL_PTR(&sixers, nbaTeamFromString("76ers"));
}

void test_from_string_multiword_franchise_last_word() {
    TEST_ASSERT_EQUAL_PTR(&trailBlazers, nbaTeamFromString("Trail Blazers"));
}

void test_from_string_multiword_full_name_last_word() {
    TEST_ASSERT_EQUAL_PTR(&trailBlazers, nbaTeamFromString("Portland Trail Blazers"));
}

void test_from_string_no_match_returns_null() {
    TEST_ASSERT_NULL(nbaTeamFromString("Globetrotters"));
}

void test_from_string_empty_returns_null() {
    TEST_ASSERT_NULL(nbaTeamFromString(""));
}

// --- nbaTeamFromAbbr ---

void test_from_abbr_known_uppercase() {
    TEST_ASSERT_EQUAL_PTR(&celtics, nbaTeamFromAbbr("BOS"));
}

void test_from_abbr_unknown_returns_null() {
    TEST_ASSERT_NULL(nbaTeamFromAbbr("ZZZ"));
}

void test_from_abbr_is_case_sensitive() {
    // Documents current behavior (raw strcmp, not strcasecmp). Every caller
    // today uppercases first (control_server.cpp) - this is a landmine if a
    // second entry point (e.g. a future companion app) skips that step.
    TEST_ASSERT_NULL(nbaTeamFromAbbr("bos"));
}

void test_from_abbr_empty_returns_null() {
    TEST_ASSERT_NULL(nbaTeamFromAbbr(""));
}

// --- teamNameToId (balldontlie's numeric ID, needed for getNextGame()'s
// request URL - NBA/balldontlie-specific, not a generic team lookup) ---

void test_team_name_to_id_known_team() {
    TEST_ASSERT_EQUAL_INT(14, teamNameToId("Lakers"));
}

void test_team_name_to_id_unknown_team() {
    TEST_ASSERT_EQUAL_INT(-1, teamNameToId("Fakers"));
}

void test_team_name_to_id_is_case_sensitive() {
    // Documents current behavior (raw strcmp) - callers must pass canonical
    // case. Flag if this should become case-insensitive.
    TEST_ASSERT_EQUAL_INT(-1, teamNameToId("lakers"));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_from_string_lowercase_nickname);
    RUN_TEST(test_from_string_mixed_case_full_name_last_word_match);
    RUN_TEST(test_from_string_76ers_special_case);
    RUN_TEST(test_from_string_multiword_franchise_last_word);
    RUN_TEST(test_from_string_multiword_full_name_last_word);
    RUN_TEST(test_from_string_no_match_returns_null);
    RUN_TEST(test_from_string_empty_returns_null);

    RUN_TEST(test_from_abbr_known_uppercase);
    RUN_TEST(test_from_abbr_unknown_returns_null);
    RUN_TEST(test_from_abbr_is_case_sensitive);
    RUN_TEST(test_from_abbr_empty_returns_null);

    RUN_TEST(test_team_name_to_id_known_team);
    RUN_TEST(test_team_name_to_id_unknown_team);
    RUN_TEST(test_team_name_to_id_is_case_sensitive);

    return UNITY_END();
}
