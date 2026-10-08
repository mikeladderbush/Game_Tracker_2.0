/*
********************************************************************************

    TeamSprite - shared logo format, any league.

********************************************************************************
*/

#pragma once
#include <cstdint>

struct TeamSprite {
    const char* teamName;
    const uint8_t* pattern;
    const uint16_t* palette;
    uint8_t width;
    uint8_t height;
};
