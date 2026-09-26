#include "http_fetch.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

static bool fetchJson(HTTPClient& http, const char* authHeader, JsonDocument& outDoc, const JsonDocument* filter) {
    http.useHTTP10(true);
    if (authHeader) {
        http.addHeader("Authorization", authHeader);
    } else {
        http.addHeader("User-Agent", "Mozilla/5.0 (compatible; ESP32)");
    }

    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("fetchJson: GET failed, code %d\n", httpCode);
        http.end();
        outDoc.clear();
        return false;
    }

    DeserializationError err = filter
        ? deserializeJson(outDoc, http.getStream(), DeserializationOption::Filter(*filter),
                          DeserializationOption::NestingLimit(JSON_NESTING_LIMIT))
        : deserializeJson(outDoc, http.getStream(), DeserializationOption::NestingLimit(JSON_NESTING_LIMIT));
    http.end();
    if (err) {
        Serial.printf("fetchJson: JSON parse error: %s\n", err.c_str());
        outDoc.clear();
        return false;
    }
    return true;
}

bool fetchJsonPlain(const char* url, JsonDocument& outDoc, const JsonDocument* filter) {
    WiFiClient client;
    HTTPClient http;
    if (!http.begin(client, url)) {
        Serial.println("fetchJson: http.begin failed");
        return false;
    }
    return fetchJson(http, nullptr, outDoc, filter);
}

bool fetchJsonSecure(const char* url, const char* authHeader, JsonDocument& outDoc, const JsonDocument* filter) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    if (!http.begin(client, url)) {
        Serial.println("fetchJson: http.begin failed");
        return false;
    }
    return fetchJson(http, authHeader, outDoc, filter);
}
