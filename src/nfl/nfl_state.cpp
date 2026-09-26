#include "nfl_state.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

void NflScheduleState::refetch() {
    // No week/seasonType/year passed - fetchNflWeekScheduleJson() resolves
    // to the actual current week server-side.
    JsonDocument doc = fetchNflWeekScheduleJson();
    NflWeekSchedule fresh = parseNflSchedule(doc);
    // A failed fetch parses to zero games - keep showing the last good week
    // rather than blanking the screen over a transient network error.
    if (fresh.count > 0) schedule_ = fresh;
    if (pageIndex_ >= schedule_.count) pageIndex_ = 0;
    Serial.printf("NFL: fetched %d games (week %d), showing %d; stack headroom %u bytes\n",
                  fresh.count, fresh.weekNumber, schedule_.count,
                  (unsigned)uxTaskGetStackHighWaterMark(nullptr));
}

void NflScheduleState::begin() {
    refetch();
    lastFetchMs_ = millis();
    lastPageMs_ = millis();
}

void NflScheduleState::update() {
    unsigned long now = millis();

    unsigned long fetchInterval = schedule_.count > 0 ? FETCH_INTERVAL_MS : RETRY_INTERVAL_MS;
    if (now - lastFetchMs_ > fetchInterval) {
        refetch();
        lastFetchMs_ = now;
    }

    if (schedule_.count > 0 && now - lastPageMs_ > PAGE_INTERVAL_MS) {
        pageIndex_ = (pageIndex_ + 1) % schedule_.count;
        lastPageMs_ = now;
    }
}
