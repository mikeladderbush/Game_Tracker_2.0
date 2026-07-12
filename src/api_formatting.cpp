#include "api_formatting.h"
#include <cstdio>
#include <string.h>

struct TeamIdEntry { const char* name; int id; };
static const TeamIdEntry TEAM_IDS[] = {
    {"Hawks",1},{"Celtics",2},{"Nets",3},{"Hornets",4},{"Bulls",5},
    {"Cavaliers",6},{"Mavericks",7},{"Nuggets",8},{"Pistons",9},{"Warriors",10},
    {"Rockets",11},{"Pacers",12},{"Clippers",13},{"Lakers",14},{"Grizzlies",15},
    {"Heat",16},{"Bucks",17},{"Timberwolves",18},{"Pelicans",19},{"Knicks",20},
    {"Thunder",21},{"Magic",22},{"Sixers",23},{"Suns",24},{"Blazers",25},
    {"Kings",26},{"Spurs",27},{"Raptors",28},{"Jazz",29},{"Wizards",30},
};

int teamNameToId(const char* name) {
    for (auto& entry : TEAM_IDS) {
        if (strcmp(entry.name, name) == 0) return entry.id;
    }
    return -1;
}

int clockStrToSecs(const String& clockStr) {
    if (clockStr.length() == 0) return -1;

    if (clockStr.startsWith("PT")) {
        String s = clockStr.substring(2);
        int minutes = 0, seconds = 0;

        int mIdx = s.indexOf('M');
        String rest = s;
        if (mIdx >= 0) {
            minutes = s.substring(0, mIdx).toInt();
            rest = s.substring(mIdx + 1);
        }

        int sIdx = rest.indexOf('S');
        if (sIdx >= 0) {
            seconds = (int)rest.substring(0, sIdx).toFloat();
        }

        return minutes * 60 + seconds;
    }

    int colonIdx = clockStr.indexOf(':');
    if (colonIdx >= 0) {
        String minutesPart = clockStr.substring(0, colonIdx);
        String secondsPart = clockStr.substring(colonIdx + 1);

        int spaceIdx = secondsPart.indexOf(' ');
        if (spaceIdx >= 0) secondsPart = secondsPart.substring(0, spaceIdx);

        int dotIdx = secondsPart.indexOf('.');
        if (dotIdx >= 0) secondsPart = secondsPart.substring(0, dotIdx);

        int minutes = minutesPart.length() ? minutesPart.toInt() : 0;
        int seconds = secondsPart.length() ? secondsPart.toInt() : 0;
        return minutes * 60 + seconds;
    }

    return -1;
}

String secsToMMSS(int totalSeconds) {
    if (totalSeconds < 0) totalSeconds = 0;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d:%02d", minutes, seconds);
    return String(buf);
}

String convertUtcToEstWithOffset(const String& timeStrHHMM, int offsetHours) {
    int colonIdx = timeStrHHMM.indexOf(':');
    if (colonIdx < 0) return timeStrHHMM;

    int hours = timeStrHHMM.substring(0, colonIdx).toInt();
    String minutes = timeStrHHMM.substring(colonIdx + 1);

    hours = ((hours + offsetHours) % 24 + 24) % 24;

    const char* period = (hours < 12) ? "AM" : "PM";
    int hours12 = hours % 12;
    if (hours12 == 0) hours12 = 12;

    char buf[16];
    snprintf(buf, sizeof(buf), "%d:%s %s", hours12, minutes.c_str(), period);
    return String(buf);
}
