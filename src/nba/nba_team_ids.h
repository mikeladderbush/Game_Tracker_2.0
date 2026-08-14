#pragma once

// balldontlie's numeric team ID, needed to build getNextGame()'s request
// URL. Pure lookup table, no network/ESP32 dependency, split out from
// nba_api_client.h/.cpp so it can be compiled and unit tested natively -
// same reasoning as common/time_formatting.h et al.
int teamNameToId(const char* name);
