#include "glyph_metrics.h"
#include <cstddef>

static const GlyphEntry* findGlyph(char ch) {
    for (size_t i = 0; i < GLYPH_TABLE_SIZE; i++) {
        if (GLYPH_TABLE[i].ch == ch) return &GLYPH_TABLE[i];
    }
    return nullptr;
}

void glyphInkBounds(const GlyphEntry& g, int& left, int& right) {
    left = g.width;
    right = -1;
    for (int row = 0; row < g.height; row++) {
        for (int col = 0; col < g.width; col++) {
            if (g.pattern[row * g.width + col] != 0) {
                if (col < left) left = col;
                if (col > right) right = col;
            }
        }
    }
    if (right < 0) {
        left = 0;
        right = g.width - 1;
    }
}

GlyphMetrics measureGlyph(char ch, uint8_t size) {
    const GlyphEntry* g = findGlyph(ch);
    if (!g) return {0, 4 * size + 1};
    int left, right;
    glyphInkBounds(*g, left, right);
    return {-left * size, (right - left + 1) * size + 1};
}

int textWidth(const char* text, uint8_t size) {
    int width = 0;
    for (const char* p = text; *p; p++) {
        width += measureGlyph(*p, size).advance;
    }
    return width;
}
