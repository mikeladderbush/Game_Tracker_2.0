#include "http_fetch.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

static bool fetchJson(HTTPClient& http, const char* authHeader, JsonDocument& outDoc) {
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
        return false;
    }

    DeserializationError err = deserializeJson(outDoc, http.getStream());
    http.end();
    if (err) {
        Serial.printf("fetchJson: JSON parse error: %s\n", err.c_str());
        return false;
    }
    return true;
}

bool fetchJsonPlain(const char* url, JsonDocument& outDoc) {
    WiFiClient client;
    HTTPClient http;
    if (!http.begin(client, url)) {
        Serial.println("fetchJson: http.begin failed");
        return false;
    }
    return fetchJson(http, nullptr, outDoc);
}

bool fetchJsonSecure(const char* url, const char* authHeader, JsonDocument& outDoc) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    if (!http.begin(client, url)) {
        Serial.println("fetchJson: http.begin failed");
        return false;
    }
    return fetchJson(http, authHeader, outDoc);
}
