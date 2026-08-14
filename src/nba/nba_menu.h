#pragma once

// The team-selection menu shown before a game starts. Its layout (grid
// column/row positions, 9/9/9/3 split) is hand-tuned to exactly 30 NBA
// teams, so unlike the rest of the drawing code this isn't sport-agnostic -
// an NFL menu (32 teams) would need its own layout, not a reuse of this one.

void drawCityMenu();
void drawSelector(int menuIdx);
