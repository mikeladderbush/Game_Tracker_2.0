#pragma once
#include "../common/team_sprite.h"

extern const TeamSprite hawks, celtics, nets, hornets, bulls, cavaliers, pistons,
    pacers, heat, bucks, knicks, magic, sixers, raptors, wizards, mavericks,
    nuggets, warriors, rockets, clippers, lakers, grizzlies, timberwolves,
    pelicans, thunder, suns, trailBlazers, kings, spurs, jazz;

const TeamSprite* nbaTeamFromString(const char* name);
const TeamSprite* nbaTeamFromAbbr(const char* abbr);
