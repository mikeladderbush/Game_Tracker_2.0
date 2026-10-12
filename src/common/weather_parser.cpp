#include "weather.h"

WeatherResult parseWeather(const JsonDocument& doc) {
    WeatherResult result;
    JsonVariantConst temp = doc["current"]["temperature_2m"];
    if (temp.isNull()) return result;
    result.found = true;
    result.tempF = temp.as<float>();
    return result;
}
