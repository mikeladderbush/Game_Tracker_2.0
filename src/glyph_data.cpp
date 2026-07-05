#include "draw_tools.h"

static const uint8_t glyph_0_pattern[20] = {0,2,2,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,2,2};
static const uint8_t glyph_1_pattern[20] = {0,0,2,0,0,2,2,0,0,0,2,0,0,0,2,0,0,2,2,2};
static const uint8_t glyph_2_pattern[20] = {0,2,2,2,0,0,0,2,0,2,2,2,0,2,0,0,0,2,2,2};
static const uint8_t glyph_3_pattern[20] = {0,2,2,2,0,0,0,2,0,0,2,2,0,0,0,2,0,2,2,2};
static const uint8_t glyph_4_pattern[20] = {0,2,0,2,0,2,0,2,0,2,2,2,0,0,0,2,0,0,0,2};
static const uint8_t glyph_5_pattern[20] = {0,2,2,2,0,2,0,0,0,2,2,2,0,0,0,2,0,2,2,2};
static const uint8_t glyph_6_pattern[20] = {0,2,2,2,0,2,0,0,0,2,2,2,0,2,0,2,0,2,2,2};
static const uint8_t glyph_7_pattern[20] = {0,2,2,2,0,0,0,2,0,0,2,0,0,0,2,0,0,0,2,0};
static const uint8_t glyph_8_pattern[20] = {0,2,2,2,0,2,0,2,0,2,2,2,0,2,0,2,0,2,2,2};
static const uint8_t glyph_9_pattern[20] = {0,2,2,2,0,2,0,2,0,2,2,2,0,0,0,2,0,0,0,2};
static const uint8_t glyph_10_pattern[20] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static const uint8_t glyph_11_pattern[15] = {0,0,0,0,2,0,0,0,0,0,2,0,0,0,0};
static const uint8_t glyph_12_pattern[15] = {0,0,0,0,0,2,0,2,0,2,0,0,0,0,0};
static const uint8_t glyph_13_pattern[15] = {0,0,0,0,0,0,0,2,2,0,0,0,0,0,0};
static const uint8_t glyph_14_pattern[20] = {0,2,2,2,0,2,0,2,0,2,2,2,0,2,0,2,0,2,0,2};
static const uint8_t glyph_15_pattern[20] = {0,2,2,0,0,0,2,0,2,2,2,0,2,0,2,0,2,2,2,2};
static const uint8_t glyph_16_pattern[20] = {2,2,0,0,2,0,2,0,2,2,0,0,2,0,2,0,2,2,0,0};
static const uint8_t glyph_17_pattern[20] = {2,2,2,0,2,0,0,0,2,0,0,0,2,0,0,0,2,2,2,0};
static const uint8_t glyph_18_pattern[20] = {2,2,0,0,2,0,2,0,2,0,2,0,2,0,2,0,2,2,0,0};
static const uint8_t glyph_19_pattern[20] = {0,0,2,0,0,0,2,0,2,2,2,0,2,0,2,0,2,2,2,0};
static const uint8_t glyph_20_pattern[20] = {2,2,2,0,2,0,0,0,2,2,2,0,2,0,0,0,2,2,2,0};
static const uint8_t glyph_21_pattern[20] = {0,2,2,0,0,2,0,0,2,2,2,0,0,2,0,0,0,2,0,0};
static const uint8_t glyph_22_pattern[20] = {0,2,2,0,2,0,0,0,2,0,2,2,2,0,0,2,0,2,2,0};
static const uint8_t glyph_23_pattern[20] = {2,0,2,0,2,0,2,0,2,2,2,0,2,0,2,0,2,0,2,0};
static const uint8_t glyph_24_pattern[20] = {2,0,0,0,2,0,0,0,2,2,2,0,2,0,2,0,2,0,2,0};
static const uint8_t glyph_25_pattern[20] = {2,2,2,0,0,2,0,0,0,2,0,0,0,2,0,0,2,2,2,0};
static const uint8_t glyph_26_pattern[20] = {2,0,2,0,2,0,2,0,2,2,0,0,2,0,2,0,2,0,2,0};
static const uint8_t glyph_27_pattern[20] = {2,0,0,0,2,0,0,0,2,0,0,0,2,0,0,0,2,2,2,0};
static const uint8_t glyph_28_pattern[20] = {2,0,0,0,2,0,0,0,2,0,0,0,2,0,0,0,2,0,0,0};
static const uint8_t glyph_29_pattern[25] = {2,0,0,0,2,2,2,0,2,2,2,0,2,0,2,2,0,0,0,2,2,0,0,0,2};
static const uint8_t glyph_30_pattern[20] = {2,0,0,2,2,2,0,2,2,2,2,2,2,0,2,2,2,0,0,2};
static const uint8_t glyph_31_pattern[20] = {0,0,0,0,0,0,0,0,2,2,2,0,2,0,2,0,2,0,2,0};
static const uint8_t glyph_32_pattern[20] = {2,2,2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,2,2,0};
static const uint8_t glyph_33_pattern[20] = {2,2,2,0,2,0,2,0,2,2,2,0,2,0,0,0,2,0,0,0};
static const uint8_t glyph_34_pattern[20] = {0,2,2,2,0,2,0,2,0,2,2,2,0,2,0,0,0,2,0,0};
static const uint8_t glyph_35_pattern[20] = {0,0,0,0,0,0,0,0,2,2,0,0,2,0,0,0,2,0,0,0};
static const uint8_t glyph_36_pattern[20] = {2,2,2,0,2,0,2,0,2,2,0,0,2,0,2,0,2,0,2,0};
static const uint8_t glyph_37_pattern[20] = {0,2,2,0,2,0,0,0,2,2,0,0,0,0,2,0,2,2,0,0};
static const uint8_t glyph_38_pattern[20] = {0,0,0,0,0,2,0,0,2,0,0,0,0,2,0,0,2,0,0,0};
static const uint8_t glyph_39_pattern[20] = {2,2,2,0,0,2,0,0,0,2,0,0,0,2,0,0,0,2,0,0};
static const uint8_t glyph_40_pattern[20] = {0,2,0,0,0,2,0,0,2,2,2,0,0,2,0,0,0,2,0,0};
static const uint8_t glyph_41_pattern[20] = {2,0,2,0,2,0,2,0,2,0,2,0,2,0,2,0,0,2,0,0};
static const uint8_t glyph_42_pattern[25] = {2,0,0,0,2,2,0,0,0,2,2,0,2,0,2,2,0,2,0,2,0,2,0,2,0};
static const uint8_t glyph_43_pattern[20] = {2,0,2,0,2,0,2,0,0,2,0,0,2,0,2,0,2,0,2,0};
static const uint8_t glyph_44_pattern[20] = {2,0,2,0,2,0,2,0,0,2,0,0,0,2,0,0,0,2,0,0};

const GlyphEntry GLYPH_TABLE[] = {
    {'0', 4, 5, glyph_0_pattern},
    {'1', 4, 5, glyph_1_pattern},
    {'2', 4, 5, glyph_2_pattern},
    {'3', 4, 5, glyph_3_pattern},
    {'4', 4, 5, glyph_4_pattern},
    {'5', 4, 5, glyph_5_pattern},
    {'6', 4, 5, glyph_6_pattern},
    {'7', 4, 5, glyph_7_pattern},
    {'8', 4, 5, glyph_8_pattern},
    {'9', 4, 5, glyph_9_pattern},
    {' ', 4, 5, glyph_10_pattern},
    {':', 3, 5, glyph_11_pattern},
    {'-', 3, 5, glyph_12_pattern},
    {'_', 3, 5, glyph_13_pattern},
    {'A', 4, 5, glyph_14_pattern},
    {'a', 4, 5, glyph_15_pattern},
    {'B', 4, 5, glyph_16_pattern},
    {'C', 4, 5, glyph_17_pattern},
    {'D', 4, 5, glyph_18_pattern},
    {'d', 4, 5, glyph_19_pattern},
    {'E', 4, 5, glyph_20_pattern},
    {'f', 4, 5, glyph_21_pattern},
    {'G', 4, 5, glyph_22_pattern},
    {'H', 4, 5, glyph_23_pattern},
    {'h', 4, 5, glyph_24_pattern},
    {'I', 4, 5, glyph_25_pattern},
    {'K', 4, 5, glyph_26_pattern},
    {'L', 4, 5, glyph_27_pattern},
    {'l', 4, 5, glyph_28_pattern},
    {'M', 5, 5, glyph_29_pattern},
    {'N', 4, 5, glyph_30_pattern},
    {'n', 4, 5, glyph_31_pattern},
    {'O', 4, 5, glyph_32_pattern},
    {'P', 4, 5, glyph_33_pattern},
    {'p', 4, 5, glyph_34_pattern},
    {'r', 4, 5, glyph_35_pattern},
    {'R', 4, 5, glyph_36_pattern},
    {'S', 4, 5, glyph_37_pattern},
    {'s', 4, 5, glyph_38_pattern},
    {'T', 4, 5, glyph_39_pattern},
    {'t', 4, 5, glyph_40_pattern},
    {'U', 4, 5, glyph_41_pattern},
    {'W', 5, 5, glyph_42_pattern},
    {'X', 4, 5, glyph_43_pattern},
    {'Y', 4, 5, glyph_44_pattern},
};
const size_t GLYPH_TABLE_SIZE = 45;