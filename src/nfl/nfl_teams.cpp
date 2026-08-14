#include "nfl_teams.h"

// TODO: mirror nba/nba_teams.cpp's NAME_TABLE/ABBR_TABLE lookup pattern once
// nfl_team_data.cpp has real TeamSprite instances to point at.

const TeamSprite* nflTeamFromString(const char* name) {
    (void)name;
    return nullptr;
}

const TeamSprite* nflTeamFromAbbr(const char* abbr) {
    (void)abbr;
    return nullptr;
}
