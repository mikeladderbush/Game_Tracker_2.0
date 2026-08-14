#pragma once
#include <Arduino.h>

// Wall-clock time sync (NTP) and date/EST formatting. Sport-agnostic - no
// league's data model is involved, just the device's live clock.

bool syncTime();
String getCurrentDate();
String convertUtcToEst(const String& timeStrHHMM);
