#include "nfl_menu.h"
#include "nfl_team_colors.h"
#include "../common/draw_tools.h"
#include <cstdio>

static const int PANEL_WIDTH = 64;
static const uint16_t WHITE = 0xffff;
static const uint8_t ABBR_SIZE = 2;
static const uint8_t TEXT_SIZE = 1;

static void drawCentered(const char* text, int y, uint8_t size, uint16_t color) {
    int x = (PANEL_WIDTH - textWidth(text, size)) / 2;
    if (x < 0) x = 0;
    drawText(text, x, y, size, color);
}

// A few teams' ESPN "primary" color is pure black, which would be invisible
// text on this display's black background - fall back to their secondary
// color in that case.
static uint16_t visibleColor(const NflTeamColor& c) {
    return (c.primary == 0x0000) ? c.secondary : c.primary;
}

void drawScheduleList(const NflWeekSchedule& schedule, int pageIndex) {
    if (schedule.count == 0) {
        drawText("NO NFL DATA", 2, 14, TEXT_SIZE, WHITE);
        return;
    }
    if (pageIndex < 0 || pageIndex >= schedule.count) return;
    const NflMatchup& game = schedule.games[pageIndex];

    NflTeamColor away = nflColorForAbbr(game.awayAbbr);
    NflTeamColor home = nflColorForAbbr(game.homeAbbr);

    drawCentered(game.awayAbbr, 2, ABBR_SIZE, visibleColor(away));
    drawCentered("VS.", 15, TEXT_SIZE, WHITE);
    drawCentered(game.homeAbbr, 23, ABBR_SIZE, visibleColor(home));

    if (game.hasOdds) {
        char line1[16];
        snprintf(line1, sizeof(line1), "%s -%.1f", game.favoriteAbbr, game.spread);
        drawCentered(line1, 38, TEXT_SIZE, WHITE);

        char line2[16];
        snprintf(line2, sizeof(line2), "OU %.1f", game.overUnder);
        drawCentered(line2, 46, TEXT_SIZE, WHITE);
    } else {
        drawCentered("ODDS TBD", 38, TEXT_SIZE, WHITE);
    }

    char pageLabel[16];
    snprintf(pageLabel, sizeof(pageLabel), "%d OF %d", pageIndex + 1, schedule.count);
    drawCentered(pageLabel, 56, TEXT_SIZE, WHITE);
}
