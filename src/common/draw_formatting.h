#pragma once

// Pure formatting/parsing logic pulled out of draw_tools.cpp. No Arduino or
// hardware (Adafruit_Protomatter) dependency, so this can be compiled and
// unit tested natively without a board.

struct ScoreDigits {
    int hundreds;
    int tens;
    int ones;
    bool hasHundreds;
};
ScoreDigits scoreToDigits(int score);

// Returns a label for the given quarter/period: "1st".."4th", "HT" (half,
// quarter == -1), "OT" (quarter == 5), "OT2".."OT9" (quarter > 5), or ""
// for anything unrecognized. The returned pointer is valid until the next
// call (OT2+ labels are built into a static buffer).
const char* quarterLabel(int quarter);

bool parseMMSS(const char* mmss, int& minutes, int& seconds);
bool parseScheduleTime(const char* hMmAmPm, int& hours, int& minutes, bool& isPM);
bool parseDateYMD(const char* gameDate, int& year, int& month, int& day);
