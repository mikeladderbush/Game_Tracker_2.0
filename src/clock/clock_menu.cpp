/*
********************************************************************************

    clock_menu.cpp. Size 4 is the largest "HH:MM" fits at (57px of 64).

********************************************************************************
*/

#include "clock_menu.h"
#include "../common/draw_tools.h"

static const int PANEL_WIDTH = 64;
static const int PANEL_HEIGHT = 64;
static const uint16_t WHITE = 0xffff;
// Largest size "HH:MM" fits the 64px panel at with margin to spare
// (textWidth 57px at size 4; size 5 would be 70px, too wide) - confirmed via
// common/glyph_metrics.h's textWidth(), not guessed.
static const uint8_t CLOCK_SIZE = 4;

static void drawCenteredClock(const char* text, int y) {
    int w = textWidth(text, CLOCK_SIZE);
    int x = (PANEL_WIDTH - w) / 2;
    if (x < 0) x = 0;
    drawText(text, x, y, CLOCK_SIZE, WHITE);
}

void drawClock(const ClockFrame& frame) {
    int h = 5 * CLOCK_SIZE;  // glyph height (5) * size
    int centerY = (PANEL_HEIGHT - h) / 2;
    if (centerY < 0) centerY = 0;

    if (!frame.animating) {
        drawCenteredClock(frame.text, centerY);
        return;
    }

    // Both values move down together: old continues past center and off
    // the bottom edge, new enters from above the top edge and settles
    // into center. drawSprite clips off-panel rows, so no bounds check
    // needed here.
    int offset = (int)(frame.progress * PANEL_HEIGHT);
    drawCenteredClock(frame.prevText, centerY + offset);
    drawCenteredClock(frame.text, centerY + offset - PANEL_HEIGHT);
}
