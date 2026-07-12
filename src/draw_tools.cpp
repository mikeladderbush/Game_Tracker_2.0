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

void drawLogo(const NbaTeam& team, int x, int y, int homeOrAway){
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
                     const NbaTeam& team, const NbaTeam& opponent) {
    drawLogo(team, 0, 0, 0);
    drawLogo(opponent, 0, 0, 1);
    drawDate(gameDate);
    drawScheduleTime(gameTime);
}
 
struct MenuChar { char ch; int x; int y; };
static const MenuChar CITY_MENU[] = {
    // ATL
    {'A',2,0},{'T',7,0},{'L',11,0},
    // BKN
    {'B',3,7},{'K',7,7},{'N',11,7},
    // BOS
    {'B',3,14},{'O',7,14},{'S',11,14},
    // CHA
    {'C',3,21},{'H',7,21},{'A',10,21},
    // CHI
    {'C',3,28},{'H',7,28},{'I',11,28},
    // CLE
    {'C',3,35},{'L',7,35},{'E',11,35},
    // DAL
    {'D',3,42},{'A',6,42},{'L',11,42},
    // DEN
    {'D',3,49},{'E',7,49},{'N',11,49},
    // DET
    {'D',3,56},{'E',7,56},{'T',11,56},
    // GSW
    {'G',18,0},{'S',22,0},{'W',25,0},
    // HOU
    {'H',18,7},{'O',22,7},{'U',26,7},
    // IND
    {'I',18,14},{'N',22,14},{'D',27,14},
    // LAC
    {'L',18,21},{'A',21,21},{'C',26,21},
    // LAL
    {'L',18,28},{'A',21,28},{'L',26,28},
    // MEM
    {'M',18,35},{'E',23,35},{'M',27,35},
    // MIA
    {'M',18,42},{'I',23,42},{'A',27,42},
    // MIL
    {'M',18,49},{'I',23,49},{'L',27,49},
    // MIN
    {'M',18,56},{'I',23,56},{'N',27,56},
    // NOP
    {'N',36,0},{'O',41,0},{'P',45,0},
    // NYK
    {'N',36,7},{'Y',40,7},{'K',44,7},
    // OKC
    {'O',36,14},{'K',40,14},{'C',44,14},
    // ORL
    {'O',36,21},{'R',40,21},{'L',44,21},
    // PHI
    {'P',36,28},{'H',40,28},{'I',44,28},
    // PHX
    {'P',36,35},{'H',40,35},{'X',44,35},
    // POR
    {'P',36,42},{'O',40,42},{'R',44,42},
    // SAC
    {'S',36,49},{'A',39,49},{'C',44,49},
    // SAS
    {'S',36,56},{'A',39,56},{'S',44,56},
    // TOR
    {'T',50,0},{'O',54,0},{'R',58,0},
    // UTA
    {'U',50,7},{'T',54,7},{'A',57,7},
    // WAS
    {'W',50,14},{'A',55,14},{'S',60,14},
};
static const size_t CITY_MENU_SIZE = sizeof(CITY_MENU) / sizeof(CITY_MENU[0]);
 
void drawCityMenu() {
    for (size_t i = 0; i < CITY_MENU_SIZE; i++) {
        drawChar(CITY_MENU[i].ch, CITY_MENU[i].x, CITY_MENU[i].y, 1);
    }
}
 
void drawSelector(int menuIdx) {
    static const int COL_X[4] = {0, 16, 34, 48};
    static const int COL_ROWS[4] = {9, 9, 9, 3};
    const int Y_STEP = 7;
 
    for (int col = 0; col < 4; col++) {
        for (int row = 0; row < COL_ROWS[col]; row++) {
            clearArea(COL_X[col], row * Y_STEP, 2, 5);
        }
    }
 
    int col, row;
    if (menuIdx < 9) { col = 0; row = menuIdx; }
    else if (menuIdx < 18) { col = 1; row = menuIdx - 9; }
    else if (menuIdx < 27) { col = 2; row = menuIdx - 18; }
    else { col = 3; row = menuIdx - 27; }
 
    drawChar('_', COL_X[col], row * Y_STEP, 1);
}
 
