#include "nfl_team_colors.h"
#include <string.h>

// clang-format off
const NflTeamColor NFL_TEAM_COLORS[] = {
    {"ARI", 0x9107, 0xffff},  // Cardinal Red / White
    {"ATL", 0xa0c6, 0xa575},  // Red / Silver
    {"BAL", 0x20ae, 0x9be1},  // Purple / Metallic Gold
    {"BUF", 0x0191, 0xc066},  // Royal Blue / Red
    {"CAR", 0x0439, 0xa575},  // Panther Blue / Silver
    {"CHI", 0x08a5, 0xc9c0},  // Navy / Orange
    {"CIN", 0xfa62, 0xffff},  // Orange / White
    {"CLE", 0xf9e0, 0x30e0},  // Orange / Brown
    {"DAL", 0x00e8, 0xb5b7},  // Navy / Metallic Silver
    {"DEN", 0xfa62, 0x0108},  // Broncos Orange / Navy
    {"DET", 0x03b6, 0xb5b7},  // Honolulu Blue / Silver
    {"GB",  0x21a6, 0xfda2},  // Green / Gold
    {"HOU", 0xa0c6, 0x0105},  // Battle Red / Deep Steel Blue
    {"IND", 0x016b, 0xffff},  // Colts Blue / White
    {"JAX", 0x032f, 0xd505},  // Teal / Gold
    {"KC",  0xe0c6, 0xfda2},  // Red / Gold
    {"LV",  0xa575, 0xffff},  // Silver / White
    {"LAC", 0x0418, 0xfe01},  // Powder Blue / Gold
    {"LAR", 0x01b2, 0xfe80},  // Royal Blue / Gold
    {"MIA", 0x0472, 0xfa60},  // Aqua / Orange
    {"MIN", 0x4930, 0xfe25},  // Purple / Gold
    {"NE",  0xc066, 0x0108},  // Red / Nautical Navy
    {"NO",  0xd5f1, 0xffff},  // Old Gold / White
    {"NYG", 0x090c, 0xa0c6},  // Blue / Red
    {"NYJ", 0x12a8, 0xffff},  // Gotham Green / White
    {"PHI", 0x026a, 0xa575},  // Midnight Green / Silver
    {"PIT", 0xfda2, 0xffff},  // Gold / White
    {"SF",  0xa800, 0xb4cb},  // Red / Gold
    {"SEA", 0x6de5, 0x0108},  // Action Green / College Navy
    {"TB",  0xd041, 0x3185},  // Buccaneer Red / Pewter
    {"TEN", 0x4c9b, 0xc885},  // Titans Blue / Red
    {"WSH", 0x58a2, 0xfda2},  // Burgundy / Gold
};
// clang-format on

const int NFL_TEAM_COLOR_COUNT = sizeof(NFL_TEAM_COLORS) / sizeof(NFL_TEAM_COLORS[0]);

NflTeamColor nflColorForAbbr(const char* abbr) {
    for (int i = 0; i < NFL_TEAM_COLOR_COUNT; i++) {
        if (strcmp(NFL_TEAM_COLORS[i].abbr, abbr) == 0) return NFL_TEAM_COLORS[i];
    }
    return {abbr, 0xffff, 0xffff};
}
