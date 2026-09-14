#include "nfl_state.h"
#include <Arduino.h>

void NflScheduleState::refetch() {
    // No week/seasonType/year passed - fetchNflWeekScheduleJson() resolves
    // to the actual current week server-side.
    JsonDocument doc = fetchNflWeekScheduleJson();
    schedule_ = parseNflSchedule(doc);
    if (pageIndex_ >= schedule_.count) pageIndex_ = 0;
}

void NflScheduleState::begin() {
    refetch();
    lastFetchMs_ = millis();
    lastPageMs_ = millis();
}

void NflScheduleState::update() {
    unsigned long now = millis();

    if (now - lastFetchMs_ > FETCH_INTERVAL_MS) {
        refetch();
        lastFetchMs_ = now;
    }

    if (schedule_.count > 0 && now - lastPageMs_ > PAGE_INTERVAL_MS) {
        pageIndex_ = (pageIndex_ + 1) % schedule_.count;
        lastPageMs_ = now;
    }
}
