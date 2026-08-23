#include "nfl_menu.h"
#include "nfl_team_colors.h"
#include "../common/draw_tools.h"
#include <cstdio>

static const int PANEL_WIDTH = 64;
static const int MARGIN = 2;
static const uint16_t WHITE = 0xffff;

// True "1.5x" isn't achievable - the renderer only supports integer
// pixel-block scaling (see drawSprite) - so ODDS_SIZE lands on the nearest
// available step.
static const uint8_t ABBR_SIZE = 3;
static const uint8_t VS_SIZE = 1;
static const uint8_t ODDS_SIZE = 2;

static void drawCentered(const char* text, int y, uint8_t size, uint16_t color) {
    int x = (PANEL_WIDTH - textWidth(text, size)) / 2;
    if (x < 0) x = 0;
    drawText(text, x, y, size, color);
}

void drawScheduleList(const NflWeekSchedule& schedule, int pageIndex) {
    if (schedule.count == 0) {
        drawText("NO NFL DATA", MARGIN, 14, VS_SIZE, WHITE);
        return;
    }
    if (pageIndex < 0 || pageIndex >= schedule.count) return;
    const NflMatchup& game = schedule.games[pageIndex];

    NflTeamColor away = nflColorForAbbr(game.awayAbbr);
    NflTeamColor home = nflColorForAbbr(game.homeAbbr);

    // Vertical stack: away, "VS.", home, then odds below. Each letter
    // alternates between the team's two colors (e.g. Patriots' "NE" - N
    // navy, E red) rather than the whole abbreviation being one solid
    // color; centered horizontally via textWidth().
    int awayX = (PANEL_WIDTH - textWidth(game.awayAbbr, ABBR_SIZE)) / 2;
    if (awayX < 0) awayX = 0;
    drawTextAlternating(game.awayAbbr, awayX, 1, ABBR_SIZE, away.primary, away.secondary);

    drawCentered("VS.", 18, VS_SIZE, WHITE);

    int homeX = (PANEL_WIDTH - textWidth(game.homeAbbr, ABBR_SIZE)) / 2;
    if (homeX < 0) homeX = 0;
    drawTextAlternating(game.homeAbbr, homeX, 25, ABBR_SIZE, home.primary, home.secondary);

    if (game.hasOdds) {
        char spreadText[16];
        snprintf(spreadText, sizeof(spreadText), "%s -%.1f", game.favoriteAbbr, game.spread);
        drawCentered(spreadText, 43, ODDS_SIZE, WHITE);

        char ouText[10];
        snprintf(ouText, sizeof(ouText), "OU%.1f", game.overUnder);
        drawCentered(ouText, 54, ODDS_SIZE, WHITE);
    } else {
        drawCentered("ODDS TBD", 46, VS_SIZE, WHITE);
    }
}
