#include <unity.h>
#include "common/weather.h"

void setUp() {}
void tearDown() {}

// Shaped like the real response - confirmed live against api.open-meteo.com,
// 2026-10-12.
static const char* JSON_REAL_SHAPE = R"JSON(
{
  "latitude": 40.710335,
  "longitude": -73.99308,
  "generationtime_ms": 0.0256,
  "utc_offset_seconds": 0,
  "timezone": "GMT",
  "timezone_abbreviation": "GMT",
  "elevation": 32.0,
  "current_units": {"time": "iso8601", "interval": "seconds", "temperature_2m": "°F"},
  "current": {"time": "2026-10-12T15:15", "interval": 900, "temperature_2m": 60.7}
}
)JSON";

void test_parses_temperature_from_real_shape() {
    JsonDocument doc;
    deserializeJson(doc, JSON_REAL_SHAPE);
    WeatherResult result = parseWeather(doc);
    TEST_ASSERT_TRUE(result.found);
    TEST_ASSERT_EQUAL_FLOAT(60.7f, result.tempF);
}

void test_missing_current_block_is_not_found() {
    JsonDocument doc;
    deserializeJson(doc, "{}");
    WeatherResult result = parseWeather(doc);
    TEST_ASSERT_FALSE(result.found);
}

static const char* JSON_NEGATIVE = R"JSON({"current": {"temperature_2m": -12.5}})JSON";

void test_negative_temperature_parses_correctly() {
    // Boston in January - must not choke on a real sub-zero reading.
    JsonDocument doc;
    deserializeJson(doc, JSON_NEGATIVE);
    WeatherResult result = parseWeather(doc);
    TEST_ASSERT_TRUE(result.found);
    TEST_ASSERT_EQUAL_FLOAT(-12.5f, result.tempF);
}

static const char* JSON_ZERO = R"JSON({"current": {"temperature_2m": 0.0}})JSON";

void test_zero_degrees_is_found_not_treated_as_missing() {
    // The real trap here: a naive `doc["current"]["temperature_2m"] | 0.0f`
    // default can't tell "field absent" from "field present and genuinely
    // 0" - parseWeather() checks isNull() specifically to avoid that.
    JsonDocument doc;
    deserializeJson(doc, JSON_ZERO);
    WeatherResult result = parseWeather(doc);
    TEST_ASSERT_TRUE(result.found);
    TEST_ASSERT_EQUAL_FLOAT(0.0f, result.tempF);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_parses_temperature_from_real_shape);
    RUN_TEST(test_missing_current_block_is_not_found);
    RUN_TEST(test_negative_temperature_parses_correctly);
    RUN_TEST(test_zero_degrees_is_found_not_treated_as_missing);
    return UNITY_END();
}
