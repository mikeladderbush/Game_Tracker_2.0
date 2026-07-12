#include "clock_sync.h"

int nextDisplaySeconds(int displaySecs, int apiSecs, int delaySecs) {
    if (displaySecs < 0 || apiSecs > displaySecs) {
        // First tick, or the game clock jumped up (new period/OT started) -
        // reseed instead of leaving displaySecs stuck below the new target.
        return apiSecs + delaySecs;
    }
    return displaySecs;
}
