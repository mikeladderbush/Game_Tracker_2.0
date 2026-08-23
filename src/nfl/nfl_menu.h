#pragma once
#include "nfl_api_client.h"

// Renders one matchup (schedule.games[pageIndex]) full-screen, stacked
// vertically with exactly 1px between each block (see the *_Y constants in
// the .cpp): away abbreviation (2x size), "VS.", home abbreviation (2x),
// favorite+spread, over/under - each line centered horizontally, drawn
// directly in team colors with no background/border, each letter
// alternating between the team's two colors. Letter and line spacing are
// both trimmed to actual ink (see draw_tools.cpp's glyphInkBounds), not
// each glyph's declared bounding box, so gaps land on a true 1px regardless
// of which letters happen to be adjacent. Caller is responsible for
// cycling pageIndex through [0, schedule.count) over time - this function
// only draws a single page, it doesn't do any timing/paging itself. Laid
// out for the confirmed 64x64 panel (see display.cpp's 5 address pins).
void drawScheduleList(const NflWeekSchedule& schedule, int pageIndex);
