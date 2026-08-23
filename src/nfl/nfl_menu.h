#pragma once
#include "nfl_api_client.h"

// Renders one matchup (schedule.games[pageIndex]) full-screen: away/home
// team abbreviations at 2x size, drawn directly in team colors with no
// background/border, each letter alternating between the team's two
// colors - away left-margined, home right-margined (no "VS." - doesn't
// fit alongside two 2x-size abbreviations, so left/right position is what
// conveys away vs. home). Favorite+spread and over/under below, smaller
// and tightly stacked. Caller is responsible for cycling pageIndex through
// [0, schedule.count) over time - this function only draws a single page,
// it doesn't do any timing/paging itself. Laid out for the confirmed 64x64
// panel (see display.cpp's 5 address pins).
void drawScheduleList(const NflWeekSchedule& schedule, int pageIndex);
