#pragma once
#include <Arduino.h>

// Pure parsing/formatting logic. Needs Arduino's String type but nothing
// network- or ESP32-specific, so it can be compiled and unit tested natively
// (String comes from a native Arduino-core shim there).

int clockStrToSecs(const String& clockStr);
String secsToMMSS(int totalSeconds);

// Same conversion convertUtcToEst() (ntp_time.h) does, with the DST offset
// passed in explicitly instead of read from the live clock - the real
// DST-detection needs a synced RTC and can't run natively, but the
// conversion math itself (wraparound, AM/PM, formatting) can be fully
// tested through this entry point.
String convertUtcToEstWithOffset(const String& timeStrHHMM, int offsetHours);
