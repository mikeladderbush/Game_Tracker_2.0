"""
Quick local stand-in for NBA's live scoreboard endpoint, for testing the
render pipeline while cdn.nba.com 403s during the off-season.

Run: python3 test_server.py
Then point api_client.cpp's NBA_SCOREBOARD_URL at this machine's IP, port 5000.

Endpoints:
  GET /fake_clock        -> current fake game state, same JSON shape fetchGame() expects
  GET /fake_clock/tick    -> advance the game clock by 10 seconds (call from browser/curl)
  GET /fake_clock/score   -> bump the home score by 2 (quick way to see drawScore update)
  GET /fake_clock/period  -> advance to the next period
  GET /fake_clock/reset   -> reset to a fresh Q1 tip-off state
"""
from flask import Flask, jsonify

app = Flask(__name__)

state = {
    "home_team": "Celtics",
    "away_team": "Knicks",
    "home_score": 0,
    "away_score": 0,
    "period": 1,
    "clock_seconds": 12 * 60,  # 12:00
    "status": 2,               # 2 = live, matches gameStatus used in game_state.cpp
}

def clock_to_pt_duration(total_seconds):
    minutes = total_seconds // 60
    seconds = total_seconds % 60
    return f"PT{minutes}M{seconds:02d}.00S"

@app.route("/fake_clock")
def fake_clock():
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
    state["period"] = min(4, state["period"] + 1)
    state["clock_seconds"] = 12 * 60
    return fake_clock()

@app.route("/fake_clock/reset")
def reset():
    state.update(home_score=0, away_score=0, period=1, clock_seconds=12 * 60, status=2)
    return fake_clock()

if __name__ == "__main__":
    # host="0.0.0.0" so the ESP32 on the same network can actually reach it —
    # 127.0.0.1 would only be reachable from this machine itself.
    app.run(host="0.0.0.0", port=4999, debug=True)