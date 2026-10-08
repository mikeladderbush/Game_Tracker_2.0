/*
********************************************************************************

    Pure clock/date parsing and formatting. No network/ESP32 dependency,
    unit tested natively.

********************************************************************************
*/

#pragma once
#include <Arduino.h>

int clockStrToSecs(const String& clockStr);
String secsToMMSS(int totalSeconds);

// Same conversion convertUtcToEst() (ntp_time.h) does, with the DST offset
// passed in explicitly instead of read from the live clock - the real
// DST-detection needs a synced RTC and can't run natively, but the
// conversion math itself (wraparound, AM/PM, formatting) can be fully
// tested through this entry point.
String convertUtcToEstWithOffset(const String& timeStrHHMM, int offsetHours);
