#pragma once
#include <cstdint>

// Generic team logo data: name plus an indexed sprite (pattern of palette
// indices) and the palette itself. Sport-agnostic - any league's team can be
// represented with this same shape, so drawing code only needs to know about
// TeamSprite, never about a specific league's team list.
struct TeamSprite {
    const char* teamName;
    const uint8_t* pattern;
    const uint16_t* palette;
    uint8_t width;
    uint8_t height;
};
