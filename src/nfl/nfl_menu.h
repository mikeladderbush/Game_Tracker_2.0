#pragma once
#include "nfl_api_client.h"

// Renders one matchup (schedule.games[pageIndex]) full-screen, stacked
// vertically: away abbreviation (2x size), "VS.", home abbreviation (2x),
// favorite+spread, over/under - each abbreviation centered horizontally,
// drawn directly in team colors with no background/border, each letter
// alternating between the team's two colors. Caller is responsible for
// cycling pageIndex through [0, schedule.count) over time - this function
// only draws a single page, it doesn't do any timing/paging itself. Laid
// out for the confirmed 64x64 panel (see display.cpp's 5 address pins).
void drawScheduleList(const NflWeekSchedule& schedule, int pageIndex);
