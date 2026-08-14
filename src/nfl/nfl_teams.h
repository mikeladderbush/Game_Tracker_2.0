#pragma once
#include "../common/team_sprite.h"

// TODO (NFL_SUPPORT_ROADMAP.md step 10): no team sprite/palette data exists
// yet - authoring it is a large manual task, deliberately deferred. Until
// then these lookups have nothing to find and will always return nullptr.
// Add `extern const TeamSprite cardinals, falcons, ...;` here once
// nfl_team_data.cpp has real entries, mirroring nba/nba_teams.h.

const TeamSprite* nflTeamFromString(const char* name);
const TeamSprite* nflTeamFromAbbr(const char* abbr);
