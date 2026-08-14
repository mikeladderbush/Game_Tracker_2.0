#pragma once
#include <cstdint>
#include <cstddef>
#include <Adafruit_Protomatter.h>
#include "team_sprite.h"
#include "glyph_data.h"

extern Adafruit_Protomatter matrix;

void drawSprite(int x, int y, uint8_t width, uint8_t height, uint8_t size, const uint8_t* pattern, const uint16_t* palette, uint8_t bgIndex = 0, bool drawBg = true);
void drawChar(char ch, int x, int y, uint8_t size);

// Single-color glyph draw with background pixels skipped (not painted black),
// so it composites over whatever's already drawn - e.g. text on top of a
// pre-filled colored rectangle. drawText draws a whole string left-to-right
// using each glyph's actual declared width (the font is variable-width),
// and returns the total pixel width drawn, useful for layout/centering.
void drawCharColored(char ch, int x, int y, uint8_t size, uint16_t color);
int drawText(const char* text, int x, int y, uint8_t size, uint16_t color);
void drawLogo(const TeamSprite& team, int x, int y, int homeOrAway);
void drawScore(int homeScore, int awayScore);
void drawGameClock(const char* mmss);
void drawScheduleTime(const char* hMmAmPm);
void drawQuarter(int quarter);
void drawDate(const char* gameDate);
void drawFutureGame(const char* gameDate, const char* gameTime, const TeamSprite& team, const TeamSprite& opponent);

inline void clearArea(int x, int y, int w, int h, uint16_t color = 0x0000) {
    matrix.fillRect(x, y, w, h, color);
}
