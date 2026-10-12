/*
********************************************************************************

    Current outdoor temperature via Open-Meteo - free, no API key.
    Boston, MA by request; no GPS on this board, so the location is a
    plain constant in weather.cpp, not auto-detected. Not a secret, so it
    lives here rather than in secrets.h.

********************************************************************************
*/

#pragma once
#include <ArduinoJson.h>

struct WeatherResult {
    bool found = false;
    float tempF = 0.0f;
};

// GET api.open-meteo.com/v1/forecast?latitude=...&longitude=...&current=temperature_2m&temperature_unit=fahrenheit
JsonDocument fetchWeatherJson();

// found=false if "current.temperature_2m" is absent - checked via isNull(),
// not a sentinel default, since 0F is a real Boston-winter reading, not a
// safe stand-in for "missing."
WeatherResult parseWeather(const JsonDocument& doc);

WeatherResult fetchWeather();
