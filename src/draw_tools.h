#pragma once
#include <cstdint>
#include <cstddef>
#include <Adafruit_Protomatter.h>
#include "nba_teams.h"

extern Adafruit_Protomatter matrix;

struct GlyphEntry {
    char ch;
    uint8_t width;
    uint8_t height;
    const uint8_t* pattern;
};

extern const GlyphEntry GLYPH_TABLE[];
extern const size_t GLYPH_TABLE_SIZE;

void drawSprite(int x, int y, uint8_t width, uint8_t height, uint8_t size, const uint8_t* pattern, const uint16_t* palette, uint8_t bgIndex = 0, bool drawBg = true);
void drawChar(char ch, int x, int y, uint8_t size);
void drawLogo(const NbaTeam& team, int x, int y, int homeOrAway);
void drawScore(int homeScore, int awayScore);
void drawGameClock(const char* mmss);
void drawScheduleTime(const char* hMmAmPm);
void drawQuarter(int quarter);
void drawDate(const char* gameDate);
void drawFutureGame(const char* gameDate, const char* gameTime, const NbaTeam& team, const NbaTeam& opponent);
void drawCityMenu();
void drawSelector(int menuIdx);

inline void clearArea(int x, int y, int w, int h, uint16_t color = 0x0000) {
    matrix.fillRect(x, y, w, h, color);
}