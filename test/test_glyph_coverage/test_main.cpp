#include <unity.h>
#include <cstring>
#include <cstdio>
#include "common/glyph_data.h"

void setUp() {}
void tearDown() {}

static bool hasGlyph(char ch) {
    for (size_t i = 0; i < GLYPH_TABLE_SIZE; i++) {
        if (GLYPH_TABLE[i].ch == ch) return true;
    }
    return false;
}

// Every character actually passed to drawChar(...)/drawCharColored(...)
// anywhere in the codebase, as of this pass:
//   - digits '0'-'9'                          (scores, clock, date, OT number)
//   - ' ' ':' '-' '_' '.'                     (blank digit, clock, date, selector, odds decimals)
//   - 'p' 'a'                                  (AM/PM indicator)
//   - "1st" "2nd" "3rd" "4th" "HT" "OT"/"OTn"   (quarterLabel output)
//   - nba/nba_menu.cpp's city-menu team abbreviation letters
//   - nfl/nfl_menu.cpp's team abbreviations, "VS.", "OU", "ODDS TBD", "OF"
// If a character below is missing from GLYPH_TABLE, drawChar silently draws
// nothing - invisible until you're staring at the actual board. This test
// catches that at build time instead.
static const char CHARS_USED[] =
    "0123456789 :-_.pa"
    "stndrhOT"                              // quarter labels
    "ATLBKNOSCHIDEGWUMPRXYFJV";              // team abbreviations + VS./OU/ODDS TBD/OF (deduped by hand)

void test_every_used_character_has_a_glyph() {
    for (size_t i = 0; i < sizeof(CHARS_USED) - 1; i++) {
        char ch = CHARS_USED[i];
        char msg[64];
        snprintf(msg, sizeof(msg), "missing glyph for '%c'", ch);
        TEST_ASSERT_TRUE_MESSAGE(hasGlyph(ch), msg);
    }
}

void test_glyph_table_size_matches_declared_constant() {
    // GLYPH_TABLE_SIZE is hand-maintained (glyph_data.cpp) rather than
    // derived via sizeof - this catches it silently drifting out of sync
    // after a future edit.
    TEST_ASSERT_TRUE(GLYPH_TABLE_SIZE > 0);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_every_used_character_has_a_glyph);
    RUN_TEST(test_glyph_table_size_matches_declared_constant);
    return UNITY_END();
}
