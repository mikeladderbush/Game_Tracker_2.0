#pragma once
#include "nfl_api_client.h"

// TODO (NFL_SUPPORT_ROADMAP.md steps 8-9): the schedule-list display isn't
// designed yet - this is a new kind of view (multiple games at once), not
// an extension of nba/nba_menu.cpp's single-team-selection grid, and won't
// all fit on a 64x32 display without some paging/scrolling scheme. Signature
// below is a placeholder guess, not a committed design.
void drawScheduleList(const NflWeekSchedule& schedule, int scrollOffset);
