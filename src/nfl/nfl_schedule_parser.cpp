#include "nfl_api_client.h"
#include <cstddef>
#include <cstring>

// strlcpy isn't standard C++ (Arduino/BSD-only) - fine on the ESP32 build,
// but this file also needs to compile natively for unit tests, so use a
// portable equivalent instead. Same truncate-and-always-null-terminate
// behavior.
static void copyBounded(char* dst, size_t dstSize, const char* src) {
    strncpy(dst, src, dstSize - 1);
    dst[dstSize - 1] = '\0';
}

// Pure JSON -> struct logic, split from nfl_api_client.cpp (which needs
// ESP32-only networking) so this half can be compiled and unit tested
// natively - same reasoning as nba/nba_team_ids.cpp.

// Every field parseNflSchedule() reads below must be named here, or it never
// survives the filtered fetch. (An array filter's [0] applies to every element.)
void buildNflScheduleFilter(JsonDocument& filter) {
    filter["week"]["number"] = true;

    JsonObject event = filter["events"][0].to<JsonObject>();
    event["id"] = true;
    event["date"] = true;
    event["status"]["type"]["state"] = true;

    JsonObject comp = event["competitions"][0].to<JsonObject>();
    JsonObject competitor = comp["competitors"][0].to<JsonObject>();
    competitor["homeAway"] = true;
    competitor["team"]["abbreviation"] = true;

    JsonObject odds = comp["odds"][0].to<JsonObject>();
    odds["spread"] = true;
    odds["overUnder"] = true;
    odds["homeTeamOdds"]["favorite"] = true;
}

NflWeekSchedule parseNflSchedule(const JsonDocument& doc) {
    NflWeekSchedule result;
    result.weekNumber = doc["week"]["number"] | 0;

    JsonArrayConst events = doc["events"].as<JsonArrayConst>();
    for (JsonObjectConst event : events) {
        if (result.count >= 16) break;

        NflMatchup& m = result.games[result.count];

        const char* id = event["id"] | "";
        copyBounded(m.eventId, sizeof(m.eventId), id);

        const char* kickoff = event["date"] | "";
        copyBounded(m.kickoffIso, sizeof(m.kickoffIso), kickoff);

        const char* state = event["status"]["type"]["state"] | "";
        m.isFinal = strcmp(state, "post") == 0;

        JsonObjectConst comp = event["competitions"][0];

        JsonArrayConst competitors = comp["competitors"].as<JsonArrayConst>();
        for (JsonObjectConst c : competitors) {
            const char* homeAway = c["homeAway"] | "";
            const char* abbr = c["team"]["abbreviation"] | "";
            if (strcmp(homeAway, "home") == 0) {
                copyBounded(m.homeAbbr, sizeof(m.homeAbbr), abbr);
            } else {
                copyBounded(m.awayAbbr, sizeof(m.awayAbbr), abbr);
            }
        }

        JsonArrayConst odds = comp["odds"].as<JsonArrayConst>();
        if (!odds.isNull() && odds.size() > 0) {
            JsonObjectConst o = odds[0];
            float rawSpread = o["spread"] | 0.0f;
            bool homeFavorite = o["homeTeamOdds"]["favorite"] | false;

            m.hasOdds = true;
            m.overUnder = o["overUnder"] | 0.0f;
            m.spread = rawSpread < 0 ? -rawSpread : rawSpread;
            copyBounded(m.favoriteAbbr, sizeof(m.favoriteAbbr), homeFavorite ? m.homeAbbr : m.awayAbbr);
        }

        result.count++;
    }

    return result;
}
