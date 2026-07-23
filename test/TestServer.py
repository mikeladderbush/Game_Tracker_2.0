"""
Quick local stand-in for NBA's live scoreboard endpoint, for testing the
render pipeline while cdn.nba.com 403s during the off-season.

Run: python3 test_server.py
Then point api_client.cpp's NBA_SCOREBOARD_URL at this machine's IP, port 5000.

Endpoints:
  GET /fake_clock          -> current fake game state, same JSON shape fetchGame() expects
  GET /fake_clock/tick      -> advance the game clock by 10 seconds (call from browser/curl)
  GET /fake_clock/score     -> bump the home score by 2 (quick way to see drawScore update)
  GET /fake_clock/period    -> advance to the next period (Q1-Q4, then OT1, OT2, ... uncapped -
                               used to reproduce/verify the period-transition clock catch-up fix)
  GET /fake_clock/reset     -> reset to a fresh Q1 tip-off state
  GET /fake_clock/fault/<mode> -> set a fault mode for the next /fake_clock response(s), to
                               exercise fetchGame()'s error paths end-to-end:
                                 none            - normal response (default)
                                 http500         - non-200 status code
                                 malformed_json  - 200 with truncated/invalid JSON body
                                 missing_fields  - 200 with valid JSON but score/clock/period
                                                   keys absent (defaults should kick in, not crash)
  GET /fake_clock/autoplay/<mode> -> autoplay is ON by default at startup - the game clock
                               counts down once per real second, occasionally stopping (dead
                               ball/foul), randomly awarding 1-3 point scores, and advancing
                               the quarter (resetting the clock) once it hits 0:00. Hit
                               .../autoplay/off to pause it (e.g. to manually drive state via
                               /fake_clock/tick|score|period), ".../on" to resume.
"""
import random
import threading
import time

from flask import Flask, jsonify, Response

app = Flask(__name__)

state_lock = threading.Lock()
state = {
    "home_team": "Celtics",
    "away_team": "Wizards",
    "home_score": 1,
    "away_score": 0,
    "period": 1,
    "clock_seconds": 12 * 60,  # 12:00
    "status": 2,               # 2 = live, matches gameStatus used in game_state.cpp
    "fault": "none",
    "autoplay": True,
}

STOPPAGE_CHANCE = 0.15   # dead ball/foul - clock doesn't move this tick
SCORE_CHANCE = 0.08      # chance of a basket on a tick where the clock does move

def _autoplay_loop():
    while True:
        time.sleep(1)
        with state_lock:
            if not (state["autoplay"] and state["status"] == 2):
                continue

            if state["clock_seconds"] <= 0:
                state["period"] += 1
                state["clock_seconds"] = 12 * 60 if state["period"] <= 4 else 5 * 60
                continue

            if random.random() < STOPPAGE_CHANCE:
                continue

            state["clock_seconds"] -= 1

            if random.random() < SCORE_CHANCE:
                points = random.choice([1, 2, 3])
                if random.random() < 0.5:
                    state["home_score"] += points
                else:
                    state["away_score"] += points

def clock_to_pt_duration(total_seconds):
    minutes = total_seconds // 60
    seconds = total_seconds % 60
    return f"PT{minutes}M{seconds:02d}.00S"

@app.route("/fake_clock")
def fake_clock():
    if state["fault"] == "http500":
        return Response("internal error", status=500)

    if state["fault"] == "malformed_json":
        # Deliberately truncated/broken - deserializeJson() should fail
        # cleanly, not crash.
        return Response('{"scoreboard": {"games": [{"gameStatus": 2,',
                         status=200, mimetype="application/json")

    if state["fault"] == "missing_fields":
        # Valid JSON, but score/gameClock/period/team fields absent -
        # fetchGame()'s `| default` fallbacks should kick in, not crash.
        return jsonify({
            "scoreboard": {
                "games": [{
                    "homeTeam": {"teamName": state["home_team"]},
                    "awayTeam": {"teamName": state["away_team"]},
                }]
            }
        })

    return jsonify({
        "scoreboard": {
            "games": [{
                "gameStatus": state["status"],
                "gameStatusText": "",
                "gameClock": clock_to_pt_duration(state["clock_seconds"]),
                "period": state["period"],
                "homeTeam": {
                    "teamName": state["home_team"],
                    "score": state["home_score"],
                },
                "awayTeam": {
                    "teamName": state["away_team"],
                    "score": state["away_score"],
                },
            }]
        }
    })

@app.route("/fake_clock/tick")
def tick():
    state["clock_seconds"] = max(0, state["clock_seconds"] - 10)
    return fake_clock()

@app.route("/fake_clock/score")
def score():
    state["home_score"] += 2
    return fake_clock()

@app.route("/fake_clock/period")
def period():
    # Uncapped - period 5+ is overtime. NBA regulation quarters are 12:00,
    # OT periods are 5:00. Used to reproduce/verify the period-transition
    # clock catch-up fix (game_state.cpp) and the OT indicator (draw_tools.cpp).
    state["period"] += 1
    state["clock_seconds"] = 12 * 60 if state["period"] <= 4 else 5 * 60
    return fake_clock()

@app.route("/fake_clock/reset")
def reset():
    state.update(home_score=0, away_score=0, period=1, clock_seconds=12 * 60,
                  status=2, fault="none")
    return fake_clock()

@app.route("/fake_clock/fault/<mode>")
def fault(mode):
    valid = {"none", "http500", "malformed_json", "missing_fields"}
    if mode not in valid:
        return Response(f"unknown fault mode '{mode}', valid: {sorted(valid)}", status=400)
    state["fault"] = mode
    return jsonify({"fault": state["fault"]})

@app.route("/fake_clock/autoplay/<mode>")
def autoplay(mode):
    if mode not in ("on", "off"):
        return Response(f"unknown autoplay mode '{mode}', valid: ['on', 'off']", status=400)
    with state_lock:
        state["autoplay"] = (mode == "on")
    return jsonify({"autoplay": state["autoplay"]})

if __name__ == "__main__":
    threading.Thread(target=_autoplay_loop, daemon=True).start()
    # host="0.0.0.0" so the ESP32 on the same network can actually reach it —
    # 127.0.0.1 would only be reachable from this machine itself.
    app.run(host="0.0.0.0", port=4999, debug=True)