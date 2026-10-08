/*
********************************************************************************

    The bitmap font. 51 glyphs: digits, uppercase letters, punctuation.
    Pure data, no hardware dependency.

********************************************************************************
*/

#pragma once
#include <cstdint>
#include <cstddef>

struct GlyphEntry {
    char ch;
    uint8_t width;
    uint8_t height;
    const uint8_t* pattern;
};

extern const GlyphEntry GLYPH_TABLE[];
extern const size_t GLYPH_TABLE_SIZE;
