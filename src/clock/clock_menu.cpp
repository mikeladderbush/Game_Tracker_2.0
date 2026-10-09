/*
********************************************************************************

    clock_menu.cpp. Static: digits + a smaller AM/PM label below, one
    block. On a quarter-hour mark a transition effect plays instead -
    SAND (:15/:45), FIRE (:30), PLASMA (:00) - then the static block
    takes over once it ends.

********************************************************************************
*/

#include "clock_menu.h"
#include "../common/draw_tools.h"
#include "../common/accelerometer.h"
#include <cstring>
#include <cstdlib>
#include <cmath>

static const int PANEL_WIDTH = 64;
static const int PANEL_HEIGHT = 64;
static const uint16_t WHITE = 0xffff;
// Largest size the time digits fit the 64px panel at with margin to spare
// ("12:45" is 57px at size 4; size 5 would be 70px, too wide) - confirmed
// via common/glyph_metrics.h's textWidth(), not guessed.
static const uint8_t CLOCK_SIZE = 4;
static const uint8_t AMPM_SIZE = 2;
static const int BLOCK_GAP = 4;  // px between the digits and the AM/PM label



/*
********************************************************************************

    Static display: digits + AM/PM.

********************************************************************************
*/

static void drawCentered(const char* text, int y, uint8_t size) {
    int w = textWidth(text, size);
    int x = (PANEL_WIDTH - w) / 2;
    if (x < 0) x = 0;
    drawText(text, x, y, size, WHITE);
}

static int blockHeight() {
    return 5 * CLOCK_SIZE + BLOCK_GAP + 5 * AMPM_SIZE;
}

// getCurrentTime() (common/ntp_time.h) appends a trailing 'a'/'p' marker,
// e.g. "9:05a" - split that off and draw it as its own smaller "AM"/"PM"
// line below the digits, rather than one uniformly-sized string (which
// doesn't fit the panel once a marker's appended at full size).
static void drawClockBlock(const char* text, int y) {
    size_t len = strlen(text);
    if (len == 0) return;

    char marker = text[len - 1];
    bool hasMarker = marker == 'a' || marker == 'p';
    size_t digitLen = hasMarker ? len - 1 : len;

    char digits[8];
    if (digitLen >= sizeof(digits)) digitLen = sizeof(digits) - 1;
    memcpy(digits, text, digitLen);
    digits[digitLen] = '\0';

    drawCentered(digits, y, CLOCK_SIZE);
    if (hasMarker) {
        drawCentered(marker == 'p' ? "PM" : "AM", y + 5 * CLOCK_SIZE + BLOCK_GAP, AMPM_SIZE);
    }
}



/*
********************************************************************************

    Shared grain walk for the sand effect - every lit pixel-cell of a
    clock block (digits at CLOCK_SIZE, AM/PM at AMPM_SIZE), with its
    on-screen position and the size it should be drawn at. Same cursor
    math drawText uses internally, via the public GLYPH_TABLE/
    measureGlyph rather than draw_tools.cpp's own private glyph lookup.

********************************************************************************
*/

static const GlyphEntry* findGlyphLocal(char ch) {
    for (size_t i = 0; i < GLYPH_TABLE_SIZE; i++) {
        if (GLYPH_TABLE[i].ch == ch) return &GLYPH_TABLE[i];
    }
    return nullptr;
}

template <typename Fn>
static void walkGlyphCells(const char* text, int x, int y, uint8_t size, Fn&& fn) {
    int cursor = x;
    for (const char* p = text; *p; p++) {
        GlyphMetrics m = measureGlyph(*p, size);
        const GlyphEntry* g = findGlyphLocal(*p);
        if (g) {
            for (int row = 0; row < g->height; row++) {
                for (int col = 0; col < g->width; col++) {
                    if (g->pattern[row * g->width + col] != 0) {
                        fn(cursor + m.drawXOffset + col * size, y + row * size, size);
                    }
                }
            }
        }
        cursor += m.advance;
    }
}

template <typename Fn>
static void walkClockBlockCells(const char* text, int baseY, Fn&& fn) {
    size_t len = strlen(text);
    if (len == 0) return;

    char marker = text[len - 1];
    bool hasMarker = marker == 'a' || marker == 'p';
    size_t digitLen = hasMarker ? len - 1 : len;

    char digits[8];
    if (digitLen >= sizeof(digits)) digitLen = sizeof(digits) - 1;
    memcpy(digits, text, digitLen);
    digits[digitLen] = '\0';

    int digitsX = (PANEL_WIDTH - textWidth(digits, CLOCK_SIZE)) / 2;
    if (digitsX < 0) digitsX = 0;
    walkGlyphCells(digits, digitsX, baseY, CLOCK_SIZE, fn);

    if (hasMarker) {
        const char* label = marker == 'p' ? "PM" : "AM";
        int labelY = baseY + 5 * CLOCK_SIZE + BLOCK_GAP;
        int labelX = (PANEL_WIDTH - textWidth(label, AMPM_SIZE)) / 2;
        if (labelX < 0) labelX = 0;
        walkGlyphCells(label, labelX, labelY, AMPM_SIZE, fn);
    }
}



/*
********************************************************************************

    SAND (:15 / :45) - the old time's lit cells fall away, the new time's
    fall into place from above, each cell's drift/color seeded from its
    own screen position (not stored state, so nothing to reset between
    transitions). Gravity direction biased by the board's tilt if the
    onboard accelerometer answered at boot; straight down otherwise.

********************************************************************************
*/

static const uint16_t SAND_PALETTE[] = {0x1b3f, 0x06ff, 0x0655, 0x89ff, 0xf9f9, 0xdf9f};
static const int SAND_PALETTE_SIZE = 6;

static uint32_t hash2(int x, int y) {
    uint32_t h = (uint32_t)(x * 374761393 + y * 668265263);
    h = (h ^ (h >> 13)) * 1274126177u;
    return h ^ (h >> 16);
}

static void drawSandTransition(const ClockFrame& frame) {
    int baseY = (PANEL_HEIGHT - blockHeight()) / 2;
    if (baseY < 0) baseY = 0;

    TiltReading tilt = readTilt();
    float t = frame.progress;
    float eased = 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);  // ease-out cubic, for the reform fall

    // Old time: falls away and off the bottom edge.
    walkClockBlockCells(frame.prevText, baseY, [&](int x, int y, int size) {
        uint32_t h = hash2(x, y);
        float drift = ((int)(h % 41) - 20) * 0.6f;
        float gravity = 60.0f + (float)((h >> 8) % 40);
        float fx = x + (tilt.x * 30.0f + drift) * t;
        float fy = y + gravity * t * t + tilt.y * 20.0f * t;
        uint16_t color = SAND_PALETTE[h % SAND_PALETTE_SIZE];
        matrix.fillRect((int)fx, (int)fy, size, size, color);
    });

    // New time: falls in from above and settles into place.
    walkClockBlockCells(frame.text, baseY, [&](int x, int y, int size) {
        uint32_t h = hash2(x + 1000, y + 1000);
        float startX = x + (float)((int)(h % 61) - 30);
        float startY = -10.0f - (float)((h >> 6) % 40);
        float curX = startX + (x - startX) * eased;
        float curY = startY + (y - startY) * eased;
        uint16_t color = SAND_PALETTE[(h >> 3) % SAND_PALETTE_SIZE];
        matrix.fillRect((int)curX, (int)curY, size, size, color);
    });
}



/*
********************************************************************************

    FIRE (:30) - classic demoscene technique: each frame, average every
    cell with its neighbors below and cool it slightly, with fresh random
    heat fed in at the bottom. A precomputed palette turns heat into
    color. Full panel, no text - the new time appears once it's done.

********************************************************************************
*/

static uint8_t fireHeat[PANEL_HEIGHT][PANEL_WIDTH];

static uint16_t fireColor(uint8_t heat) {
    if (heat == 0) return 0x0000;
    if (heat < 60) return 0xb000;   // deep red
    if (heat < 120) return 0xf9e0;  // red-orange
    if (heat < 180) return 0xfc60;  // orange
    if (heat < 220) return 0xfe43;  // yellow-orange
    return 0xff8f;                  // yellow
}

static void drawFireTransition(const ClockFrame& frame) {
    if (frame.progress < 0.05f) {
        // Fresh transition: seed the whole buffer with a bottom-hot
        // gradient instead of starting cold, so the panel is already
        // full of fire by the first frame rather than growing upward
        // for the first half of a very short effect.
        for (int y = 0; y < PANEL_HEIGHT; y++) {
            for (int x = 0; x < PANEL_WIDTH; x++) {
                int bias = 255 - (255 * y) / PANEL_HEIGHT;
                fireHeat[y][x] = (uint8_t)(rand() % 256 * bias / 255);
            }
        }
    }

    for (int x = 0; x < PANEL_WIDTH; x++) {
        fireHeat[PANEL_HEIGHT - 1][x] = (uint8_t)(160 + rand() % 96);
    }
    for (int y = 0; y < PANEL_HEIGHT - 1; y++) {
        for (int x = 0; x < PANEL_WIDTH; x++) {
            int left = fireHeat[y + 1][x > 0 ? x - 1 : 0];
            int mid = fireHeat[y + 1][x];
            int right = fireHeat[y + 1][x < PANEL_WIDTH - 1 ? x + 1 : PANEL_WIDTH - 1];
            int decay = rand() % 4;
            int next = (left + mid + right) / 3 - decay;
            fireHeat[y][x] = (uint8_t)(next < 0 ? 0 : next);
        }
    }

    for (int y = 0; y < PANEL_HEIGHT; y++) {
        for (int x = 0; x < PANEL_WIDTH; x++) {
            matrix.drawPixel(x, y, fireColor(fireHeat[y][x]));
        }
    }
}



/*
********************************************************************************

    PLASMA (:00) - a sum of sine waves sampled on a coarse grid, cycled
    through a full-saturation rainbow. No persistent state - purely a
    function of elapsed time, unlike fire's heat buffer.

********************************************************************************
*/

static const int PLASMA_CELL = 2;  // px per plasma block - full-resolution would be ~4x the sin() calls

static uint16_t hueToRgb565(float hue) {
    hue -= floorf(hue);
    float h6 = hue * 6.0f;
    int i = (int)h6;
    float f = h6 - i;
    uint8_t p = 0;
    uint8_t q = (uint8_t)(255 * (1.0f - f));
    uint8_t tt = (uint8_t)(255 * f);
    uint8_t r, g, b;
    switch (i % 6) {
        case 0: r = 255; g = tt; b = p; break;
        case 1: r = q; g = 255; b = p; break;
        case 2: r = p; g = 255; b = tt; break;
        case 3: r = p; g = q; b = 255; break;
        case 4: r = tt; g = p; b = 255; break;
        default: r = 255; g = p; b = q; break;
    }
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

static void drawPlasmaTransition(const ClockFrame& frame) {
    float t = frame.progress * 6.2831853f * 3.0f;  // 3 full cycles over the effect's duration

    for (int by = 0; by < PANEL_HEIGHT; by += PLASMA_CELL) {
        for (int bx = 0; bx < PANEL_WIDTH; bx += PLASMA_CELL) {
            float dx = bx - 32.0f, dy = by - 32.0f;
            float v = sinf(bx * 0.15f + t) +
                      sinf(by * 0.15f + t * 1.3f) +
                      sinf((bx + by) * 0.1f + t * 0.7f) +
                      sinf(sqrtf(dx * dx + dy * dy) * 0.15f - t * 1.5f);
            float hue = (v + 4.0f) / 8.0f;
            matrix.fillRect(bx, by, PLASMA_CELL, PLASMA_CELL, hueToRgb565(hue));
        }
    }
}



void drawClock(const ClockFrame& frame) {
    switch (frame.anim) {
        case ClockAnim::SAND:   drawSandTransition(frame); return;
        case ClockAnim::FIRE:   drawFireTransition(frame); return;
        case ClockAnim::PLASMA: drawPlasmaTransition(frame); return;
        default: break;
    }

    int y = (PANEL_HEIGHT - blockHeight()) / 2;
    if (y < 0) y = 0;
    drawClockBlock(frame.text, y);
}
