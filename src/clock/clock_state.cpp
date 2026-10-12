/*
********************************************************************************

    clock_state.cpp.

********************************************************************************
*/

#include "clock_state.h"
#include <Arduino.h>
#include <cstring>
#include <cstdio>
#include <cmath>
#include "../common/ntp_time.h"
#include "../common/weather.h"

void ClockState::refresh() {
    String now = getCurrentTime();  // ::getCurrentTime() in common/ntp_time.h

    int hour24, minute;
    if (getLocalHourMinute(hour24, minute)) {
        if (lastMinute_ != -1 && minute != lastMinute_) {
            ClockAnim next = ClockAnim::NONE;
            if (minute == 0) next = ClockAnim::PLASMA;
            else if (minute == 30) next = ClockAnim::FIRE;
            else if (minute == 15 || minute == 45) next = ClockAnim::SAND;

            if (next != ClockAnim::NONE) {
                strncpy(prevBuf_, buf_, sizeof(prevBuf_) - 1);
                prevBuf_[sizeof(prevBuf_) - 1] = '\0';
                anim_ = next;
                animStartMs_ = millis();
            }
        }
        lastMinute_ = minute;
    }

    strncpy(buf_, now.c_str(), sizeof(buf_) - 1);
    buf_[sizeof(buf_) - 1] = '\0';
}

void ClockState::refreshWeather() {
    WeatherResult result = fetchWeather();
    // On failure, keep showing the last good reading rather than blanking
    // it - same choice NFL's NflScheduleState makes on a failed fetch.
    if (result.found) {
        snprintf(tempBuf_, sizeof(tempBuf_), "%dF", (int)roundf(result.tempF));
    }
}

void ClockState::begin() {
    refresh();
    refreshWeather();
    lastUpdateMs_ = millis();
    lastWeatherMs_ = millis();
}

unsigned long ClockState::animDurationMs() const {
    switch (anim_) {
        case ClockAnim::SAND: return SAND_DURATION_MS;
        case ClockAnim::FIRE: return FIRE_DURATION_MS;
        case ClockAnim::PLASMA: return PLASMA_DURATION_MS;
        default: return 0;
    }
}

void ClockState::update() {
    unsigned long now = millis();

    if (anim_ != ClockAnim::NONE && now - animStartMs_ >= animDurationMs()) {
        anim_ = ClockAnim::NONE;
    }

    if (now - lastUpdateMs_ >= UPDATE_INTERVAL_MS) {
        refresh();
        lastUpdateMs_ = now;
    }

    if (now - lastWeatherMs_ >= WEATHER_INTERVAL_MS) {
        refreshWeather();
        lastWeatherMs_ = now;
    }
}

ClockFrame ClockState::frame() const {
    ClockFrame f;
    strncpy(f.text, buf_, sizeof(f.text) - 1);
    f.text[sizeof(f.text) - 1] = '\0';
    strncpy(f.temp, tempBuf_, sizeof(f.temp) - 1);
    f.temp[sizeof(f.temp) - 1] = '\0';

    f.anim = anim_;
    if (anim_ != ClockAnim::NONE) {
        strncpy(f.prevText, prevBuf_, sizeof(f.prevText) - 1);
        f.prevText[sizeof(f.prevText) - 1] = '\0';
        unsigned long elapsed = millis() - animStartMs_;
        unsigned long dur = animDurationMs();
        f.progress = elapsed >= dur ? 1.0f : (float)elapsed / (float)dur;
    }
    return f;
}
