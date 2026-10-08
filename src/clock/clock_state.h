/*
********************************************************************************

    Formats the synced RTC as "HH:MM", refreshed once a second.
    Scrolls old/new value on an hour change - see ClockFrame.

********************************************************************************
*/

#pragma once

// What clock_menu.cpp needs for one frame. Static most of the time;
// animating for ANIM_DURATION_MS right after the hour changes.
struct ClockFrame {
    char text[6] = "--:--";
    bool animating = false;
    char prevText[6] = "--:--";
    float progress = 0.0f;  // 0..1, only meaningful if animating
};

class ClockState {
public:
    void begin();
    void update();

    ClockFrame frame() const;

private:
    void refresh();

    char buf_[6] = "--:--";
    char prevBuf_[6] = "--:--";
    int lastHour_ = -1;

    bool animating_ = false;
    unsigned long animStartMs_ = 0;

    unsigned long lastUpdateMs_ = 0;

    static const unsigned long UPDATE_INTERVAL_MS = 1000UL;
    static const unsigned long ANIM_DURATION_MS = 600UL;
};
