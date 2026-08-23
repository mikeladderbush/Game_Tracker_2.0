#pragma once
#include <cstdint>

// Team accent colors (RGB565), for drawing an abbreviation in team colors
// without needing full pixel-art logo data (see nfl_teams.h/nfl_team_data.cpp
// for that separate, much larger, still-undone task). Hand-curated "classic"
// colors per team, not ESPN's team.color/alternateColor fields - those were
// used in an earlier pass but didn't always match how a fan would describe
// a team's colors (e.g. ESPN's Patriots "color" is navy, not red).
//
// Neither primary nor secondary is ever pure black: nfl_menu.cpp's
// drawTextAlternating() draws each letter of an abbreviation directly in
// one or the other, so a team whose "real" second color is black (Raiders,
// Steelers, etc.) has it swapped for a neutral stand-in (usually white) -
// otherwise every other letter would be invisible against the display's
// black background. Merely dark colors (navy, deep steel, brown) are fine
// left as-is - on an emissive LED panel a dim pixel still reads as visibly
// "on" against an off one, unlike ink-on-paper contrast.
struct NflTeamColor {
    const char* abbr;
    uint16_t primary;
    uint16_t secondary;
};

extern const NflTeamColor NFL_TEAM_COLORS[];
extern const int NFL_TEAM_COLOR_COUNT;

// Falls back to {abbr, white, white} if abbr isn't recognized, rather than
// a null pointer - callers always get something drawable.
NflTeamColor nflColorForAbbr(const char* abbr);
