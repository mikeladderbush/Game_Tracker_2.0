/*
********************************************************************************

    Generic HTTP(S)-GET-a-JSON-document layer. Plain or TLS, optional auth
    header, optional ArduinoJson filter. Nesting limit raised to 24 -
    ESPN's NFL response nests 15 deep, past ArduinoJson's default of 10.
    outDoc is cleared on any failure, never left half-filled.

********************************************************************************
*/

#pragma once
#include <ArduinoJson.h>

constexpr size_t JSON_NESTING_LIMIT = 24;

bool fetchJsonPlain(const char* url, JsonDocument& outDoc, const JsonDocument* filter = nullptr);
bool fetchJsonSecure(const char* url, const char* authHeader, JsonDocument& outDoc, const JsonDocument* filter = nullptr);
