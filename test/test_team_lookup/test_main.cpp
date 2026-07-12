#include <unity.h>
#include "nba_teams.h"

void setUp() {}
void tearDown() {}

// --- teamFromString ---

void test_from_string_lowercase_nickname() {
    TEST_ASSERT_EQUAL_PTR(&celtics, teamFromString("celtics"));
}

void test_from_string_mixed_case_full_name_last_word_match() {
    TEST_ASSERT_EQUAL_PTR(&celtics, teamFromString("Boston Celtics"));
}

void test_from_string_76ers_special_case() {
    TEST_ASSERT_EQUAL_PTR(&sixers, teamFromString("76ers"));
}

void test_from_string_multiword_franchise_last_word() {
    TEST_ASSERT_EQUAL_PTR(&trailBlazers, teamFromString("Trail Blazers"));
}

void test_from_string_multiword_full_name_last_word() {
    TEST_ASSERT_EQUAL_PTR(&trailBlazers, teamFromString("Portland Trail Blazers"));
}

void test_from_string_no_match_returns_null() {
    TEST_ASSERT_NULL(teamFromString("Globetrotters"));
}

void test_from_string_empty_returns_null() {
    TEST_ASSERT_NULL(teamFromString(""));
}

// --- teamFromAbbr ---

void test_from_abbr_known_uppercase() {
    TEST_ASSERT_EQUAL_PTR(&celtics, teamFromAbbr("BOS"));
}

void test_from_abbr_unknown_returns_null() {
    TEST_ASSERT_NULL(teamFromAbbr("ZZZ"));
}

void test_from_abbr_is_case_sensitive() {
    // Documents current behavior (raw strcmp, not strcasecmp). Every caller
    // today uppercases first (control_server.cpp) - this is a landmine if a
    // second entry point (e.g. a future companion app) skips that step.
    TEST_ASSERT_NULL(teamFromAbbr("bos"));
}

void test_from_abbr_empty_returns_null() {
    TEST_ASSERT_NULL(teamFromAbbr(""));
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

    return UNITY_END();
}
