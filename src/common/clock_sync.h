#pragma once

// Pure decision logic behind the clock catch-up fix (bug: displaySecs_ used
// to freeze instead of jumping forward at period/OT transitions). No
// Arduino/network dependency, so it's directly unit-testable natively.
int nextDisplaySeconds(int displaySecs, int apiSecs, int delaySecs);
