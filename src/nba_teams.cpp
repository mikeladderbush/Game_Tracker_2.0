#include "nba_teams.h"
#include <Arduino.h>
#include <string.h>

struct NameEntry { const char* key; const NbaTeam* team; };

static const NameEntry NAME_TABLE[] = {
    {"hawks", &hawks}, {"celtics", &celtics}, {"nets", &nets}, {"hornets", &hornets},
    {"bulls", &bulls}, {"cavaliers", &cavaliers}, {"mavericks", &mavericks},
    {"nuggets", &nuggets}, {"pistons", &pistons}, {"warriors", &warriors},
    {"rockets", &rockets}, {"pacers", &pacers}, {"lakers", &lakers},
    {"clippers", &clippers}, {"grizzlies", &grizzlies}, {"heat", &heat},
    {"bucks", &bucks}, {"timberwolves", &timberwolves}, {"pelicans", &pelicans},
    {"knicks", &knicks}, {"thunder", &thunder}, {"magic", &magic},
    {"sixers", &sixers}, {"suns", &suns}, {"blazers", &trailBlazers},
    {"kings", &kings}, {"spurs", &spurs}, {"raptors", &raptors},
    {"jazz", &jazz}, {"wizards", &wizards},
};

const NbaTeam* teamFromString(const char* name) {
    String lower = String(name);
    lower.toLowerCase();

    if (lower == "76ers") return &sixers;

    for (auto& entry : NAME_TABLE) {
        if (lower == entry.key) return entry.team;
    }

    int lastSpace = lower.lastIndexOf(' ');
    if (lastSpace >= 0) {
        String lastWord = lower.substring(lastSpace + 1);
        for (auto& entry : NAME_TABLE) {
            if (lastWord == entry.key) return entry.team;
        }
    }

    return nullptr;
}

struct AbbrEntry { const char* abbr; const NbaTeam* team; };

static const AbbrEntry ABBR_TABLE[] = {
    {"ATL", &hawks}, {"BKN", &nets}, {"BOS", &celtics}, {"CHA", &hornets},
    {"CHI", &bulls}, {"CLE", &cavaliers}, {"DAL", &mavericks}, {"DEN", &nuggets},
    {"DET", &pistons}, {"GSW", &warriors}, {"HOU", &rockets}, {"IND", &pacers},
    {"LAC", &clippers}, {"LAL", &lakers}, {"MEM", &grizzlies}, {"MIA", &heat},
    {"MIL", &bucks}, {"MIN", &timberwolves}, {"NOP", &pelicans}, {"NYK", &knicks},
    {"OKC", &thunder}, {"ORL", &magic}, {"PHI", &sixers}, {"PHX", &suns},
    {"POR", &trailBlazers}, {"SAC", &kings}, {"SAS", &spurs}, {"TOR", &raptors},
    {"UTA", &jazz}, {"WAS", &wizards},
};

const NbaTeam* teamFromAbbr(const char* abbr) {
    for (auto& entry : ABBR_TABLE) {
        if (strcmp(entry.abbr, abbr) == 0) return entry.team;
    }
    return nullptr;
}