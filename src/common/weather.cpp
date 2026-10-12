#include "weather.h"
#include "http_fetch.h"
#include <cstdio>

// Boston, MA. Change here if the board moves.
static const double LATITUDE = 42.3601;
static const double LONGITUDE = -71.0589;

JsonDocument fetchWeatherJson() {
    JsonDocument doc;
    char url[160];
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
             "&current=temperature_2m&temperature_unit=fahrenheit",
             LATITUDE, LONGITUDE);
    fetchJsonSecure(url, nullptr, doc);
    return doc;
}

WeatherResult fetchWeather() {
    JsonDocument doc = fetchWeatherJson();
    return parseWeather(doc);
}
