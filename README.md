# Game Tracker 2.0

A 64×64 HUB75 LED matrix scoreboard, built on an [Adafruit MatrixPortal ESP32-S3](https://www.adafruit.com/product/5778), that runs in one of two modes:

- **NBA** — live score, game clock, and period for a chosen team, falling back to the next scheduled game when nothing's live.
- **NFL** — the current week's full schedule, cycling one matchup per page with each team's abbreviation in its real colors plus the favorite/spread and over/under below it.

Both modes are switchable live, from a phone or browser, without reflashing or rebooting the board. A minimal Kotlin/Compose Android app (`GameTrackerApp`, a sibling project) provides the phone-side control surface.

## Features

- **Live NBA tracking** — score, game clock, and period, pulled from `cdn.nba.com`'s public scoreboard endpoint; falls back to a next-scheduled-game lookup via [balldontlie](https://www.balldontlie.io/) when no game is live.
- **NFL weekly schedule + odds** — every game for the current week, fetched from ESPN's public scoreboard endpoint (odds arrive embedded in the same response — no second API call). Auto-advances through the week's games and re-fetches hourly.
- **Live sport switching** — a single `AppInput.sport` flag drives both the render loop and the state-update loop every iteration, so switching from NBA to NFL (or back) takes effect immediately, mid-runtime, from the control server or the app.
- **Local HTTP control server** with mDNS advertisement (`scoreboard.local`) — power on/off, NBA team selection, and sport switching, reachable from any browser on the LAN or from the companion app without typing an IP.
- **Companion Android app** — power toggle, NBA/NFL mode toggle, and a 30-team picker, built with plain `HttpURLConnection` (deliberately no Retrofit/OkHttp) and `NsdManager`-based mDNS auto-discovery.
- **Pixel-exact bitmap text rendering** — a custom ink-bounds font layout system (see [Text rendering](#text-rendering-pixel-exact-letter-spacing) below) guarantees exactly one physical pixel of gap between any two characters, at any supported size, for any pairing in the font — proven exhaustively by a native test rather than spot-checked.
- **Two local mock servers for offline development** — `test/TestServer.py` simulates both the NBA live-clock API (with autoplay, fault injection, and manual score/period control) and the NFL weekly-schedule API, so the board's full logic can be exercised without a live game or network access to the real APIs.
- **103 native unit tests** across 8 suites covering clock parsing, team lookup, game-state transitions, draw-layout math, font-glyph coverage, NFL schedule parsing, and NFL team colors — all run on your dev machine, no board required.

## Hardware

- Adafruit MatrixPortal ESP32-S3
- 64×64 HUB75 RGB LED matrix panel — 5 address lines (`common/display.cpp`); per Adafruit_Protomatter's `height = 2^addrCount × 2`, that's a 64-row/32-scan panel, not the more common 4-address-line 64×32 panel

| Signal | Pins |
|---|---|
| RGB (`[R1,G1,B1,R2,G2,B2]`) | 40, 41, 42, 37, 39, 38 |
| Address (`addrPins`) | 45, 36, 48, 35, 21 |
| Clock | 2 |
| Latch | 47 |
| OE | 14 |
| Bit depth | 4 |

The RGB pin order above already has R and B swapped relative to a naive reading of the silkscreen — an earlier version had them in physical order, which rendered every red as blue and vice versa on both NBA and NFL screens (confirmed by a near-pure-red team color rendering blue). If you're wiring a different panel/board combination, treat this order as the one to verify first.

## Architecture

Code is split three ways, by how sport-specific it is:

```
src/
  main.cpp        - boot, WiFi connect, FreeRTOS task creation, NBA/NFL branching
  secrets.h       - WiFi + API credentials (gitignored - see Setup)

  common/         - sport-agnostic, shared by both leagues
  nba/            - NBA- and balldontlie-specific
  nfl/            - NFL- and ESPN-specific
```

The intent: anything that isn't inherently tied to a specific league's data shape or team list lives in `common/` and is reused as-is. Adding NFL support meant adding `nfl/` alongside `nba/`, not touching NBA's code — `common/draw_tools.h`, `common/http_fetch.h`, `common/glyph_metrics.h`, etc. are used unmodified by both.

**Runtime model:** `main.cpp` pins two FreeRTOS tasks to the ESP32-S3's two cores — `renderTask` (core 1, ~60 Hz) draws whatever the current sport/state calls for, and `stateTask` (core 0, ~20 Hz) polls the control server, applies pending team/sport changes, and drives whichever sport's state machine is active. Both tasks branch on `appInput.sport` (`"NBA"` or `"NFL"`) every single iteration — not just at boot — which is what makes switching sports live rather than requiring a restart.

## NBA mode

- `nba/nba_api_client.cpp` fetches the live scoreboard; `nba/game_state.cpp`'s `GameStateMachine` decides whether the selected team is currently live (polls every 20s, with clock-catchup logic to reseed the displayed clock across period transitions) or not yet started (polls balldontlie every 5 min for the next scheduled game).
- `nba/nba_menu.cpp` draws the 30-team select grid shown at boot and whenever no team is chosen yet.
- Team sprites/palettes/colors live in `nba/nba_team_data.cpp` and `nba/nba_teams.cpp`.
- `nba_api_client.cpp` currently defaults to `TEST_SERVER 1` — pointed at the local mock server, not the live NBA API. Flip it to `0` to go live (see [Testing without live data](#testing-without-live-data)).

## NFL mode

- `nfl/nfl_api_client.cpp` fetches the current week's schedule from ESPN's undocumented public scoreboard endpoint (`site.api.espn.com/apis/site/v2/sports/football/nfl/scoreboard`) — a bare request with no query params resolves to whatever week is actually current, server-side, so the board never needs to compute that itself.
- Odds (favorite, spread, over/under) arrive embedded in the same response under `competitions[0].odds[]` — confirmed live, no separate odds call was ever needed. When a market hasn't posted odds yet, the `odds` key is simply absent, which `nfl_schedule_parser.cpp` treats as `hasOdds=false`, not an error.
- The full ESPN response is ~280 KB and nests 15 levels deep, past ArduinoJson's default nesting limit of 10 — parsing aborts partway through the first game unless the limit is raised (`JSON_NESTING_LIMIT` in `common/http_fetch.h`). The fetch also streams through an ArduinoJson filter (`buildNflScheduleFilter()`) that keeps only the fields the parser reads. A failed fetch yields an empty document, never a half-parsed one.
- `nfl/nfl_state.cpp`'s `NflScheduleState` re-fetches the week hourly (retrying every 60s while nothing's loaded, and keeping the last good week if a refresh fails) and auto-advances to the next game every 7 seconds.
- Games already played have no odds on ESPN, so they show `FINAL` instead of the favorite/spread/over-under.
- `nfl/nfl_menu.cpp`'s `drawScheduleList()` renders one matchup per page: away team, "VS.", home team (each abbreviation with its letters alternating between the team's two colors), then the favorite+spread (e.g. `BUF -7.0`) and the over/under labelled `+/-` (e.g. `+/-50.5`) below — all centered and vertically spaced with exactly 1px between blocks.
- Team colors (`nfl/nfl_team_colors.cpp`) are hand-curated per team rather than pulled from ESPN's `team.color` field — ESPN's own color data didn't always match how a fan would actually describe a team's colors (e.g. its Patriots "color" is navy, not red). Teams whose real secondary color is black have it swapped for a neutral stand-in, since letters are drawn directly in each color rather than filling a background.
- `nfl_api_client.cpp` currently defaults to `TEST_SERVER 0` — **live** against ESPN, as of the 2026 regular season. See `NFL_SUPPORT_ROADMAP.md` for the verification history and for what's still not done (kickoff time display, bye-week handling, full pixel-art logos, and the fact that this is an unofficial/undocumented endpoint with no change alerting).

## Text rendering: pixel-exact letter spacing

Both sports' screens draw text from a hand-authored bitmap font (`common/glyph_data.cpp`, 51 glyphs — digits, uppercase letters, and punctuation actually used across both displays). Early layout code positioned each glyph using its *declared* bounding-box width, which produced visually uneven gaps: some glyphs (like `S`) have a blank trailing column baked into their declared width, others (like `A`) have a blank leading column, so a flat `width + constant` advance looked inconsistent letter-to-letter.

`common/glyph_metrics.cpp` fixes this by computing each glyph's actual *ink* bounds (`glyphInkBounds`) and deriving a per-glyph draw offset and advance (`measureGlyph`) such that:

- every glyph's ink starts exactly at its nominal cursor position, regardless of its own leading padding, and
- the gap between any glyph's ink and the next glyph's ink is exactly 1 physical pixel, at any of the sizes actually used (1×–3×) — by construction, not by tuning.

`test/test_glyph_metrics/test_main.cpp` proves this exhaustively rather than by spot-check: it simulates the cursor math for every glyph paired with every other glyph (including itself) at every supported size — `51 × 51 × 3 ≈ 7800` cases — and asserts the ink-to-ink gap is exactly 1px in every one. Space is deliberately excluded from that rule and kept wider than a letter-gap, so a word-space still reads as a real separator rather than just another 1px gap.

## Project layout

```
src/
  main.cpp                     - boot, WiFi connect, FreeRTOS tasks, NBA/NFL branching
  secrets.h                    - WiFi + API credentials (gitignored, see Setup)
  secrets.h.example             - template for the above

  common/                      - sport-agnostic, reusable by any league
    display.cpp/.h               - Adafruit_Protomatter matrix init + pin mapping
    draw_tools.cpp/.h             - sprite/text/score/clock rendering (needs the live matrix)
    draw_formatting.cpp/.h        - pure layout/formatting logic behind draw_tools, unit tested
    glyph_data.cpp/.h             - bitmap font/character sprite table (51 glyphs), unit tested
    glyph_metrics.cpp/.h          - ink-bounds text layout math (pixel-exact spacing), unit tested
    team_sprite.h                 - generic TeamSprite struct (name/pattern/palette/size)
    http_fetch.cpp/.h             - generic HTTP(S)-GET-a-JSON-document helpers
    ntp_time.cpp/.h               - NTP time sync + date/EST formatting
    time_formatting.cpp/.h        - pure clock-string/date parsing logic, unit tested
    clock_sync.cpp/.h             - display-clock catch-up math, unit tested

  nba/                         - NBA/balldontlie-specific
    nba_api_client.cpp/.h         - NBA scoreboard + balldontlie HTTP fetch/parse
    nba_team_ids.cpp/.h            - balldontlie's numeric team IDs, unit tested
    nba_teams.cpp/.h, nba_team_data.cpp - team lookup + sprite/palette data, unit tested
    nba_menu.cpp/.h                - 30-team city-select menu UI
    game_state.cpp/.h             - state machine: live game vs. scheduled game, clock catch-up
    control_server.cpp/.h         - local HTTP control server + mDNS (serves both sports)

  nfl/                         - ESPN-specific weekly schedule + odds screen
    nfl_api_client.cpp/.h         - ESPN scoreboard HTTP fetch (odds embedded, no separate call)
    nfl_schedule_parser.cpp        - JSON -> NflWeekSchedule parsing, unit tested
    nfl_team_colors.cpp/.h        - hand-curated per-team RGB565 accent colors (32 teams), unit tested
    nfl_menu.cpp/.h                - drawScheduleList(): one matchup per page, team-colored text
    nfl_state.cpp/.h              - hourly refetch + auto-paging through the week's games
    nfl_teams.cpp/.h, nfl_team_data.cpp - full-logo lookup, stubbed (no pixel art authored yet)
    README.md                      - status notes for this folder

test/
  TestServer.py         - local Flask stand-in for both live APIs (NBA fake clock + NFL fake week)
  TEST_PLAN.md            - test rationale and coverage notes
  test_*/                 - native Unity test suites (PlatformIO `native` env)

NFL_SUPPORT_ROADMAP.md  - living status doc for NFL feature work
```

## Setup

1. Copy `src/secrets.h.example` to `src/secrets.h` and fill in your WiFi SSID/password and a [balldontlie API token](https://www.balldontlie.io/) (only needed for NBA's next-scheduled-game lookup; NFL mode needs no token). `secrets.h` is gitignored — never commit it.
2. Wire the matrix panel per the [pin table above](#hardware).
3. Set `monitor_port` in `platformio.ini` to whatever serial port your board enumerates as. The COM number can change between flashes, so re-check it if the serial monitor can't open the port.
4. Build and upload:
   ```
   pio run --target upload
   ```
   If the upload fails ("Could not open COMx" / "Failed to connect"), put the board in download mode by hand: hold **BOOT**, tap **RESET**, release **BOOT**, then upload again. To wipe the old flash first, run `pio run -t erase` in that same download mode. Tap **RESET** afterwards to start the new build.
5. On boot, the board connects to WiFi, starts the control server, and hibernates (panel stays black) until powered on. It then shows the NBA team-select menu by default — pick a team, or switch to NFL mode — either via the web page it serves at its own IP, or the companion Android app.

## Control server API

Served on port 80, no authentication (see [Known limitations](#known-limitations)):

| Endpoint | Effect |
|---|---|
| `GET /` | HTML control page (power buttons, sport dropdown, NBA team dropdown) |
| `GET /on` | Power on |
| `GET /off` | Power off (board restarts back into hibernation) |
| `GET /team?name=<ABBR>` | Select an NBA team by its 3-letter abbreviation; also switches to NBA mode |
| `GET /sport?value=NBA\|NFL` | Switch modes live |

The board also advertises itself over mDNS as `scoreboard.local` (deliberately generic — no user- or location-identifying info broadcast on the LAN).

## Testing without live data

Both `nba/nba_api_client.cpp` and `nfl/nfl_api_client.cpp` have a `TEST_SERVER` compile-time flag, checked independently:

```
python test/TestServer.py
```

**NBA fixture** (`/fake_clock`, `TEST_SERVER 1` by default): a simulated live game with:
- `autoplay` **on by default** — the game clock counts down once per real second, occasionally pausing (simulated dead ball), randomly awarding 1–3 point baskets, and advancing the quarter at 0:00
- `/fake_clock/tick`, `/score`, `/period`, `/reset` for manual control
- `/fake_clock/fault/<mode>` to inject HTTP errors, malformed JSON, or missing fields, exercising `fetchGame()`'s error-handling paths
- `/fake_clock/autoplay/on|off` to toggle autoplay

**NFL fixture** (`/fake_nfl_week`, `TEST_SERVER 0` by default — i.e. NFL is live unless you flip it back): mirrors the real ESPN response shape, including one game with no `odds` key, to exercise the `hasOdds=false` path.

Update the hardcoded test server IP in whichever `*_api_client.cpp` you're testing to match the machine running `TestServer.py` (must be on the same LAN as the board).

## Native unit tests

Pure logic — anything that doesn't touch the matrix, WiFi, or a live HTTP call — is split out so it can run on your dev machine, no board needed:

```
pio test -e native
```

103 tests across 8 suites:

| Suite | Covers |
|---|---|
| `test_clock_parsing` | NBA clock string parsing/formatting, UTC→EST conversion |
| `test_draw_formatting` | Score digit math, clock/date/time string parsing, quarter/OT labels |
| `test_game_state` | Clock catch-up math across period/OT transitions |
| `test_glyph_coverage` | Every character used across `drawChar` call sites resolves in the font table |
| `test_glyph_metrics` | Exhaustive pixel-exact letter-spacing proof (see [Text rendering](#text-rendering-pixel-exact-letter-spacing)) |
| `test_nfl_schedule` | ESPN JSON → `NflWeekSchedule` parsing (no-odds and finished games included), plus proof the fetch filter never changes what the parser produces and that the device's default JSON nesting limit can't parse the real response |
| `test_nfl_team_colors` | Per-team color lookup, including the unknown-abbreviation fallback |
| `test_team_lookup` | NBA team name/abbreviation resolution |

See `test/TEST_PLAN.md` for the full rationale, including bugs the test-writing process itself caught.

## Companion app

A minimal Android app lives in a sibling project, `GameTrackerApp` (Kotlin + Jetpack Compose), and talks to this board's control server over plain local HTTP — deliberately using only `HttpURLConnection`, no Retrofit/OkHttp:

- Power on/off, NBA/NFL mode toggle, and a 30-team picker (`ScoreboardApi.kt`)
- mDNS auto-discovery of the board's IP via `NsdManager` (`BoardDiscovery.kt`), holding a multicast lock for the duration of discovery since most Android devices drop multicast packets by default
- A private-LAN-only IP regex guard (`isValidBoardHost`), since the control server is plaintext HTTP with no auth

## Known limitations

- **No authentication** on the control server — anything on the same LAN can hit its endpoints directly. Acceptable for a home-LAN device with no port forwarding; would need a shared-token check before exposing it any more broadly.
- **mDNS discovery and control both require the phone/app and the board to be on the same local network.**
- **ESPN's NFL endpoint is unofficial and undocumented** — nothing alerts if it changes shape or goes away mid-season. If the board shows "NO NFL DATA" during the season, check the serial monitor for `fetchJson` errors first.
- **No full pixel-art NFL team logos** — `nfl_teams.h`/`nfl_team_data.cpp` are stubs; the current NFL screen uses team-colored text instead, which doesn't need them.
- **No kickoff time display** — `NflMatchup.kickoffIso` captures the raw ISO datetime, but nothing formats/renders it yet.
- **Bye weeks aren't specially called out** — a team on bye just won't appear in that week's games, which the parser already tolerates, but the screen doesn't say "BYE" anywhere.

See `NFL_SUPPORT_ROADMAP.md` for the fuller, living to-do list on the NFL side.
