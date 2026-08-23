#pragma once
#include <cstdint>
#include "glyph_data.h"

// Ink-bounds-aware text layout math - pure data/logic, no
// Adafruit_Protomatter dependency, so it's compiled and exhaustively unit
// tested natively (test/test_glyph_metrics/) rather than trusted by
// spot-check. draw_tools.cpp's drawText/drawTextAlternating use this for
// the actual matrix drawing.
//
// A flat per-glyph declared width can't produce a uniform visual gap: most
// glyphs have blank padding columns baked into their bitmap (e.g. 'S' has a
// fully blank trailing column - declared width 4, ink only fills columns
// 0-2 - while 'A' has a blank *leading* column instead), and how much
// varies letter to letter. Measuring each glyph's actual ink columns at
// runtime and trimming to that, instead of its declared bounding box, is
// what makes the gap land on a true, uniform 1px for every letter pair.

// Leftmost/rightmost column (inclusive) containing ink (a pattern value
// != 0) in the glyph. An entirely blank glyph (space) reports the full
// declared width instead, so it still takes up visible room.
void glyphInkBounds(const GlyphEntry& g, int& left, int& right);

struct GlyphMetrics {
    int drawXOffset;  // how far left of the nominal cursor to actually draw, so ink (not the bounding box) starts at the cursor
    int advance;       // how far to move the cursor for the next glyph: ink width * size + a flat 1px gap
};

// {0, 4*size+1} for an unrecognized character.
GlyphMetrics measureGlyph(char ch, uint8_t size);

int textWidth(const char* text, uint8_t size);
