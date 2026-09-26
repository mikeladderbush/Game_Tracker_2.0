# NFL Schedule + Odds To-Do

Scope: weekly scheduled matchups with betting odds (favorite, spread, over/under). Live game tracking is deprioritized for now.

1. **[confirmed]** Odds come embedded directly in the scoreboard response (`competitions[0].odds[]`) - checked against a real regular-season week-1 response (`NE @ SEA`, 2026-09-10). No separate per-game odds call needed.
2. **[resolved]** Given (1), the answer is: one fetch per week, always. `fetchNflOddsJson()`/`parseNflOdds()` (the separate-call path) were never built as real functions.
3. **[done, live]** `fetchNflWeekScheduleJson()` in `src/nfl/nfl_api_client.cpp` - `TEST_SERVER` flipped to `0` as of 2026-09-07 (regular season week 1) - re-confirmed live moments before the flip: zero query params returned `season.type=2` (regular season), `week.number=1`, 16 real games, first one Patriots @ Seahawks 2026-09-10, `hasOdds: true`. Also checked balldontlie.io (already used for NBA in this project) as an alternative - its NFL free tier is schedule-only, betting odds require the $39.99/mo GOAT tier - so ESPN remains the better fit (free, no key, odds included).

    **First run on real hardware (2026-09-26) failed, twice, and was fixed.** The board first showed one blank-away-team matchup vs `GB` with "ODDS TBD". Root cause: ArduinoJson 7's default nesting limit is 10, and the live ESPN response nests **15** deep (deepest path `events[].competitions[].competitors[].leaders[].leaders[].athlete.links[].rel`, measured live). The parse aborted with `TooDeep` inside the first game (home `GB` parsed, away `ATL` never did) and the half-filled document was displayed as if it were real. Filtered-out values still count toward the limit, so a filter alone doesn't help. Fix: `JSON_NESTING_LIMIT = 24` in `common/http_fetch.h`, passed on every deserialize; regression tests `test_the_devices_default_nesting_limit_cannot_parse_the_real_response` / `test_production_nesting_limit_covers_the_real_response`. (An earlier diagnosis blamed running out of memory on the ~279 KB response - that was a wrong guess, never confirmed. The filter, `buildNflScheduleFilter()`, is kept anyway as cheap insurance and keeps only a few KB, but the unfiltered tree's fit in memory was never actually tested.)

    Hardening added along the way: `fetchJson` clears the document on any failure instead of returning a partial one (which is why the second failure showed an honest "NO NFL DATA" instead of garbage); `NflScheduleState` keeps the last good week if a refresh fails and retries every 60s while nothing is loaded; a finished game (ESPN drops its odds) shows "FINAL" instead of "ODDS TBD" (`NflMatchup.isFinal`, from `events[].status.type.state == "post"`); the state task and Arduino loop task stacks are 16 KB (deep JSON parse with a TLS read at the bottom of the call stack), with the remaining stack logged over serial after each fetch (`NFL: fetched N games ... stack headroom N bytes`). **Confirmed on the physical board 2026-09-26:** serial showed `NFL: fetched 16 games (week 3), showing 16; stack headroom 10960 bytes` (about 5.4 KB of the 16 KB stack used) and the panel was confirmed to look right.
4. **[done]** `parseNflSchedule()` in `src/nfl/nfl_schedule_parser.cpp` - real parsing, unit tested (`test/test_nfl_schedule/`).
5. ~~Separate odds call~~ - not needed, see (1)/(2).
6. ~~`parseNflOdds()`~~ - folded into `parseNflSchedule()`.
7. **[done]** `NflMatchup`/`NflWeekSchedule` in `src/nfl/nfl_api_client.h`.
8. **[done, verified on hardware]** `drawScheduleList()` in `src/nfl/nfl_menu.cpp` - one matchup per page: away abbreviation, "VS.", home abbreviation (each abbreviation's letters alternate between the team's two colors, no borders), then the favorite+spread (`BUF -7.0`) and the over/under labelled `+/-` (`+/-50.5`) below in white. Finished games show `FINAL`, games without odds yet show `ODDS TBD`. Laid out for the confirmed 64x64 panel with exactly 1px between text blocks and between letters (`common/glyph_metrics.*`, proven exhaustively in `test/test_glyph_metrics/`).
9. **[done]** `NflScheduleState` (`src/nfl/nfl_state.*`) fetches the current week hourly (60s retry while empty, keeps the last good week on failure) and auto-advances `pageIndex` every 7s.
10. **[still open, real work]** Author NFL team sprite/palette data for full logos (`nfl_team_data.cpp`) - unrelated to the color-only chips above, which use `nfl_team_colors.cpp`'s flat per-team RGB565 pairs instead of pixel art. Still undone if ever wanted.
11. **[done]** `TestServer.py`'s `/fake_nfl_week` mirrors the confirmed real shape (including one finished game and one game with no `odds` key, to exercise `isFinal` and `hasOdds=false`).
12. **[done]** Native unit tests: `test/test_nfl_schedule/`, `test/test_nfl_team_colors/`.
13. **[done]** Runtime NBA/NFL switching: `AppInput.sport` + `/sport?value=NBA|NFL` control server endpoint, live-switchable (not just at boot) via `main.cpp`'s `stateTask`/`renderTask` branching on `appInput.sport` every iteration. Android app has NBA/NFL toggle buttons that call it.

## Not done / explicitly out of scope so far

- **Live season monitoring.** ESPN's endpoint is unofficial/undocumented - nothing alerts if it changes shape or goes away mid-season. If the board ever shows "NO NFL DATA" during the season, check the serial monitor for `fetchJson` errors first.

- **Kickoff time display.** `NflMatchup.kickoffIso` captures the raw ISO datetime but nothing formats/displays it - the requested screen only asked for matchup + odds.
- **Bye weeks.** Not specially handled - a team simply won't appear in `events[]` that week, which `parseNflSchedule()` already tolerates fine (doesn't assume a fixed team list), but nothing calls it out.
- **Remaining on-hardware checks.** The live NFL fetch and screen are confirmed (see item 3), and switching into NFL mode over HTTP worked. Not yet exercised on the board: switching back from NFL to NBA mid-run, the hourly refresh, and the Tuesday week rollover.
- **Full pixel-art team logos** (see item 10).

## Confirmed real JSON fields (live-checked, not secondhand)

- Team abbreviations are 3-letter (mostly - `GB`, `LV`, `NE`, `NO`, `SF`, `TB` are 2), no collision with NBA.
- `competitors[].team.color` / `.alternateColor` are real hex (no `#`), confirmed for all 32 teams via `site.api.espn.com/apis/site/v2/sports/football/nfl/teams` - baked into `nfl_team_colors.cpp` as precomputed RGB565.
- `competitions[0].odds[0]`: `spread` (signed float, negative = home favored), `overUnder` (float), `homeTeamOdds.favorite`/`awayTeamOdds.favorite` (bool), `details` (human string, e.g. `"SEA -3.5"`, unused - computed the same thing from the structured fields instead).
- `competitions[0].odds` can be **entirely absent** (not just empty) when no market's posted yet - `parseNflSchedule()` treats this as `hasOdds=false`, not an error.
- A bare scoreboard request (no `week`/`seasontype`/`dates`) returns the actual current week - confirmed twice live, once at Hall-of-Fame-weekend time and again at Preseason-Week-2 time, both correct for when they were fetched.
- `status.displayClock` is `"MM:SS"` — not relevant to schedule-only scope, kept for reference if live tracking comes back later.
