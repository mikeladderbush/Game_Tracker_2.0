#include "draw_tools.h"
#include "draw_formatting.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

static const uint16_t GLYPH_PALETTE[3] = {0x0000, 0x0000, 0xffff};

void drawSprite(int x, int y, uint8_t width, uint8_t height, uint8_t size, const uint8_t* pattern, const uint16_t* palette, uint8_t bgIndex, bool drawBg) {
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            uint8_t idx = pattern[row * width + col];
            if (idx == 0 && !drawBg) continue;

            uint16_t color = palette[idx == 0 ? bgIndex : idx];
            int px = x + col * size;
            int py = y + row * size;
            if (size == 1) {
                matrix.drawPixel(px, py, color);
            } else {
                matrix.fillRect(px, py, size, size, color);
            }
        }
    }
}

static const GlyphEntry* findGlyph(char ch) {
    for (size_t i = 0; i < GLYPH_TABLE_SIZE; i++){
        if (GLYPH_TABLE[i].ch == ch) return &GLYPH_TABLE[i];
    }
    return nullptr;
}

void drawChar(char ch, int x, int y, uint8_t size){
    const GlyphEntry* g = findGlyph(ch);
    if (!g) return;
    drawSprite(x, y, g->width, g->height, size, g->pattern, GLYPH_PALETTE, 0, true);
}

void drawCharColored(char ch, int x, int y, uint8_t size, uint16_t color) {
    const GlyphEntry* g = findGlyph(ch);
    if (!g) return;
    uint16_t palette[3] = {0x0000, 0x0000, color};
    drawSprite(x, y, g->width, g->height, size, g->pattern, palette, 0, false);
}

int drawText(const char* text, int x, int y, uint8_t size, uint16_t color) {
    int cursorX = x;
    for (const char* p = text; *p; p++) {
        const GlyphEntry* g = findGlyph(*p);
        int advance = (g ? g->width + 1 : 4) * size;  // +1px spacing between glyphs
        drawCharColored(*p, cursorX, y, size, color);
        cursorX += advance;
    }
    return cursorX - x;
}

void drawLogo(const TeamSprite& team, int x, int y, int homeOrAway){
    int drawX = (homeOrAway == 1) ? x + 32 : x;
    drawSprite(drawX, y, team.width, team.height, 1, team.pattern, team.palette, 0, true);
}

static void drawScoreDigits(int score, int tensX, int onesX, int hundredsX) {
    ScoreDigits d = scoreToDigits(score);

    if (d.hasHundreds) {
        drawChar('0' + d.hundreds, hundredsX, 48, 2);
        drawChar('0' + d.tens, tensX, 48 ,2);
        drawChar('0' + d.ones, onesX, 48, 2);
    } else {
        drawChar('0' + d.tens, tensX, 48, 2);
        drawChar('0' + d.ones, onesX, 48, 2);
    }
}

void drawScore(int homeScore, int awayScore) {
    drawScoreDigits(homeScore, 8, 16, 0);
    drawScoreDigits(awayScore, 45, 53, 37);
}

void drawQuarter(int quarter) {
    const char* label = quarterLabel(quarter);
    size_t len = strlen(label);
    if (len == 0) return;

    // Hand-tuned pixel x-positions per label shape, preserved exactly from
    // the original per-case layout ("1st".."4th" use wider spacing than
    // the 2-char "HT"/"OT" and 3-char "OT2".."OT9" labels).
    static const int POS_QUARTER[3] = {25, 30, 33};  // "1st","2nd","3rd","4th"
    static const int POS_HALF_OT[2] = {28, 32};       // "HT","OT"
    static const int POS_OT_NUM[3]  = {25, 29, 33};   // "OT2".."OT9"

    const int* xs;
    if (len == 2) {
        xs = POS_HALF_OT;
    } else if (quarter >= 6) {
        xs = POS_OT_NUM;
    } else {
        xs = POS_QUARTER;
    }

    for (size_t i = 0; i < len; i++) {
        drawChar(label[i], xs[i], 48, 1);
    }
}

void drawGameClock(const char* mmss) {
    int minutes, seconds;
    if (!parseMMSS(mmss, minutes, seconds)) return;

    if (minutes >= 10) {
        drawChar('0' + (minutes / 10), 9, 34, 2);
    } else {
        drawChar(' ', 9, 34, 2);  // blank_number
    }
    drawChar('0' + (minutes % 10), 17, 34, 2);
    drawChar(':', 25, 34, 2);
    drawChar('0' + (seconds / 10), 31, 34, 2);
    drawChar('0' + (seconds % 10), 39, 34, 2);
}

void drawScheduleTime(const char* hMmAmPm) {
    int hours, minutes;
    bool isPM;
    if (!parseScheduleTime(hMmAmPm, hours, minutes, isPM)) return;

    if (hours >= 10) {
        drawChar('0' + (hours / 10), 9, 34, 2);
    } else {
        drawChar(' ', 9, 34, 2);
    }
    drawChar('0' + (hours % 10), 17, 34, 2);
    drawChar(':', 25, 34, 2);
    drawChar('0' + (minutes / 10), 31, 34, 2);
    drawChar('0' + (minutes % 10), 39, 34, 2);
    drawChar(isPM ? 'p' : 'a', 47, 39, 1);
}

void drawDate(const char* gameDate) {
    int year, month, day;
    if (!parseDateYMD(gameDate, year, month, day)) return;
    year = year % 100;

    int monthTens = month / 10, monthOnes = month % 10;
    int dayTens = day / 10, dayOnes = day % 10;
    int yearTens = year / 10, yearOnes = year % 10;

    if (monthTens > 0) drawChar('0' + monthTens, 1, 50, 2);
    drawChar('0' + monthOnes, 7, 50, 2);
    drawChar('-', 17, 53, 1);

    if (dayTens > 0) drawChar('0' + dayTens, 20, 50, 2);
    drawChar('0' + dayOnes, 28, 50, 2);
    drawChar('-', 39, 53, 1);

    if (yearTens > 0) drawChar('0' + yearTens, 43, 50, 2);
    drawChar('0' + yearOnes, 51, 50, 2);
}

void drawFutureGame(const char* gameDate, const char* gameTime,
                     const TeamSprite& team, const TeamSprite& opponent) {
    drawLogo(team, 0, 0, 0);
    drawLogo(opponent, 0, 0, 1);
    drawDate(gameDate);
    drawScheduleTime(gameTime);
}
