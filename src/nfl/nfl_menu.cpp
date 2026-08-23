#include "nfl_menu.h"
#include "nfl_team_colors.h"
#include "../common/draw_tools.h"
#include <cstdio>
#include <cstring>

static const int PANEL_WIDTH = 64;
static const int MARGIN = 4;
static const uint16_t WHITE = 0xffff;
static const uint8_t ABBR_SIZE = 1;
static const uint8_t ODDS_SIZE = 2;

static void drawCentered(const char* text, int y, uint8_t size, uint16_t color) {
    int x = (PANEL_WIDTH - textWidth(text, size)) / 2;
    if (x < 0) x = 0;
    drawText(text, x, y, size, color);
}

void drawScheduleList(const NflWeekSchedule& schedule, int pageIndex) {
    if (schedule.count == 0) {
        drawText("NO NFL DATA", MARGIN, 14, ABBR_SIZE, WHITE);
        return;
    }
    if (pageIndex < 0 || pageIndex >= schedule.count) return;
    const NflMatchup& game = schedule.games[pageIndex];

    NflTeamColor away = nflColorForAbbr(game.awayAbbr);
    NflTeamColor home = nflColorForAbbr(game.homeAbbr);

    // Horizontal top row, margined: away left, home right-aligned so it
    // never runs off the edge regardless of abbreviation length. Each
    // letter alternates between the team's two colors (e.g. Patriots'
    // "NE" - N red, E blue) rather than the whole abbreviation being one
    // solid color.
    drawTextAlternating(game.awayAbbr, MARGIN, 4, ABBR_SIZE, away.primary, away.secondary);
    int homeX = PANEL_WIDTH - MARGIN - textWidth(game.homeAbbr, ABBR_SIZE);
    drawTextAlternating(game.homeAbbr, homeX, 4, ABBR_SIZE, home.primary, home.secondary);
    drawCentered("VS.", 4, ABBR_SIZE, WHITE);

    if (game.hasOdds) {
        // Spread sits under whichever side is favored (same left/right
        // convention as the abbreviation row above) instead of repeating
        // the team abbreviation - leaves room for the number to be big.
        char spreadText[8];
        snprintf(spreadText, sizeof(spreadText), "-%.1f", game.spread);
        bool homeFavored = strcmp(game.favoriteAbbr, game.homeAbbr) == 0;
        int spreadX = homeFavored
            ? PANEL_WIDTH - MARGIN - textWidth(spreadText, ODDS_SIZE)
            : MARGIN;
        drawText(spreadText, spreadX, 18, ODDS_SIZE, WHITE);

        char ouText[10];
        snprintf(ouText, sizeof(ouText), "OU%.1f", game.overUnder);
        drawCentered(ouText, 30, ODDS_SIZE, WHITE);
    } else {
        drawCentered("ODDS TBD", 20, ABBR_SIZE, WHITE);
    }
}
