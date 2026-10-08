/*
********************************************************************************

    clock_state.cpp.

********************************************************************************
*/

#include "clock_state.h"
#include <Arduino.h>
#include <cstdlib>
#include <cstring>
#include "../common/ntp_time.h"

void ClockState::refresh() {
    String now = getCurrentTime();  // ::getCurrentTime() in common/ntp_time.h
    int hour = atoi(now.c_str());

    if (lastHour_ != -1 && hour != lastHour_) {
        strncpy(prevBuf_, buf_, sizeof(prevBuf_) - 1);
        prevBuf_[sizeof(prevBuf_) - 1] = '\0';
        animating_ = true;
        animStartMs_ = millis();
    }
    lastHour_ = hour;

    strncpy(buf_, now.c_str(), sizeof(buf_) - 1);
    buf_[sizeof(buf_) - 1] = '\0';
}

void ClockState::begin() {
    refresh();
    lastUpdateMs_ = millis();
}

void ClockState::update() {
    unsigned long now = millis();

    if (animating_ && now - animStartMs_ >= ANIM_DURATION_MS) {
        animating_ = false;
    }

    if (now - lastUpdateMs_ >= UPDATE_INTERVAL_MS) {
        refresh();
        lastUpdateMs_ = now;
    }
}

ClockFrame ClockState::frame() const {
    ClockFrame f;
    strncpy(f.text, buf_, sizeof(f.text) - 1);
    f.text[sizeof(f.text) - 1] = '\0';

    f.animating = animating_;
    if (animating_) {
        strncpy(f.prevText, prevBuf_, sizeof(f.prevText) - 1);
        f.prevText[sizeof(f.prevText) - 1] = '\0';
        unsigned long elapsed = millis() - animStartMs_;
        f.progress = elapsed >= ANIM_DURATION_MS ? 1.0f : (float)elapsed / (float)ANIM_DURATION_MS;
    }
    return f;
}
