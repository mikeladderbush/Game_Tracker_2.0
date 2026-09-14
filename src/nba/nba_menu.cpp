#include "nba_menu.h"
#include "../common/draw_tools.h"
#include <cstddef>

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
