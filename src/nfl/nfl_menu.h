#pragma once
#include "nfl_api_client.h"

// Renders one matchup (schedule.games[pageIndex]) full-screen: away/home
// team abbreviations drawn directly in team colors (no background/border) -
// away left-margined, home right-margined, "VS." centered between them -
// with the spread (under whichever side is favored) and over/under below,
// bigger and tightly stacked. Caller is responsible for cycling pageIndex
// through [0, schedule.count) over time - this function only draws a single
// page, it doesn't do any timing/paging itself. Laid out for the confirmed
// 64x64 panel (see display.cpp's 5 address pins).
void drawScheduleList(const NflWeekSchedule& schedule, int pageIndex);
