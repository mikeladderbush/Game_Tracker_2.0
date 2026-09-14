#pragma once
#include <cstdint>
#include <cstddef>

// Pure glyph data - no Arduino/hardware dependency, so it (and a test that
// walks it) can be compiled and run natively without a board.

struct GlyphEntry {
    char ch;
    uint8_t width;
    uint8_t height;
    const uint8_t* pattern;
};

extern const GlyphEntry GLYPH_TABLE[];
extern const size_t GLYPH_TABLE_SIZE;
