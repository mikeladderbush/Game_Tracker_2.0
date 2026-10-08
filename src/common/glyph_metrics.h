/*
********************************************************************************

    Pixel-exact text layout. Measures each glyph's actual ink columns
    instead of trusting its declared width, so letter gaps land on a flat
    1px regardless of blank padding in the bitmap. Proven exhaustively in
    test/test_glyph_metrics/ (~7800 cases), not spot-checked.

********************************************************************************
*/

#pragma once
#include <cstdint>
#include "glyph_data.h"

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
