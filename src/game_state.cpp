#include "game_state.h"
#include <Arduino.h>
#include <string.h>

const char* GameStateMachine::apiTeamName() const{
    if (strcmp(teamShortName_, "Sixers") == 0) return "76ers";
    return teamShortName_;
}

void GameStateMachine::applyFetchResult(const FetchResult& result) {
    strlcpy(frame_.teamName, teamShortName_, sizeof(frame_.teamName));
    frame_.homeScore = result.homeScore;
    frame_.awayScore = result.awayScore;
    strlcpy(frame_.opponent, result.opponent, sizeof(frame_.opponent));
    strlcpy(frame_.gameTimeText, result.gameTimeText, sizeof(frame_.gameTimeText));
    frame_.gameStatus = result.gameStatus;
    frame_.period = result.period;
}

void GameStateMachine::refreshScheduledInfo() {
    NextGameResult next = getNextGame(teamShortName_);
    if (!next.found) return;

    scheduled_.valid = true;
    strlcpy(scheduled_.dateStr, next.dateStr, sizeof(scheduled_.dateStr));

    String timeStr = next.timeStr;
    if (timeStr == "Final") timeStr = "12:00";
    timeStr.toCharArray(scheduled_.timeStr, sizeof(scheduled_.timeStr));

    strlcpy(scheduled_.teamFullName, next.teamFullName, sizeof(scheduled_.teamFullName));
    strlcpy(scheduled_.opponentFullName, next.opponentFullName, sizeof(scheduled_.opponentFullName));
}

void GameStateMachine::begin(const char* teamShortName) {
    strlcpy(teamShortName_, teamShortName, sizeof(teamShortName_));

    FetchResult result = fetchGame(apiTeamName());

    inGame_ = result.found && (result.gameStatus == 1 || result.gameStatus == 2);

    if (inGame_) {
        applyFetchResult(result);
        targetSecs_ = clockStrToSecs(result.gameClock);
        displaySecs_ = (targetSecs_ >= 0) ? targetSecs_ : -1;
    } else if (result.found) {
        scheduled_.valid = true;
        strlcpy(scheduled_.dateStr, getCurrentDate().c_str(), sizeof(scheduled_.dateStr));
        strlcpy(scheduled_.timeStr, result.gameTimeText, sizeof(scheduled_.timeStr));
        strlcpy(scheduled_.teamFullName, teamShortName_, sizeof(scheduled_.teamFullName));
        strlcpy(scheduled_.opponentFullName, result.opponent, sizeof(scheduled_.opponentFullName));
    } else {
        refreshScheduledInfo();
    }

    lastApiCallMs_ = millis();
    nextTickMs_ = millis() + 1000;
}

void GameStateMachine::update() {
    if (inGame_){
        pollLiveGame();
    } else {
        pollScheduled();
    }
}

void GameStateMachine::pollLiveGame() {
    unsigned long now = millis();

    if (now - lastApiCallMs_ > LIVE_POLL_MS) {
        lastApiCallMs_ = now;

        FetchResult result = fetchGame(apiTeamName());

        if (!result.found || result.gameStatus < 1) {
            inGame_ = false;
            refreshScheduledInfo();
            return;
        }

        applyFetchResult(result);

        int apiSecs = clockStrToSecs(result.gameClock);
        if (apiSecs >= 0) {
            targetSecs_ = apiSecs;
            if (displaySecs_ < 0) {
                displaySecs_ = targetSecs_ + DELAY_SECS;
            }
        }
    }

    if (displaySecs_ >= 0 && targetSecs_ >= 0 && now >= nextTickMs_) {
        if (displaySecs_ > targetSecs_) {
            displaySecs_ -= 1;
        }
        nextTickMs_ += 1000;

        if (nextTickMs_ < now - 250) {
            nextTickMs_ = now + 750;
        }
    }

    if (displaySecs_ >= 0) {
        secsToMMSS(displaySecs_).toCharArray(frame_.clock, sizeof(frame_.clock));
    }
}

void GameStateMachine::pollScheduled() {
    unsigned long now = millis();

    if (now - lastApiCallMs_ <= SCHED_POLL_MS) return;
    lastApiCallMs_ = now;

    FetchResult result = fetchGame(apiTeamName());
    if (result.found && result.gameStatus >= 1) {
        applyFetchResult(result);
        targetSecs_ = clockStrToSecs(result.gameClock);
        displaySecs_ = (targetSecs_ >= 0) ? targetSecs_ + DELAY_SECS : DELAY_SECS;
        inGame_ = true;
        return;
    }

    refreshScheduledInfo();
}