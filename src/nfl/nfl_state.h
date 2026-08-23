#pragma once
#include "nfl_api_client.h"

// Holds the current week's schedule and cycles which game is on screen.
// Unlike NBA's GameStateMachine there's no live clock to interpolate - just
// a slow-changing schedule (re-fetched hourly) and a page that advances
// every few seconds through however many games are loaded.
class NflScheduleState {
public:
    void begin();
    void update();

    const NflWeekSchedule& schedule() const { return schedule_; }
    int currentPage() const { return pageIndex_; }

private:
    void refetch();

    NflWeekSchedule schedule_;
    int pageIndex_ = 0;

    unsigned long lastFetchMs_ = 0;
    unsigned long lastPageMs_ = 0;

    // TODO: week/seasonType are placeholders (week 1, regular season) -
    // there's no real "what NFL week is it today" calculation yet. Fine for
    // now since TEST_SERVER's /fake_nfl_week ignores these params anyway;
    // needs real logic before this points at the live ESPN endpoint.
    static const int PLACEHOLDER_WEEK = 1;
    static const int PLACEHOLDER_SEASON_TYPE = 2;

    static const unsigned long FETCH_INTERVAL_MS = 3600000UL;  // schedule/odds don't need to refresh often
    static const unsigned long PAGE_INTERVAL_MS = 5000UL;
};
