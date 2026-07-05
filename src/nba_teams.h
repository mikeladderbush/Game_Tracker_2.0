#pragma once
#include <cstdint>

struct NbaTeam {
    const char* teamName;     
    const uint8_t* pattern;   
    const uint16_t* palette;  
    uint8_t width;
    uint8_t height;
};

extern const NbaTeam hawks, celtics, nets, hornets, bulls, cavaliers, pistons,
    pacers, heat, bucks, knicks, magic, sixers, raptors, wizards, mavericks,
    nuggets, warriors, rockets, clippers, lakers, grizzlies, timberwolves,
    pelicans, thunder, suns, trailBlazers, kings, spurs, jazz;

const NbaTeam* teamFromString(const char* name);
const NbaTeam* teamFromAbbr(const char* abbr);