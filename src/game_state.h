#pragma once
#include "api_client.h"

struct GameFrame {
    char teamName[32] = {0};     
    int homeScore = 0;
    int awayScore = 0;
    char opponent[32] = {0};
    char clock[16] = {0};         
    char gameTimeText[32] = {0};
    int gameStatus = 0;
    int period = 0;
};

struct ScheduledGame {
    bool valid = false;
    char dateStr[16] = {0};
    char timeStr[16] = {0};
    char teamFullName[32] = {0};
    char opponentFullName[32] = {0};
};

class GameStateMachine {
public:
    void begin(const char* teamShortName);

    void update();

    bool isInGame() const { return inGame_; }
    const GameFrame& currentFrame() const { return frame_; }
    const ScheduledGame& scheduledGame() const { return scheduled_; }

private:
    void pollLiveGame();
    void pollScheduled();
    void applyFetchResult(const FetchResult& result);
    void refreshScheduledInfo();
    const char* apiTeamName() const; // handles Sixers -> 76ers mapping

    char teamShortName_[32] = {0};
    bool inGame_ = false;

    GameFrame frame_;
    ScheduledGame scheduled_;

    int targetSecs_ = -1;
    int displaySecs_ = -1;

    unsigned long lastApiCallMs_ = 0;
    unsigned long nextTickMs_ = 0;

    static const unsigned long LIVE_POLL_MS = 20000;
    static const unsigned long SCHED_POLL_MS = 300000;
    static const int DELAY_SECS = 30;
};