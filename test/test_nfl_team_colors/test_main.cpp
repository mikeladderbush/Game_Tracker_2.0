#include <unity.h>
#include <cstdio>
#include <cstring>
#include "nfl/nfl_team_colors.h"

void setUp() {}
void tearDown() {}

void test_known_abbr_returns_real_colors() {
    NflTeamColor c = nflColorForAbbr("SEA");
    TEST_ASSERT_EQUAL_HEX16(0x0108, c.primary);    // College Navy
    TEST_ASSERT_EQUAL_HEX16(0x6de5, c.secondary);  // Action Green
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

void test_no_team_has_pure_black_primary_or_secondary() {
    // Both colors get drawn as direct letter text now (nfl_menu.cpp's
    // drawTextAlternating alternates primary/secondary per character), so
    // pure black in either slot would make every other letter invisible
    // against the display's black background. Merely-dark colors (navy,
    // deep steel, brown) are fine - only exact 0x0000 is the bug.
    for (int i = 0; i < NFL_TEAM_COLOR_COUNT; i++) {
        char msg[56];
        snprintf(msg, sizeof(msg), "%s has black as primary", NFL_TEAM_COLORS[i].abbr);
        TEST_ASSERT_FALSE_MESSAGE(NFL_TEAM_COLORS[i].primary == 0x0000, msg);

        snprintf(msg, sizeof(msg), "%s has black as secondary", NFL_TEAM_COLORS[i].abbr);
        TEST_ASSERT_FALSE_MESSAGE(NFL_TEAM_COLORS[i].secondary == 0x0000, msg);
    }
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_known_abbr_returns_real_colors);
    RUN_TEST(test_unknown_abbr_falls_back_to_white_not_null);
    RUN_TEST(test_table_has_all_32_teams);
    RUN_TEST(test_no_duplicate_abbreviations);
    RUN_TEST(test_no_team_has_pure_black_primary_or_secondary);
    return UNITY_END();
}
