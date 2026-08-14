# NBA Scoreboard (Game Tracker 2.0)

A 64x32 HUB75 LED matrix scoreboard, driven by an [Adafruit MatrixPortal ESP32-S3](https://www.adafruit.com/product/5778), that tracks a chosen NBA team's live score and clock, falls back to the next scheduled game when nothing's live, and can be controlled from a phone over the local network.

## Features

- Live score, game clock, and period display for a selected team, pulled from `cdn.nba.com`'s public scoreboard endpoint
- Next-scheduled-game lookup (date/time, opponent) via the [balldontlie](https://www.balldontlie.io/) API when no game is live
- Team logos and colors rendered from hand-authored sprite/palette data (`nba/nba_team_data.cpp`, `nba/nba_teams.cpp`)
- Local HTTP control server for power on/off and team selection, reachable from a browser or the companion Android app
- mDNS advertisement (`scoreboard.local`) so the companion app can auto-discover the board's IP instead of requiring manual entry
- A local Flask test server (`test/TestServer.py`) that mimics the live scoreboard API, with fault-injection and an autoplay mode that runs a simulated game clock — for developing/testing without a live NBA game to point at
- Native (host-machine) Unity unit tests for all the pure logic — clock parsing/formatting, team lookup, game state transitions, glyph coverage — no board required

## Hardware

- Adafruit MatrixPortal ESP32-S3
- 64x32 HUB75 RGB LED matrix panel

## Project layout

Code is split into `common/` (sport-agnostic - display, drawing, HTTP fetch, time sync, generic formatting), `nba/` (everything NBA/balldontlie-specific), and `nfl/` (not implemented yet - see `NFL_SUPPORT_ROADMAP.md`). The goal is that adding a second sport means adding a new `nfl/` module that reuses `common/` as-is, not touching NBA code.

```
src/
  main.cpp                     - setup/loop, WiFi connect, FreeRTOS task creation
  secrets.h                    - WiFi + API credentials (gitignored, see Setup)

  common/                      - sport-agnostic, reusable by any league
    display.cpp/.h              - Adafruit_Protomatter matrix init
    draw_tools.cpp/.h            - sprite/score/clock rendering (needs the live matrix)
    draw_formatting.cpp/.h       - pure layout/formatting logic behind draw_tools, unit tested
    team_sprite.h                - generic TeamSprite struct (name/pattern/palette/size)
    http_fetch.cpp/.h            - generic HTTP(S)-GET-a-JSON-document helpers
    ntp_time.cpp/.h              - NTP time sync + date/EST formatting
    time_formatting.cpp/.h       - pure clock-string/date parsing logic, unit tested
    clock_sync.cpp/.h            - display-clock catch-up math, unit tested
    glyph_data.cpp/.h            - font/character sprite table, unit tested

  nba/                         - NBA/balldontlie-specific
    nba_api_client.cpp/.h        - NBA scoreboard + balldontlie HTTP fetch/parse
    nba_team_ids.cpp/.h          - balldontlie's numeric team IDs, unit tested
    nba_teams.cpp/.h, nba_team_data.cpp - team lookup + sprite/palette data, unit tested
    nba_menu.cpp/.h               - 30-team city-select menu UI
    game_state.cpp/.h            - state machine: live game vs. scheduled game, clock catch-up
    control_server.cpp/.h        - local HTTP control server + mDNS

  nfl/
    README.md                    - not implemented yet; intended mirror of nba/

test/
  TestServer.py        - local Flask stand-in for the live scoreboard API
  TEST_PLAN.md          - test rationale and coverage notes
  test_*/               - native Unity test suites (PlatformIO `native` env)
```

## Setup

1. Copy `src/secrets.h.example` to `src/secrets.h` and fill in your WiFi SSID/password and a [balldontlie API token](https://www.balldontlie.io/). `secrets.h` is gitignored — never commit it.
2. Wire the matrix panel per the pin mapping in `common/display.cpp`.
3. Set `monitor_port` in `platformio.ini` to whatever serial port your board enumerates as.
4. Build and upload:
   ```
   pio run --target upload
   ```
5. On boot, the board connects to WiFi, starts the control server, and waits (hibernating) until powered on and a team is selected — either via the web page it serves at its own IP, or the companion Android app.

## Testing without a live game

`nba/nba_api_client.cpp` has a `TEST_SERVER` compile-time flag. When set to `1`, it points at a local Flask server instead of the real NBA API:

```
python test/TestServer.py
```

This serves a fake live game at `/fake_clock`, with:
- `autoplay` **on by default** — the game clock counts down once per real second, occasionally pausing (simulated dead ball), randomly awarding 1-3 point baskets, and advancing the quarter at 0:00
- `/fake_clock/tick`, `/score`, `/period`, `/reset` for manual control
- `/fake_clock/fault/<mode>` to inject HTTP errors, malformed JSON, or missing fields, exercising `fetchGame()`'s error handling paths
- `/fake_clock/autoplay/on|off` to toggle autoplay

Update the hardcoded test server IP in `nba/nba_api_client.cpp` to match the machine running `TestServer.py` (must be on the same LAN as the board).

## Native unit tests

Pure logic (clock parsing, team lookup, game state transitions, glyph coverage) is split out from anything ESP32/Arduino/network-specific so it can run on your dev machine, no board needed:

```
pio test -e native
```

## Companion app

A minimal Android app lives in a sibling project, `GameTrackerApp`, and talks to this board's control server (`/on`, `/off`, `/team?name=<abbr>`) over plain local HTTP, with mDNS-based auto-discovery of the board's IP.

## Known limitations

- The control server has no authentication — anything on the same LAN can hit its endpoints directly. Acceptable for a home-LAN device with no port forwarding; would need a shared-token check before exposing it any more broadly.
- mDNS discovery and control both require the phone/app and the board to be on the same local network.
