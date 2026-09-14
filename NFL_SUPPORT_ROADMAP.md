# NFL Schedule + Odds To-Do

Scope: weekly scheduled matchups with betting odds (favorite, spread, over/under). Live game tracking is deprioritized for now.

1. **[confirmed]** Odds come embedded directly in the scoreboard response (`competitions[0].odds[]`) - checked against a real regular-season week-1 response (`NE @ SEA`, 2026-09-10). No separate per-game odds call needed.
2. **[resolved]** Given (1), the answer is: one fetch per week, always. `fetchNflOddsJson()`/`parseNflOdds()` (the separate-call path) were never built as real functions.
3. **[done, live]** `fetchNflWeekScheduleJson()` in `src/nfl/nfl_api_client.cpp` - `TEST_SERVER` flipped to `0` as of 2026-09-07 (regular season week 1) - re-confirmed live moments before the flip: zero query params returned `season.type=2` (regular season), `week.number=1`, 16 real games, first one Patriots @ Seahawks 2026-09-10, `hasOdds: true`. Also checked balldontlie.io (already used for NBA in this project) as an alternative - its NFL free tier is schedule-only, betting odds require the $39.99/mo GOAT tier - so ESPN remains the better fit (free, no key, odds included).
4. **[done]** `parseNflSchedule()` in `src/nfl/nfl_schedule_parser.cpp` - real parsing, unit tested (`test/test_nfl_schedule/`).
5. ~~Separate odds call~~ - not needed, see (1)/(2).
6. ~~`parseNflOdds()`~~ - folded into `parseNflSchedule()`.
7. **[done]** `NflMatchup`/`NflWeekSchedule` in `src/nfl/nfl_api_client.h`.
8. **[done, first draft]** `drawScheduleList()` in `src/nfl/nfl_menu.cpp` - one matchup per page: team abbreviations in real team colors with a white border, "VS." separator, favorite+spread and over/under below in white. Laid out for the confirmed 64x64 panel. Not yet checked on real hardware - pixel positions are a reasonable first guess, not verified.
9. **[done]** `NflScheduleState` (`src/nfl/nfl_state.*`) fetches the current week hourly and auto-advances `pageIndex` every 5s.
10. **[still open, real work]** Author NFL team sprite/palette data for full logos (`nfl_team_data.cpp`) - unrelated to the color-only chips above, which use `nfl_team_colors.cpp`'s flat per-team RGB565 pairs instead of pixel art. Still undone if ever wanted.
11. **[done]** `TestServer.py`'s `/fake_nfl_week` mirrors the confirmed real shape (including one game with no `odds` key, to exercise `hasOdds=false`).
12. **[done]** Native unit tests: `test/test_nfl_schedule/`, `test/test_nfl_team_colors/`.
13. **[done]** Runtime NBA/NFL switching: `AppInput.sport` + `/sport?value=NBA|NFL` control server endpoint, live-switchable (not just at boot) via `main.cpp`'s `stateTask`/`renderTask` branching on `appInput.sport` every iteration. Android app has NBA/NFL toggle buttons that call it.

## Not done / explicitly out of scope so far

- **Live season monitoring.** ESPN's endpoint is unofficial/undocumented - nothing alerts if it changes shape or goes away mid-season. If the board ever shows "NO NFL DATA" during the season, check the serial monitor for `fetchJson` errors first.

- **Kickoff time display.** `NflMatchup.kickoffIso` captures the raw ISO datetime but nothing formats/displays it - the requested screen only asked for matchup + odds.
- **Bye weeks.** Not specially handled - a team simply won't appear in `events[]` that week, which `parseNflSchedule()` already tolerates fine (doesn't assume a fixed team list), but nothing calls it out.
- **Real on-hardware verification.** The `drawScheduleList()` layout and the sport-switch flow have only been build/test-verified, not seen on the actual panel yet.
- **Full pixel-art team logos** (see item 10).

## Confirmed real JSON fields (live-checked, not secondhand)

- Team abbreviations are 3-letter (mostly - `GB`, `LV`, `NE`, `NO`, `SF`, `TB` are 2), no collision with NBA.
- `competitors[].team.color` / `.alternateColor` are real hex (no `#`), confirmed for all 32 teams via `site.api.espn.com/apis/site/v2/sports/football/nfl/teams` - baked into `nfl_team_colors.cpp` as precomputed RGB565.
- `competitions[0].odds[0]`: `spread` (signed float, negative = home favored), `overUnder` (float), `homeTeamOdds.favorite`/`awayTeamOdds.favorite` (bool), `details` (human string, e.g. `"SEA -3.5"`, unused - computed the same thing from the structured fields instead).
- `competitions[0].odds` can be **entirely absent** (not just empty) when no market's posted yet - `parseNflSchedule()` treats this as `hasOdds=false`, not an error.
- A bare scoreboard request (no `week`/`seasontype`/`dates`) returns the actual current week - confirmed twice live, once at Hall-of-Fame-weekend time and again at Preseason-Week-2 time, both correct for when they were fetched.
- `status.displayClock` is `"MM:SS"` — not relevant to schedule-only scope, kept for reference if live tracking comes back later.
