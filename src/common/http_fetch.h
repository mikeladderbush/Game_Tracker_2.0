#pragma once
#include <ArduinoJson.h>

// Generic HTTP(S)-GET-a-JSON-document helpers. Only know about a URL, which
// client to speak plaintext/TLS with, and an optional auth header - nothing
// about any specific API's response shape - so any sport/data source's
// fetch layer can be built on top of these.
bool fetchJsonPlain(const char* url, JsonDocument& outDoc);
bool fetchJsonSecure(const char* url, const char* authHeader, JsonDocument& outDoc);
