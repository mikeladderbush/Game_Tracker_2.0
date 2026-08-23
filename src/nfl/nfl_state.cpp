#include "nfl_state.h"
#include "../common/ntp_time.h"
#include <Arduino.h>

void NflScheduleState::refetch() {
    int year = getCurrentDate().substring(0, 4).toInt();
    JsonDocument doc = fetchNflWeekScheduleJson(PLACEHOLDER_WEEK, PLACEHOLDER_SEASON_TYPE, year);
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
