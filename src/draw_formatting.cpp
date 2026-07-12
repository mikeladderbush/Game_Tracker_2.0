#include "draw_formatting.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>

ScoreDigits scoreToDigits(int score) {
    ScoreDigits d;
    d.hundreds = (score / 100) % 10;
    d.tens = (score / 10) % 10;
    d.ones = score % 10;
    d.hasHundreds = d.hundreds > 0;
    return d;
}

const char* quarterLabel(int quarter) {
    switch (quarter) {
        case 1: return "1st";
        case 2: return "2nd";
        case 3: return "3rd";
        case 4: return "4th";
        case -1: return "HT";
        case 5: return "OT";
        default:
            if (quarter > 5) {
                static char buf[5];
                int otNum = quarter - 4;
                snprintf(buf, sizeof(buf), "OT%d", otNum % 10);
                return buf;
            }
            return "";
    }
}

bool parseMMSS(const char* mmss, int& minutes, int& seconds) {
    if (!mmss || strlen(mmss) == 0) return false;
    const char* colon = strchr(mmss, ':');
    if (!colon) return false;

    minutes = atoi(mmss);
    seconds = atoi(colon + 1);
    return true;
}

bool parseScheduleTime(const char* hMmAmPm, int& hours, int& minutes, bool& isPM) {
    if (!hMmAmPm || strlen(hMmAmPm) == 0) return false;

    char buf[24];
    strncpy(buf, hMmAmPm, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char* et = strstr(buf, " ET");
    if (et) memmove(et, et + 3, strlen(et + 3) + 1);
    char* est = strstr(buf, " EST");
    if (est) memmove(est, est + 4, strlen(est + 4) + 1);
    char* edt = strstr(buf, " EDT");
    if (edt) memmove(edt, edt + 4, strlen(edt + 4) + 1);

    char* colon = strchr(buf, ':');
    if (!colon) return false;

    hours = atoi(buf);
    minutes = atoi(colon + 1);
    isPM = (strstr(buf, "PM") != nullptr || strstr(buf, "pm") != nullptr);
    return true;
}

bool parseDateYMD(const char* gameDate, int& year, int& month, int& day) {
    if (!gameDate) return false;
    return sscanf(gameDate, "%d-%d-%d", &year, &month, &day) == 3;
}
