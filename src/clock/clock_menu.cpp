/*
********************************************************************************

    clock_menu.cpp. Static: digits on top, a 2x2 grid below (AM/PM,
    date, turkey in November, temp). On a quarter-hour mark a transition
    effect plays instead - SAND (:15/:45), FIRE (:30), PLASMA (:00),
    none of which touch the grid - then the static block takes over
    once it ends.

********************************************************************************
*/

#include "clock_menu.h"
#include "../common/draw_tools.h"
#include "../common/accelerometer.h"
#include <cstring>
#include <cmath>

static const int PANEL_WIDTH = 64;
static const int PANEL_HEIGHT = 64;
static const uint16_t WHITE = 0xffff;
static const uint16_t DATE_GRAY = 0x94b2;  // dimmer than WHITE - subordinate to the time
// Largest size the time digits fit the 64px panel at with margin to spare
// ("12:45" is 57px at size 4; size 5 would be 70px, too wide) - confirmed
// via common/glyph_metrics.h's textWidth(), not guessed.
static const uint8_t CLOCK_SIZE = 4;
static const uint8_t AMPM_SIZE = 2;
static const uint8_t TEMP_SIZE = 2;
static const uint8_t DATE_SIZE = 1;
static const int BLOCK_GAP = 4;  // px between each line

static uint16_t lerp565(uint16_t a, uint16_t b, float t) {
    int ar = (a >> 11) & 0x1F, ag = (a >> 5) & 0x3F, ab = a & 0x1F;
    int br = (b >> 11) & 0x1F, bg = (b >> 5) & 0x3F, bb = b & 0x1F;
    int r = ar + (int)((br - ar) * t);
    int g = ag + (int)((bg - ag) * t);
    int b2 = ab + (int)((bb - ab) * t);
    return (uint16_t)(((r & 0x1F) << 11) | ((g & 0x3F) << 5) | (b2 & 0x1F));
}

// Temp line color: blue (cold) to yellow (hot), clamped to a Boston-ish
// year-round range rather than true extremes.
static const float TEMP_COLD_F = 20.0f;
static const float TEMP_HOT_F = 90.0f;
static const uint16_t TEMP_COLD_COLOR = 0x3c5f;
static const uint16_t TEMP_HOT_COLOR = 0xfe64;

static uint16_t tempColor(const ClockFrame& frame) {
    if (!frame.hasTemp) return WHITE;
    float t = (frame.tempF - TEMP_COLD_F) / (TEMP_HOT_F - TEMP_COLD_F);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;
    return lerp565(TEMP_COLD_COLOR, TEMP_HOT_COLOR, t);
}



/*
********************************************************************************

    One little sprite per month, bottom-left quadrant - see drawQuadrants.
    Same 9x8 pixel art budget as each other, drawn at SPRITE_SCALE (2x)
    and nudged by a slow sine bob (see monthBobPx) so it idles rather
    than sitting dead still.

********************************************************************************
*/

static const uint8_t SPRITE_SCALE = 2;
static const uint8_t SPRITE_W = 9;
static const uint8_t SPRITE_H = 8;

struct MonthSprite {
    const uint8_t* pattern;
    const uint16_t* palette;
};

// January - snowflake
static const uint8_t jan_pattern[SPRITE_W * SPRITE_H] = {
    0,0,0,1,0,1,0,0,0,
    1,0,0,1,0,1,0,0,1,
    0,1,0,1,0,1,0,1,0,
    0,0,1,1,1,1,1,0,0,
    1,1,1,1,2,1,1,1,1,
    0,0,1,1,1,1,1,0,0,
    0,1,0,1,0,1,0,1,0,
    1,0,0,1,0,1,0,0,1,
};
static const uint16_t jan_palette[3] = {0x0000, 0xa6df, 0xffff};

// February - heart
static const uint8_t feb_pattern[SPRITE_W * SPRITE_H] = {
    0,1,1,0,0,0,1,1,0,
    1,2,1,1,0,1,1,1,1,
    1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,
    0,1,1,1,1,1,1,1,0,
    0,0,1,1,1,1,1,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,0,0,1,0,0,0,0,
};
static const uint16_t feb_palette[3] = {0x0000, 0xd0e7, 0xfbb2};

// March - clover
static const uint8_t mar_pattern[SPRITE_W * SPRITE_H] = {
    0,1,1,0,0,0,1,1,0,
    1,1,1,1,0,1,1,1,1,
    1,1,1,1,1,1,1,1,1,
    0,1,1,0,1,0,1,1,0,
    0,0,0,1,1,1,0,0,0,
    0,0,0,0,2,0,0,0,0,
    0,0,0,0,2,0,0,0,0,
    0,0,0,0,2,0,0,0,0,
};
static const uint16_t mar_palette[3] = {0x0000, 0x24a7, 0x6a45};

// April - umbrella
static const uint8_t apr_pattern[SPRITE_W * SPRITE_H] = {
    0,0,1,1,1,1,1,0,0,
    0,1,1,1,1,1,1,1,0,
    1,1,1,1,1,1,1,1,1,
    0,0,0,0,2,0,0,0,0,
    0,0,0,0,2,0,0,0,0,
    0,0,0,0,2,3,0,0,0,
    0,0,3,0,0,0,0,0,0,
    0,0,0,0,0,3,0,0,0,
};
static const uint16_t apr_palette[4] = {0x0000, 0x3c58, 0x5226, 0x8e3f};

// May - flower
static const uint8_t may_pattern[SPRITE_W * SPRITE_H] = {
    0,0,0,1,1,1,0,0,0,
    0,0,1,1,1,1,1,0,0,
    0,1,1,1,1,1,1,1,0,
    0,0,1,1,1,1,1,0,0,
    0,0,0,0,2,0,0,0,0,
    0,0,0,0,2,0,0,0,0,
    0,0,3,2,2,2,3,0,0,
    0,0,0,0,0,0,0,0,0,
};
static const uint16_t may_palette[4] = {0x0000, 0xdb75, 0x3ca8, 0x3ca8};

// June - sun
static const uint8_t jun_pattern[SPRITE_W * SPRITE_H] = {
    0,1,0,0,1,0,0,1,0,
    0,0,2,2,2,2,2,0,0,
    1,2,2,2,2,2,2,2,1,
    0,2,2,2,2,2,2,2,0,
    1,2,2,2,2,2,2,2,1,
    0,0,2,2,2,2,2,0,0,
    0,1,0,0,1,0,0,1,0,
    0,0,0,0,0,0,0,0,0,
};
static const uint16_t jun_palette[3] = {0x0000, 0xfca4, 0xfe64};

// July - firework
static const uint8_t jul_pattern[SPRITE_W * SPRITE_H] = {
    0,0,0,0,1,0,0,0,0,
    0,0,3,0,1,0,3,0,0,
    0,0,0,3,0,3,0,0,0,
    0,0,2,0,4,0,2,0,0,
    0,0,2,0,4,0,2,0,0,
    0,0,0,3,0,3,0,0,0,
    0,0,3,0,1,0,3,0,0,
    0,0,0,0,1,0,0,0,0,
};
static const uint16_t jul_palette[5] = {0x0000, 0xd986, 0xf626, 0x44bc, 0xffbb};

// August - ice cream
static const uint8_t aug_pattern[SPRITE_W * SPRITE_H] = {
    0,0,1,1,1,1,1,0,0,
    0,1,1,1,1,1,1,1,0,
    0,1,1,2,1,1,1,0,0,
    0,0,1,1,1,1,0,0,0,
    0,0,3,3,3,3,0,0,0,
    0,0,0,3,3,0,0,0,0,
    0,0,0,0,3,0,0,0,0,
    0,0,0,0,3,0,0,0,0,
};
static const uint16_t aug_palette[4] = {0x0000, 0xf679, 0xc0e4, 0xccea};

// September - falling leaf
static const uint8_t sep_pattern[SPRITE_W * SPRITE_H] = {
    0,0,0,1,0,0,0,0,0,
    0,0,1,1,1,0,0,0,0,
    0,1,1,1,1,1,0,0,0,
    1,1,1,1,1,1,1,0,0,
    0,1,1,1,1,1,0,0,0,
    0,0,1,1,1,0,0,0,0,
    0,0,0,2,0,0,0,0,0,
    0,0,0,0,2,0,0,0,0,
};
static const uint16_t sep_palette[3] = {0x0000, 0xdb63, 0x7223};

// October - pumpkin
static const uint8_t oct_pattern[SPRITE_W * SPRITE_H] = {
    0,0,0,2,0,0,0,0,0,
    0,1,1,1,1,1,1,0,0,
    1,1,1,1,1,1,1,1,0,
    1,3,1,1,1,3,1,1,0,
    1,1,1,3,1,1,1,1,0,
    1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,0,0,
    0,0,0,0,0,0,0,0,0,
};
static const uint16_t oct_palette[4] = {0x0000, 0xe3a3, 0x4c46, 0x2081};

// November - turkey
static const uint8_t nov_pattern[SPRITE_W * SPRITE_H] = {
    0,2,0,3,0,4,0,0,0,
    0,1,1,1,1,1,0,0,0,
    2,1,1,1,1,1,1,5,0,
    1,1,1,1,1,1,1,6,0,
    1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,0,0,
    0,0,1,1,1,1,0,0,0,
    0,0,5,0,0,5,0,0,0,
};
static const uint16_t nov_palette[7] = {0x0000, 0x8ac6, 0x7083, 0xdd84, 0xdba3, 0xdba3, 0xc0e4};

// December - tree
static const uint8_t dec_pattern[SPRITE_W * SPRITE_H] = {
    0,0,0,0,1,0,0,0,0,
    0,0,0,1,1,1,0,0,0,
    0,0,1,1,1,1,1,0,0,
    0,0,1,1,2,1,1,0,0,
    0,1,1,1,1,1,1,1,0,
    0,1,1,2,1,1,2,1,0,
    0,0,0,3,3,3,0,0,0,
    0,0,0,3,3,3,0,0,0,
};
static const uint16_t dec_palette[4] = {0x0000, 0x2447, 0xd126, 0x6a45};

static const MonthSprite MONTH_SPRITES[12] = {
    {jan_pattern, jan_palette},
    {feb_pattern, feb_palette},
    {mar_pattern, mar_palette},
    {apr_pattern, apr_palette},
    {may_pattern, may_palette},
    {jun_pattern, jun_palette},
    {jul_pattern, jul_palette},
    {aug_pattern, aug_palette},
    {sep_pattern, sep_palette},
    {oct_pattern, oct_palette},
    {nov_pattern, nov_palette},
    {dec_pattern, dec_palette},
};

// Gentle idle float, +-1px over a ~2.5s cycle - stateless (reads millis()
// directly), same "nothing to reset" approach SAND's drift uses.
static int monthBobPx() {
    return (int)lroundf(sinf(millis() * 0.0025f));
}



/*
********************************************************************************

    Static display: digits + AM/PM.

********************************************************************************
*/

static void drawCentered(const char* text, int y, uint8_t size, uint16_t color = WHITE) {
    int w = textWidth(text, size);
    int x = (PANEL_WIDTH - w) / 2;
    if (x < 0) x = 0;
    drawText(text, x, y, size, color);
}

static int blockHeight() {
    return 5 * CLOCK_SIZE + BLOCK_GAP + 5 * AMPM_SIZE;
}

// Info block below the digits, as a 2x2 grid: AM/PM top-left, date
// top-right, the month's sprite bottom-left, temp bottom-right. Row 2's
// two halves are narrow enough to center in COL_WIDTH; row 1's date
// ("Nov. 10th" and the like) routinely runs 33-38px - over half the
// panel - so AM/PM and date are corner-anchored instead of split into
// two equal halves, which left no room for the date at all.
static const int COL_WIDTH = PANEL_WIDTH / 2;
static const int AMPM_MARGIN = 2;
static const int DATE_MARGIN = 2;

static int row2Height() {
    int spriteH = SPRITE_H * SPRITE_SCALE;
    int tempH = 5 * TEMP_SIZE;
    return spriteH > tempH ? spriteH : tempH;
}

static int quadBlockHeight() {
    return 5 * AMPM_SIZE + BLOCK_GAP + row2Height();
}

static void drawQuadrants(const ClockFrame& frame, const char* ampmLabel, int row1Y) {
    int row2Y = row1Y + 5 * AMPM_SIZE + BLOCK_GAP;
    int r2h = row2Height();

    drawText(ampmLabel, AMPM_MARGIN, row1Y, AMPM_SIZE, WHITE);

    int dateW = textWidth(frame.date, DATE_SIZE);
    int dateX = PANEL_WIDTH - dateW - DATE_MARGIN;
    int dateY = row1Y + (5 * AMPM_SIZE - 5 * DATE_SIZE) / 2;
    drawText(frame.date, dateX, dateY, DATE_SIZE, DATE_GRAY);

    if (frame.month >= 1 && frame.month <= 12) {
        const MonthSprite& s = MONTH_SPRITES[frame.month - 1];
        int sw = SPRITE_W * SPRITE_SCALE;
        int sh = SPRITE_H * SPRITE_SCALE;
        int sx = (COL_WIDTH - sw) / 2;
        int sy = row2Y + (r2h - sh) / 2 + monthBobPx();
        drawSprite(sx, sy, SPRITE_W, SPRITE_H, SPRITE_SCALE, s.pattern, s.palette, 0, false);
    }

    int tempW = textWidth(frame.temp, TEMP_SIZE);
    int tempX = COL_WIDTH + (COL_WIDTH - tempW) / 2;
    int tempY = row2Y + (r2h - 5 * TEMP_SIZE) / 2;
    drawText(frame.temp, tempX, tempY, TEMP_SIZE, tempColor(frame));
}

// getCurrentTime() (common/ntp_time.h) appends a trailing 'a'/'p' marker,
// e.g. "9:05a" - split that off and feed the quadrant grid below the
// digits, rather than drawing one uniformly-sized string (which doesn't
// fit the panel once a marker's appended at full size).
static void drawClockBlock(const ClockFrame& frame, int y) {
    const char* text = frame.text;
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
        int row1Y = y + 5 * CLOCK_SIZE + BLOCK_GAP;
        drawQuadrants(frame, marker == 'p' ? "PM" : "AM", row1Y);
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

    FIRE (:30) and PLASMA (:00) share a shape: both color the NEW time's
    own cells (digits + AM/PM, same walkClockBlockCells as sand) instead
    of painting the full panel - the number is legible as itself from the
    first frame, not a separate effect that cuts to text at the end. Each
    cell's effect color blends to solid white over the back half of the
    duration (resolveToWhite), so it reads as "ignites/shimmers, then
    settles into the number" rather than a hard cut.

********************************************************************************
*/

// Blend weight toward solid white: holds at pure effect color for the
// first 45% of the duration, then eases in over the rest - two phases,
// not a snap at the very end.
static float resolveToWhite(float progress) {
    const float holdUntil = 0.45f;
    if (progress <= holdUntil) return 0.0f;
    float t = (progress - holdUntil) / (1.0f - holdUntil);
    return t * t * (3.0f - 2.0f * t);  // smoothstep
}

template <typename ColorFn>
static void drawMorphingBlock(const ClockFrame& frame, int baseY, ColorFn&& colorFn) {
    float whiteAmount = resolveToWhite(frame.progress);
    walkClockBlockCells(frame.text, baseY, [&](int x, int y, int size) {
        uint16_t color = lerp565(colorFn(x, y), WHITE, whiteAmount);
        matrix.fillRect(x, y, size, size, color);
    });
}

static uint16_t fireColor(uint8_t heat) {
    if (heat < 60) return 0xb000;   // deep red
    if (heat < 120) return 0xf9e0;  // red-orange
    if (heat < 180) return 0xfc60;  // orange
    if (heat < 220) return 0xfe43;  // yellow-orange
    return 0xff8f;                  // yellow
}

static void drawFireTransition(const ClockFrame& frame) {
    int baseY = (PANEL_HEIGHT - blockHeight()) / 2;
    if (baseY < 0) baseY = 0;
    float t = frame.progress;

    drawMorphingBlock(frame, baseY, [&](int x, int y) {
        // Per-cell flicker, phase-offset by a hash of its own position so
        // neighboring cells don't flicker in lockstep - stable across
        // frames without storing anything, same trick sand uses.
        uint32_t h = hash2(x, y);
        float phase = (float)(h % 1000) * 0.01f;
        float flicker = sinf(phase + t * 14.0f) * 0.5f + 0.5f;
        return fireColor((uint8_t)(80 + flicker * 175));
    });
}



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
    int baseY = (PANEL_HEIGHT - blockHeight()) / 2;
    if (baseY < 0) baseY = 0;
    float t = frame.progress * 6.2831853f * 3.0f;  // 3 full cycles over the effect's duration

    drawMorphingBlock(frame, baseY, [&](int x, int y) {
        float dx = x - 32.0f, dy = y - 32.0f;
        float v = sinf(x * 0.15f + t) +
                  sinf(y * 0.15f + t * 1.3f) +
                  sinf((x + y) * 0.1f + t * 0.7f) +
                  sinf(sqrtf(dx * dx + dy * dy) * 0.15f - t * 1.5f);
        float hue = (v + 4.0f) / 8.0f;
        return hueToRgb565(hue);
    });
}



void drawClock(const ClockFrame& frame) {
    switch (frame.anim) {
        case ClockAnim::SAND:   drawSandTransition(frame); return;
        case ClockAnim::FIRE:   drawFireTransition(frame); return;
        case ClockAnim::PLASMA: drawPlasmaTransition(frame); return;
        default: break;
    }

    // Resting state only - the quadrant grid isn't part of any transition
    // (nothing "transitions" about the date/temp/turkey), so this is
    // taller than blockHeight() (digits+AM/PM), which sand/fire/plasma
    // above still center against unchanged.
    int restingHeight = 5 * CLOCK_SIZE + BLOCK_GAP + quadBlockHeight();
    int y = (PANEL_HEIGHT - restingHeight) / 2;
    if (y < 0) y = 0;
    drawClockBlock(frame, y);
}
