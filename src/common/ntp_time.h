/*
********************************************************************************

    Wall-clock time: NTP sync + reading the RTC. Sport-agnostic.

********************************************************************************
*/

#pragma once
#include <Arduino.h>

bool syncTime();
String getCurrentDate();
String getCurrentTime();

// Raw local hour (0-23) / minute (0-59), for callers that need actual
// values rather than a display string - e.g. clock mode's minute-boundary
// detection. Returns false (hour/minute untouched) if the RTC isn't
// synced yet.
bool getLocalHourMinute(int& hour24, int& minute);

// Raw local month (1-12) / day (1-31), same shape as getLocalHourMinute.
bool getLocalMonthDay(int& month, int& day);

String convertUtcToEst(const String& timeStrHHMM);
