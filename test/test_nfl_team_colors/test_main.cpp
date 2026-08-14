#include <unity.h>
#include <cstdio>
#include <cstring>
#include "nfl/nfl_team_colors.h"

void setUp() {}
void tearDown() {}

void test_known_abbr_returns_real_colors() {
    NflTeamColor c = nflColorForAbbr("SEA");
    TEST_ASSERT_EQUAL_HEX16(0x014b, c.primary);
    TEST_ASSERT_EQUAL_HEX16(0x6de5, c.secondary);
}

void test_unknown_abbr_falls_back_to_white_not_null() {
    NflTeamColor c = nflColorForAbbr("ZZZ");
    TEST_ASSERT_EQUAL_HEX16(0xffff, c.primary);
    TEST_ASSERT_EQUAL_HEX16(0xffff, c.secondary);
}

void test_table_has_all_32_teams() {
    TEST_ASSERT_EQUAL_INT(32, NFL_TEAM_COLOR_COUNT);
}

void test_no_duplicate_abbreviations() {
    for (int i = 0; i < NFL_TEAM_COLOR_COUNT; i++) {
        for (int j = i + 1; j < NFL_TEAM_COLOR_COUNT; j++) {
            char msg[64];
            snprintf(msg, sizeof(msg), "duplicate abbr at %d and %d", i, j);
            TEST_ASSERT_NOT_EQUAL_MESSAGE(
                0, strcmp(NFL_TEAM_COLORS[i].abbr, NFL_TEAM_COLORS[j].abbr), msg);
        }
    }
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_known_abbr_returns_real_colors);
    RUN_TEST(test_unknown_abbr_falls_back_to_white_not_null);
    RUN_TEST(test_table_has_all_32_teams);
    RUN_TEST(test_no_duplicate_abbreviations);
    return UNITY_END();
}
