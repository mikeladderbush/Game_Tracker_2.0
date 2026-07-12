# Game Tracker 2.0 — Test Plan

Grounded in the actual code as of this pass. Organized by file. Flagged items marked
**[BUG]** are real defects found while writing this, not hypothetical edge cases —
worth fixing before or alongside writing the test that catches them.

**Status: 4 bugs fixed (clock catch-up on period transition, silent invalid-team
failure, missing OT indicator, hardcoded DST). Items below now read as regression
tests for those fixes rather than open bugs — marked [FIXED].**

Testing approach: split pure logic (string/clock parsing, state machine, team lookup)
into PlatformIO's native Unity test environment — none of it touches hardware and can
run on your dev machine in milliseconds. Drawing functions depend on the global
`matrix` object (Adafruit_Protomatter), so they need either a mock matrix or stay as
manual/visual tests via TestServer.py.

---

## 1. `clockStrToSecs` (api_client.cpp)

- `"PT12M00.00S"` → 720
- `"PT0M00.00S"` → 0
- `"PT5M9.30S"` (single-digit seconds, decimal) → 309
- `""` (empty) → -1
- `"PT"` alone, no M/S → 0 (falls through with minutes=0, seconds=0 — confirm this is intended, not -1)
- `"12:34"` (MM:SS, no decimal) → 754
- `"12:34.5"` (decimal seconds) → 754
- `"5:09 PM"` — space after seconds, confirm space-strip works → 309
- `"Final"` (no colon, no "PT") → -1
- Garbage string `"abc"` → -1
- Negative-looking input `"-5:00"` → confirm behavior is defined, not garbage math

## 2. `secsToMMSS` (api_client.cpp)

- 0 → `"00:00"`
- negative input (-5) → clamped to `"00:00"`
- 59 → `"00:59"`
- 60 → `"01:00"`
- 719 → `"11:59"`
- **[EDGE]** 6000+ seconds (100+ min) → `"100:00"` fits the 8-byte buffer exactly; test the boundary just above where it would truncate, since nothing currently prevents a bad targetSecs_ from ever reaching this

## 3. `convertUtcToEst` (api_client.cpp)

- `"00:30"` UTC → confirm modulo wraparound doesn't go negative (`((hours+offset)%24+24)%24`)
- `"23:59"` UTC → correct day-rollover hour
- Malformed input with no colon → returns input unchanged (confirm, don't crash)
- Noon boundary `"12:00"`/`"00:00"` → correct AM/PM label and 12-hour conversion (`hours12==0` → 12)
- **[FIXED]** `DAY_LIGHT_SAVINGS` constant removed. `syncTime()` now uses `configTzTime("EST5EDT,M3.2.0,M11.1.0", ...)` and `convertUtcToEst()` reads DST from `isDaylightSavingNow()` (checks `tm_isdst`). Regression test: mock/force `tm_isdst` on both sides of the March/November transition and confirm the offset flips without a firmware change.

## 4. `teamNameToId` (api_client.cpp)

- Known name `"Lakers"` → correct id
- Unknown name `"Fakers"` → -1
- Case mismatch `"lakers"` (lowercase) → -1, since it's a raw `strcmp` — confirm this is intentional given callers always pass canonical case

## 5. `fetchGame` (api_client.cpp) — needs TestServer.py or a mock HTTP layer

- HTTP non-200 response → `result.found == false`, no crash
- Malformed JSON → `result.found == false`, error logged not thrown
- Valid JSON, but requested team not in either game slot → not found
- Team is the home team vs. away team → score/opponent fields correctly swapped either way
- Missing individual fields (`score`, `gameClock`, `period` absent from JSON) → defaults (`""`/`0`) used, no crash on missing keys
- Multiple concurrent games in the array → correct game is selected, not just games[0]
- `"Sixers"` vs `"76ers"` — confirm this function is never called with `"Sixers"` directly (that's handled one layer up in `apiTeamName()`); test that assumption explicitly since a future caller could break it silently

## 6. `getNextGame` (api_client.cpp)

- Successful first attempt → returns immediately, no unneeded retries
- First 2 attempts fail (simulate via bad host/timeout), 3rd succeeds → retry loop recovers
- All 5 attempts fail → returns `result.found == false`, not stuck retrying forever
- `games` array empty → not found, returns early (confirm no crash on `games[0]` access)
- `teamId == -1` (unknown team passed in) → confirm request still fails gracefully rather than hitting the API with a nonsense id silently
- `datetimeStr` shorter than 16 characters (malformed/truncated) → `substring(11,16)` doesn't crash or produce garbage silently

## 7. GameStateMachine::begin() (game_state.cpp)

- Team currently live (status 1 or 2) → `inGame_ == true`, frame populated, clock seeded
- Team found but not yet started (status 0) → scheduled_ populated via the *inline* branch (uses `result.gameTimeText` and `result.opponent` directly) — separately from `refreshScheduledInfo()`'s path, which pulls from balldontlie instead. These two paths can disagree; test both independently.
- Team not found in today's games at all → falls through to `refreshScheduledInfo()` (balldontlie call)
- `clockStrToSecs` returns -1 on a garbled clock string → `displaySecs_` stays -1, doesn't crash render

## 8. GameStateMachine::pollLiveGame() (game_state.cpp) — the highest-value area to test

- Does **not** call the API before `LIVE_POLL_MS` (20s) has elapsed
- Game ends mid-poll (`result.found == false` or `gameStatus < 1`) → transitions to `inGame_ = false` and calls `refreshScheduledInfo()`
- **[FIXED]** Clock now reseeds (`displaySecs_ = apiSecs + DELAY_SECS`) whenever `apiSecs > displaySecs_`, not just on the first tick. Regression tests: (a) normal decrement — apiSecs at or below displaySecs_ across several polls never triggers a reseed; (b) period-transition jump — displaySecs_ near 0, next poll returns apiSecs ≈ 720 (new period), displaySecs_ snaps to ~750 instead of freezing; (c) OT transition — same jump behavior at period 4→5 boundary.
- Buffer/delay behavior: confirm `displaySecs_` is intentionally seeded at `targetSecs_ + DELAY_SECS` (30s behind live) on first tick, and that display time trails live time by roughly that amount under normal ticking
- `nextTickMs_` drift correction (`if (nextTickMs_ < now - 250) nextTickMs_ = now + 750`) — simulate a stalled/delayed task loop and confirm the clock resyncs instead of rapid-fire ticking to catch up

## 9. GameStateMachine::pollScheduled() (game_state.cpp)

- Does not call the API before `SCHED_POLL_MS` (5 min) elapsed
- Game transitions from scheduled → live (`gameStatus >= 1`) → `inGame_` flips true, `displaySecs_` seeded correctly whether or not `clockStrToSecs` succeeded
- Still no game found → `refreshScheduledInfo()` called again, no infinite tight loop
- `refreshScheduledInfo()`: `timeStr == "Final"` gets rewritten to `"12:00"` — test that substitution specifically, and ask whether that's the right placeholder for a completed game being shown as "next game"

## 10. `apiTeamName` (game_state.cpp)

- `"Sixers"` → `"76ers"`
- Every other team name → passthrough unchanged

## 11. `teamFromString` / `teamFromAbbr` (nba_teams.cpp)

- Full name lowercase match: `"celtics"` → Celtics
- Mixed case: `"Boston Celtics"` → matches via last-word fallback (`"celtics"`)
- `"76ers"` special-cased directly → sixers
- Multi-word franchise, last-word match: `"Trail Blazers"` → confirm `"blazers"` resolves to `trailBlazers`, not a false match
- No match at all (`"Globetrotters"`) → `nullptr`, not a crash
- Empty string → `nullptr`
- **[EDGE]** `teamFromAbbr` is case-sensitive (`strcmp`, not `strcasecmp`). `"bos"` (lowercase) → `nullptr` even though it's a real team. Confirm every caller uppercases first (control_server.cpp does) — but this is a landmine if a second entry point to team selection is ever added (e.g., the app you're planning) and forgets to uppercase.

## 12. Control server `/team`, `/on`, `/off` (control_server.cpp)

- **[FIXED — changed approach]** Free-text input replaced with a sport dropdown (NBA only, for now) and a team dropdown populated with all 30 valid abbreviations, so the UI can no longer submit garbage. Server-side `teamFromAbbr` guard kept as defense-in-depth since the endpoint has no auth. Regression tests: valid dropdown value → 200, `teamPending = true`; direct request with bogus value (`?name=ZZZ`, bypassing the dropdown via curl) → 400, "Invalid selection - please try again.", `appInput` untouched.
- Abbreviation longer than 3 characters via direct request (`?name=BOSTON`) → truncated to 3 chars by the `char buf[4]` buffer before validation; confirm truncation doesn't accidentally produce a *different valid* abbreviation
- Empty `?name=` via direct request → fails the `teamFromAbbr` guard cleanly, 400 response, no crash
- `/on` then `/off` then `/on` in rapid succession → `appInput.powerOn` reflects the last call, no dropped requests
- Concurrent requests (app + browser hitting `/team` and `/on` near-simultaneously) — `WebServer` is single-threaded via `handleClient()` in the state task loop, so verify there's no torn read of `appInput` from the render task reading `powerOn`/team state mid-write (currently no mutex around `AppInput` — flag if this becomes a real race once more fields are added)

## 13. `draw_tools.cpp` — testable once digit/format math is separated from `matrix.*` calls

- `drawScoreDigits` boundary: 0, 9, 10, 99, 100, 999 — confirm hundreds digit only draws when `> 0`, and column layout doesn't overlap between 2-digit and 3-digit scores
- **[FIXED, harness still needed]** `drawQuarter` now handles period 5 ("OT") and 6+ ("OT2", "OT3", ...); `main.cpp`'s gate loosened to `frame.period >= 1`. Regression tests: period 5 → "OT"; period 6 → "OT2"; period 11 → "OT7" (mod-10 wrap). Still can't exercise this end-to-end until TestServer.py's period cap is lifted — see section 16.
- `drawGameClock`: no colon → no-op; empty string → no-op; minutes ≥ 10 vs < 10 (leading blank digit)
- `drawScheduleTime`: `" ET"`, `" EST"`, `" EDT"` suffixes each stripped correctly; string with none of those suffixes unaffected; input longer than 24 chars truncated safely by `strncpy` without corrupting the ET-stripping logic; lowercase `"pm"` detected same as `"PM"`
- `drawDate`: well-formed `"YYYY-MM-DD"` → correct digits; malformed date (`sscanf` returns ≠ 3 fields) → no-op, doesn't draw garbage; month/day 1–9 draw without leading zero by design — confirm that's intentional and not a truncation bug
- `findGlyph` / `GLYPH_TABLE` coverage: write one test that walks every character used across `drawChar` call sites (digits, `:`, `-`, space, `p`, `a`, `H`, `T`, `s`, `t`, `n`, `d`, `r`, `h`, `A`–`Z` for the city menu) and asserts each resolves to a non-null glyph — silent no-ops on missing glyphs are otherwise invisible until you're staring at the actual board

## 14. `main.cpp` / boot & integration

- WiFi fails to connect within 15s → `ESP.restart()` fires (can't easily unit test the restart itself, but the timeout math is testable in isolation)
- `syncTime()` failure doesn't block boot — confirm time-dependent draw functions degrade gracefully (garbage/blank date) rather than crash when NTP never synced
- Device always boots into the `hibernating` loop (`AppInput.powerOn` defaults false) — confirm this is intentional (i.e., board never auto-powers-on after a reboot until the app says so) since that directly affects the "restart mid-game" bug below
- **[BUG carried over]** `FORCE_RESET_MS` (4 hours) restarts unconditionally, including mid-live-game, and `selectedTeam` is in-memory only — a restart during a game drops the board back to the team-selection menu. Root-causing what the 4-hour reset is actually working around should happen before writing more state-machine tests, since it may explain other "weird after a while" symptoms.
- Invalid team abbreviation submitted while in `menuActive` loop → loop never exits, never surfaces an error (same root cause as #12)

## 15. `display.cpp`

- `matrix.begin()` failure → currently `while(1)` with no watchdog feed and no serial diagnostic beyond the initial log line. Not unit-testable without hardware, but worth a manual test: pull a data pin and confirm the board's actual failure behavior (silent hang vs. watchdog reset vs. crash loop) matches what you want a shipped product to do.

## 16. Test harness gaps (TestServer.py)

**[DONE]** `/fake_clock/period` is now uncapped (period 5+ = OT, clock resets to 5:00
instead of 12:00) — reproduces the period-transition and OT scenarios directly.
`/fake_clock/fault/<mode>` (`http500`, `malformed_json`, `missing_fields`) makes the
next `/fake_clock` response fail in each of those ways, so `fetchGame`'s error paths
(section 5) can be exercised end-to-end instead of only against well-formed JSON.
`/fake_clock/reset` also resets the fault mode back to `none`.

---

## Build Plan — status: built and passing

Two more files needed splitting out (same reasoning as the `draw_tools.cpp` →
`draw_formatting.cpp` split): `api_client.cpp` pulls in `WiFiClientSecure.h` /
`HTTPClient.h` / `ArduinoJson.h` / ESP32's `configTzTime`/`getLocalTime` — none of
which exist on the native platform, so the file can't be compiled natively at all,
not even for its pure functions. Same story for `game_state.cpp`, which depends on
`api_client.h`.

- **`api_formatting.h/.cpp`** (new) — `clockStrToSecs`, `secsToMMSS`, `teamNameToId`,
  and `convertUtcToEstWithOffset` (the conversion math from `convertUtcToEst`, with
  the DST offset passed in explicitly instead of read from the live clock). No
  network/ESP32 dependency. `api_client.cpp` now calls into these instead of owning
  the logic, and `convertUtcToEst()` there is a two-line wrapper that reads the real
  DST offset and delegates.
- **`clock_sync.h/.cpp`** (new) — `nextDisplaySeconds`, the exact decision behind the
  bug 1 fix, pulled out of `game_state.cpp` the same way.

Native env (`[env:native]` in `platformio.ini`) excludes `main.cpp`, `display.cpp`,
`draw_tools.cpp`, `control_server.cpp`, `api_client.cpp`, and `game_state.cpp` —
everything else compiles and links natively via `ArduinoFake` (for `String`/`millis`
in `nba_teams.cpp`).

| File | Covers | Sections |
|---|---|---|
| `test/test_clock_parsing/test_main.cpp` | `clockStrToSecs`, `secsToMMSS`, `convertUtcToEstWithOffset`, `teamNameToId` | 1, 2, 3, 4 |
| `test/test_team_lookup/test_main.cpp` | `teamFromString`, `teamFromAbbr` | 11 |
| `test/test_game_state/test_main.cpp` | `nextDisplaySeconds` — the 3 clock catch-up regression cases (normal decrement, period jump, OT jump) | 8 |
| `test/test_draw_formatting/test_main.cpp` | Score digit math, clock/date/time string parsing, quarter/OT label logic | 13 |
| `test/test_glyph_coverage/test_main.cpp` | Every character used across `drawChar` call sites resolves in `GLYPH_TABLE` | 13 |

Still needs a mock/stub rather than pure native compilation (unchanged from the original plan):
- `fetchGame` / `getNextGame` (sections 5, 6) — exercise via TestServer.py, now including its fault-injection endpoints, rather than natively
- `control_server.cpp` (section 12) — needs `WebServer` running; a Python integration test hitting `/on`, `/off`, `/team` against a live board (or `test_control_server.py`, same pattern as TestServer.py) is still the plan
- `display.cpp` (section 15) — hardware-only, stays a manual test
- The seasonal DST *flip* itself (choosing -4 vs. -5 based on the real calendar date) — `isDaylightSavingNow()` needs a synced RTC and can't run natively; the conversion *math* is fully covered via `convertUtcToEstWithOffset`, but the flip is a manual/on-device check

**Verification:** all 5 native suites — 79 tests total — compile and pass. This
sandbox's network access blocks PlatformIO's package registry (`registry.platformio.org`),
so `pio test -e native` itself couldn't be run here; verification used a small
hand-rolled Unity + Arduino-`String` shim compiled directly with `g++` against your
actual source files instead. Run the real `pio test -e native` once on your machine
(needs network the first time, to fetch the `native` platform and `ArduinoFake` -
both cache locally after that) to confirm with the real toolchain.
