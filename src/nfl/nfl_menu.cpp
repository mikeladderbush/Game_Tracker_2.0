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
static const uint8_t ABBR_SIZE = 2;
static const uint8_t VS_SIZE = 1;
static const uint8_t ODDS_SIZE = 2;

// Every Y below leaves exactly one blank row between blocks (e.g. away
// ends at row 16, VS starts at 18 - row 17 is the 1px gap), computed from
// each block's actual rendered height (size * 5, glyph height) rather than
// guessed. Centered vertically as a whole: 49px of content in the 64px
// panel leaves 15px to split, 7 above/8 below.
static const int AWAY_Y = 7;
static const int VS_Y = 18;
static const int HOME_Y = 24;
static const int SPREAD_Y = 35;
static const int OU_Y = 46;
static const int NO_ODDS_Y = 40;

static void drawCentered(const char* text, int y, uint8_t size, uint16_t color) {
    int x = (PANEL_WIDTH - textWidth(text, size)) / 2;
    if (x < 0) x = 0;
    drawText(text, x, y, size, color);
}

static void drawCenteredAlternating(const char* text, int y, uint8_t size, uint16_t colorA, uint16_t colorB) {
    int x = (PANEL_WIDTH - textWidth(text, size)) / 2;
    if (x < 0) x = 0;
    drawTextAlternating(text, x, y, size, colorA, colorB);
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
    // color; each line centered horizontally via textWidth().
    drawCenteredAlternating(game.awayAbbr, AWAY_Y, ABBR_SIZE, away.primary, away.secondary);
    drawCentered("VS.", VS_Y, VS_SIZE, WHITE);
    drawCenteredAlternating(game.homeAbbr, HOME_Y, ABBR_SIZE, home.primary, home.secondary);

    if (game.hasOdds) {
        char spreadText[16];
        snprintf(spreadText, sizeof(spreadText), "%s -%.1f", game.favoriteAbbr, game.spread);
        drawCentered(spreadText, SPREAD_Y, ODDS_SIZE, WHITE);

        char ouText[10];
        snprintf(ouText, sizeof(ouText), "OU%.1f", game.overUnder);
        drawCentered(ouText, OU_Y, ODDS_SIZE, WHITE);
    } else {
        drawCentered("ODDS TBD", NO_ODDS_Y, VS_SIZE, WHITE);
    }
}
