#include "nfl_menu.h"
#include "nfl_team_colors.h"
#include "../common/draw_tools.h"
#include <cstdio>
#include <cstring>

static const int CHIP_WIDTH = 20;
static const int CHIP_HEIGHT = 14;
static const int CHIP_Y = 4;
static const int AWAY_CHIP_X = 2;
static const int HOME_CHIP_X = 42;
static const uint16_t WHITE = 0xffff;

// White-bordered, team-colored abbreviation "chip".
static void drawTeamChip(int x, const char* abbr) {
    NflTeamColor color = nflColorForAbbr(abbr);

    clearArea(x - 1, CHIP_Y - 1, CHIP_WIDTH + 2, CHIP_HEIGHT + 2, WHITE);
    clearArea(x, CHIP_Y, CHIP_WIDTH, CHIP_HEIGHT, color.primary);
    drawText(abbr, x + 3, CHIP_Y + 4, 1, color.secondary);
}

void drawScheduleList(const NflWeekSchedule& schedule, int pageIndex) {
    if (pageIndex < 0 || pageIndex >= schedule.count) return;
    const NflMatchup& game = schedule.games[pageIndex];

    drawTeamChip(AWAY_CHIP_X, game.awayAbbr);
    drawText("VS.", 26, CHIP_Y + 4, 1, WHITE);
    drawTeamChip(HOME_CHIP_X, game.homeAbbr);

    if (game.hasOdds) {
        char line1[16];
        snprintf(line1, sizeof(line1), "%s -%.1f", game.favoriteAbbr, game.spread);
        drawText(line1, 4, 26, 1, WHITE);

        char line2[16];
        snprintf(line2, sizeof(line2), "OU %.1f", game.overUnder);
        drawText(line2, 4, 36, 1, WHITE);
    } else {
        drawText("ODDS TBD", 4, 26, 1, WHITE);
    }

    char pageLabel[16];
    snprintf(pageLabel, sizeof(pageLabel), "%d OF %d", pageIndex + 1, schedule.count);
    drawText(pageLabel, 4, 54, 1, WHITE);
}
