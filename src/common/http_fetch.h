#pragma once
#include <ArduinoJson.h>

// ArduinoJson 7's default nesting limit is 10. ESPN's NFL scoreboard nests 15
// deep (measured 2026-09-26) and values skipped by a filter still count, so
// the default aborts the parse inside the first game. Well above the measured
// depth, but still bounded so a pathological response can't overflow the
// task's stack.
constexpr size_t JSON_NESTING_LIMIT = 24;

// Generic HTTP(S)-GET-a-JSON-document helpers. Only know about a URL, which
// client to speak plaintext/TLS with, and an optional auth header - nothing
// about any specific API's response shape - so any sport/data source's
// fetch layer can be built on top of these.
//
// filter (optional): an ArduinoJson filter document - only the fields it names
// are kept while streaming, so a response far bigger than the ESP32's heap
// (ESPN's NFL scoreboard is ~280 KB) can still be parsed.
//
// On any failure outDoc is cleared, never left half-filled.
bool fetchJsonPlain(const char* url, JsonDocument& outDoc, const JsonDocument* filter = nullptr);
bool fetchJsonSecure(const char* url, const char* authHeader, JsonDocument& outDoc, const JsonDocument* filter = nullptr);
