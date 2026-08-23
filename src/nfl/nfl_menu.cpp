#include "nfl_menu.h"
#include "nfl_team_colors.h"
#include "../common/draw_tools.h"
#include <cstdio>
#include <cstring>

static const int PANEL_WIDTH = 64;
static const int MARGIN = 2;
static const uint16_t WHITE = 0xffff;

// The renderer only supports integer pixel scaling (each logical pixel
// becomes an NxN block - see drawSprite), so "1.5x" isn't literally
// achievable. ODDS_SIZE=1 is the nearest step below ABBR_SIZE=2, preserving
// the requested size hierarchy without a fractional-scaling renderer this
// project doesn't have (and which would look inconsistent on a font this
// small anyway).
static const uint8_t ABBR_SIZE = 2;
static const uint8_t ODDS_SIZE = 1;

static void drawCentered(const char* text, int y, uint8_t size, uint16_t color) {
    int x = (PANEL_WIDTH - textWidth(text, size)) / 2;
    if (x < 0) x = 0;
    drawText(text, x, y, size, color);
}

void drawScheduleList(const NflWeekSchedule& schedule, int pageIndex) {
    if (schedule.count == 0) {
        drawText("NO NFL DATA", MARGIN, 14, ODDS_SIZE, WHITE);
        return;
    }
    if (pageIndex < 0 || pageIndex >= schedule.count) return;
    const NflMatchup& game = schedule.games[pageIndex];

    NflTeamColor away = nflColorForAbbr(game.awayAbbr);
    NflTeamColor home = nflColorForAbbr(game.homeAbbr);

    // Horizontal top row, margined: away left, home right-aligned so it
    // never runs off the edge regardless of abbreviation length. No "VS."
    // here - two abbreviations at 2x size plus a separator doesn't fit
    // (two 3-letter abbreviations alone can reach 60-64px), so left/right
    // position is what conveys away vs. home. Each letter alternates
    // between the team's two colors (e.g. Patriots' "NE" - N red, E blue).
    drawTextAlternating(game.awayAbbr, MARGIN, 2, ABBR_SIZE, away.primary, away.secondary);
    int homeX = PANEL_WIDTH - MARGIN - textWidth(game.homeAbbr, ABBR_SIZE);
    drawTextAlternating(game.homeAbbr, homeX, 2, ABBR_SIZE, home.primary, home.secondary);

    if (game.hasOdds) {
        char spreadText[16];
        snprintf(spreadText, sizeof(spreadText), "%s -%.1f", game.favoriteAbbr, game.spread);
        drawCentered(spreadText, 16, ODDS_SIZE, WHITE);

        char ouText[10];
        snprintf(ouText, sizeof(ouText), "OU%.1f", game.overUnder);
        drawCentered(ouText, 23, ODDS_SIZE, WHITE);
    } else {
        drawCentered("ODDS TBD", 16, ODDS_SIZE, WHITE);
    }
}
