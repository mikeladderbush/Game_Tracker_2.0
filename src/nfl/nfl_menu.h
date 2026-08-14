#pragma once
#include "nfl_api_client.h"

// Renders one matchup (schedule.games[pageIndex]) full-screen: away/home
// team abbreviations in team colors with a white border ("chips"), a "VS."
// separator, and the favorite/spread/over-under below in white. Caller is
// responsible for cycling pageIndex through [0, schedule.count) over time -
// this function only draws a single page, it doesn't do any timing/paging
// itself. Laid out for the confirmed 64x64 panel (see display.cpp's 5
// address pins). First-draft layout - not yet checked on real hardware.
void drawScheduleList(const NflWeekSchedule& schedule, int pageIndex);
