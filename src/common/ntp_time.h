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
String convertUtcToEst(const String& timeStrHHMM);
