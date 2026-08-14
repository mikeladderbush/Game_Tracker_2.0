#include "nba_team_ids.h"
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
