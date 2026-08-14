#pragma once
#include <cstdint>
#include <cstddef>
#include <Adafruit_Protomatter.h>
#include "team_sprite.h"
#include "glyph_data.h"

extern Adafruit_Protomatter matrix;

void drawSprite(int x, int y, uint8_t width, uint8_t height, uint8_t size, const uint8_t* pattern, const uint16_t* palette, uint8_t bgIndex = 0, bool drawBg = true);
void drawChar(char ch, int x, int y, uint8_t size);
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
