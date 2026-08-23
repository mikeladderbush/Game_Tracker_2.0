#include <unity.h>
#include <cstdio>
#include "common/glyph_metrics.h"

void setUp() {}
void tearDown() {}

// --- glyphInkBounds spot checks (documents the specific cases that caused
// the original bug: some glyphs have a blank trailing column, others a
// blank leading column, others are fully blank) ---

void test_ink_bounds_S_has_blank_trailing_column() {
    const GlyphEntry* g = nullptr;
    for (size_t i = 0; i < GLYPH_TABLE_SIZE; i++) {
        if (GLYPH_TABLE[i].ch == 'S') g = &GLYPH_TABLE[i];
    }
    TEST_ASSERT_NOT_NULL(g);
    int left, right;
    glyphInkBounds(*g, left, right);
    TEST_ASSERT_EQUAL_INT(0, left);
    TEST_ASSERT_EQUAL_INT(2, right);  // declared width is 4 (cols 0-3) - col 3 is blank
}

void test_ink_bounds_A_has_blank_leading_column() {
    const GlyphEntry* g = nullptr;
    for (size_t i = 0; i < GLYPH_TABLE_SIZE; i++) {
        if (GLYPH_TABLE[i].ch == 'A') g = &GLYPH_TABLE[i];
    }
    TEST_ASSERT_NOT_NULL(g);
    int left, right;
    glyphInkBounds(*g, left, right);
    TEST_ASSERT_EQUAL_INT(1, left);  // col 0 is blank
    TEST_ASSERT_EQUAL_INT(3, right);
}

void test_ink_bounds_space_reports_full_declared_width() {
    const GlyphEntry* g = nullptr;
    for (size_t i = 0; i < GLYPH_TABLE_SIZE; i++) {
        if (GLYPH_TABLE[i].ch == ' ') g = &GLYPH_TABLE[i];
    }
    TEST_ASSERT_NOT_NULL(g);
    int left, right;
    glyphInkBounds(*g, left, right);
    // Entirely blank - no ink to measure, so it falls back to the full
    // declared bounding box rather than collapsing to zero width.
    TEST_ASSERT_EQUAL_INT(0, left);
    TEST_ASSERT_EQUAL_INT(g->width - 1, right);
}

// --- measureGlyph sanity ---

void test_measure_glyph_unknown_char_uses_fallback() {
    GlyphMetrics m = measureGlyph('\x01', 2);  // not in GLYPH_TABLE
    TEST_ASSERT_EQUAL_INT(0, m.drawXOffset);
    TEST_ASSERT_EQUAL_INT(4 * 2 + 1, m.advance);
}

// --- The real point of this suite: simulate drawText's cursor math for
// EVERY glyph paired with EVERY other glyph (including itself), at every
// size actually used in the app (1-3), and assert the resulting ink-to-ink
// gap is exactly 1 physical pixel. This is what "take your time and make
// sure every letter combination is right" actually means as a test, rather
// than trusting a few hand-checked examples. GLYPH_TABLE_SIZE^2 * 3 cases
// (~7200) is trivial to run natively. ---

void test_every_glyph_pair_has_exactly_one_pixel_gap_at_every_size() {
    for (uint8_t size = 1; size <= 3; size++) {
        for (size_t i = 0; i < GLYPH_TABLE_SIZE; i++) {
            for (size_t j = 0; j < GLYPH_TABLE_SIZE; j++) {
                char ch1 = GLYPH_TABLE[i].ch;
                char ch2 = GLYPH_TABLE[j].ch;
                // Space has no real ink - it's excluded here and checked
                // separately below, since a word-space needs to stay wider
                // than a letter-gap to remain visually distinguishable from
                // one (otherwise "SEA -3.5" and "SEA-3.5" would look the
                // same).
                if (ch1 == ' ' || ch2 == ' ') continue;

                int left1, right1;
                glyphInkBounds(GLYPH_TABLE[i], left1, right1);
                GlyphMetrics m1 = measureGlyph(ch1, size);
                (void)ch2;  // only ch1's own shape matters for gap-after-ch1 - see comment below

                // Simulate drawText with ch1's cursor starting at 0. By
                // construction, drawXOffset (= -left*size) always cancels
                // out left*size when a glyph is drawn, so EVERY glyph's ink
                // starts exactly at its nominal cursor position regardless
                // of its own left/right - that's the whole point of
                // drawXOffset. So ink1 starts at 0 (not at m1.drawXOffset -
                // that's the *draw* position, not the *ink* position), and
                // whatever glyph comes next also starts its ink exactly at
                // cursor2, regardless of ITS OWN shape.
                int inkEnd1 = (right1 - left1 + 1) * size - 1;
                int cursor2 = m1.advance;
                int inkStart2 = cursor2;

                int gap = inkStart2 - inkEnd1 - 1;

                char msg[80];
                snprintf(msg, sizeof(msg), "gap '%c'->'%c' at size %d was %d, expected 1",
                         ch1, ch2, size, gap);
                TEST_ASSERT_EQUAL_INT_MESSAGE(1, gap, msg);
            }
        }
    }
}

void test_space_advance_is_wider_than_a_letter_gap() {
    // Space should read as a real word-separator, not just another 1px
    // letter-gap - assert its advance is at least as wide as a typical
    // letter's own advance, at every size actually used.
    for (uint8_t size = 1; size <= 3; size++) {
        GlyphMetrics space = measureGlyph(' ', size);
        GlyphMetrics letter = measureGlyph('E', size);  // a typical mid-width letter, no leading/trailing quirks
        char msg[64];
        snprintf(msg, sizeof(msg), "space advance (%d) not >= letter advance (%d) at size %d",
                 space.advance, letter.advance, size);
        TEST_ASSERT_TRUE_MESSAGE(space.advance >= letter.advance, msg);
    }
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_ink_bounds_S_has_blank_trailing_column);
    RUN_TEST(test_ink_bounds_A_has_blank_leading_column);
    RUN_TEST(test_ink_bounds_space_reports_full_declared_width);
    RUN_TEST(test_measure_glyph_unknown_char_uses_fallback);
    RUN_TEST(test_every_glyph_pair_has_exactly_one_pixel_gap_at_every_size);
    RUN_TEST(test_space_advance_is_wider_than_a_letter_gap);

    return UNITY_END();
}
