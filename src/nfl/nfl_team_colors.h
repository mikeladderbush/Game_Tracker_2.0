#pragma once
#include <cstdint>

// Team accent colors (RGB565), for drawing an abbreviation in team colors
// without needing full pixel-art logo data (see nfl_teams.h/nfl_team_data.cpp
// for that separate, much larger, still-undone task). Sourced directly from
// ESPN's own team.color/team.alternateColor fields
// (site.api.espn.com/apis/site/v2/sports/football/nfl/teams), so these match
// what ESPN itself considers each team's two colors - not always the same
// pairing a fan would call "primary/secondary" (e.g. ESPN's Patriots "color"
// is navy, not red), but consistent and real.
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
