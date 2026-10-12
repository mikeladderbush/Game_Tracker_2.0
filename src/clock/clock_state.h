/*
********************************************************************************

    Formats the synced RTC as "H:MMa"/"H:MMp", refreshed once a second.
    Flags an effect at each quarter-hour mark - see ClockFrame.

********************************************************************************
*/

#pragma once
#include <cstdint>

enum class ClockAnim : uint8_t { NONE, SAND, FIRE, PLASMA };

// What clock_menu.cpp needs for one frame. Static most of the time;
// anim != NONE for that effect's own duration right after the matching
// minute mark (see ClockState::refresh()).
struct ClockFrame {
    char text[8] = "--:--";
    char prevText[8] = "--:--";
    char temp[8] = "--";  // "62F"/"-5F" - last good Open-Meteo reading, see weather.h
    ClockAnim anim = ClockAnim::NONE;
    float progress = 0.0f;  // 0..1, only meaningful if anim != NONE
};

class ClockState {
public:
    void begin();
    void update();

    ClockFrame frame() const;

private:
    void refresh();
    void refreshWeather();
    unsigned long animDurationMs() const;

    char buf_[8] = "--:--";
    char prevBuf_[8] = "--:--";
    int lastMinute_ = -1;

    char tempBuf_[8] = "--";
    unsigned long lastWeatherMs_ = 0;

    ClockAnim anim_ = ClockAnim::NONE;
    unsigned long animStartMs_ = 0;

    unsigned long lastUpdateMs_ = 0;

    static const unsigned long UPDATE_INTERVAL_MS = 1000UL;
    // Open-Meteo's own "current" reading only updates server-side on
    // roughly this cadence - polling faster wouldn't get fresher data.
    static const unsigned long WEATHER_INTERVAL_MS = 900000UL;
    static const unsigned long SAND_DURATION_MS = 1200UL;
    static const unsigned long FIRE_DURATION_MS = 900UL;
    static const unsigned long PLASMA_DURATION_MS = 1800UL;
};
