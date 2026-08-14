#include "nfl_team_colors.h"
#include <string.h>

// clang-format off
const NflTeamColor NFL_TEAM_COLORS[] = {
    {"ARI", 0xa004, 0xffff},  // #a40227 / #ffffff
    {"ATL", 0xa0c6, 0x0000},  // #a71930 / #000000
    {"BAL", 0x288d, 0x0000},  // #29126f / #000000
    {"BUF", 0x0191, 0xd041},  // #00338d / #d50a0a
    {"CAR", 0x0439, 0x0000},  // #0085ca / #000000
    {"CHI", 0x08e7, 0xe200},  // #0b1c3a / #e64100
    {"CIN", 0xfa62, 0x0000},  // #fb4f14 / #000000
    {"CLE", 0x4141, 0xf9e0},  // #472a08 / #ff3c00
    {"DAL", 0x014b, 0xb5b7},  // #002a5c / #b0b7bc
    {"DEN", 0x0908, 0xfa60},  // #0a2343 / #fc4c02
    {"DET", 0x03b6, 0xbdd7},  // #0076b6 / #bbbbbb
    {"GB",  0x2266, 0xfda2},  // #204e32 / #ffb612
    {"HOU", 0x0083, 0xe805},  // #021018 / #eb0028
    {"IND", 0x01ce, 0xffff},  // #003b75 / #ffffff
    {"JAX", 0x03b0, 0xd505},  // #007487 / #d7a22a
    {"KC",  0xe0c6, 0xfda2},  // #e31837 / #ffb612
    {"LV",  0x0000, 0xa575},  // #000000 / #a5acaf
    {"LAC", 0x0418, 0xfe01},  // #0080c6 / #ffc20e
    {"LAR", 0x01b2, 0xfe80},  // #003594 / #ffd100
    {"MIA", 0x0472, 0xfa60},  // #008e97 / #fc4c02
    {"MIN", 0x4930, 0xfe25},  // #4f2683 / #ffc62f
    {"NE",  0x014b, 0xc066},  // #002a5c / #c60c30
    {"NO",  0xd5f1, 0x0000},  // #d3bc8d / #000000
    {"NYG", 0x01ef, 0xc927},  // #003c7f / #c9243f
    {"NYJ", 0x12a8, 0xffff},  // #115740 / #ffffff
    {"PHI", 0x0209, 0x0000},  // #06424d / #000000
    {"PIT", 0x0000, 0xfda2},  // #000000 / #ffb612
    {"SF",  0xa800, 0xb4cb},  // #aa0000 / #b3995d
    {"SEA", 0x014b, 0x6de5},  // #002a5c / #69be28
    {"TB",  0xb8e6, 0x39c6},  // #bd1c36 / #3e3a35
    {"TEN", 0x44ba, 0x00a6},  // #4495d2 / #001532
    {"WSH", 0x58a2, 0xfda2},  // #5a1414 / #ffb612
};
// clang-format on

const int NFL_TEAM_COLOR_COUNT = sizeof(NFL_TEAM_COLORS) / sizeof(NFL_TEAM_COLORS[0]);

NflTeamColor nflColorForAbbr(const char* abbr) {
    for (int i = 0; i < NFL_TEAM_COLOR_COUNT; i++) {
        if (strcmp(NFL_TEAM_COLORS[i].abbr, abbr) == 0) return NFL_TEAM_COLORS[i];
    }
    return {abbr, 0xffff, 0xffff};
}
