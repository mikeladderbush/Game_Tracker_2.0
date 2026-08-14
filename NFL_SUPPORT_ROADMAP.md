# NFL Schedule + Odds To-Do

Scope: weekly scheduled matchups with betting odds (favorite, spread, over/under). Live game tracking is deprioritized for now.

1. **[confirmed]** Odds come embedded directly in the scoreboard response (`competitions[0].odds[]`) - checked against a real regular-season week-1 response (`NE @ SEA`, 2026-09-10). No separate per-game odds call needed.
2. **[resolved]** Given (1), the answer is: one fetch per week, always. `fetchNflOddsJson()`/`parseNflOdds()` (the separate-call path) were never built as real functions - once (1) confirmed the embedded case, building them would've been dead code.
3. **[done]** `fetchNflWeekScheduleJson()` in `src/nfl/nfl_api_client.cpp` - real, working call, `TEST_SERVER`-gated same pattern as `nba/nba_api_client.cpp`.
4. **[done]** `parseNflSchedule()` in `src/nfl/nfl_schedule_parser.cpp` - real parsing, unit tested (`test/test_nfl_schedule/`) against fixtures shaped like the confirmed live response.
5. ~~If odds aren't embedded, add `fetchNflOddsJson(eventId)`~~ - not needed, see (1)/(2).
6. ~~Add `parseNflOdds()`~~ - folded into `parseNflSchedule()` directly, since odds arrive in the same response.
7. **[done]** `NflMatchup`/`NflWeekSchedule` in `src/nfl/nfl_api_client.h`.
8. **[done, first draft]** `drawScheduleList()` in `src/nfl/nfl_menu.cpp` - one matchup per page: team abbreviations in real team colors with a white border, "VS." separator, favorite+spread and over/under below in white. Laid out for the confirmed 64x64 panel. Not yet checked on real hardware - pixel positions are a reasonable first guess, not verified.
9. **[partially done]** `drawScheduleList(schedule, pageIndex)` takes a page index so something can cycle through `[0, schedule.count)` - the actual cycling (timer? button?) isn't wired into `main.cpp` yet, and neither is a way to choose NFL vs. NBA mode at runtime. See "Not done" below.
10. **[still open, real work]** Author NFL team sprite/palette data for full logos (`nfl_team_data.cpp`) - unrelated to the color-only chips above, which use `nfl_team_colors.cpp`'s flat per-team RGB565 pairs (sourced directly from ESPN's `team.color`/`team.alternateColor`) instead of pixel art. Full logos are still a real, undone, manual task if ever wanted.
11. **[done]** `TestServer.py`'s `/fake_nfl_week` mirrors the confirmed real shape (including one game with no `odds` key, to exercise `hasOdds=false`).
12. **[done]** Native unit tests: `test/test_nfl_schedule/` (parsing) and `test/test_nfl_team_colors/` (color lookup).

## Not done / explicitly out of scope for this pass

- **Runtime integration.** Nothing in `main.cpp` or `control_server.cpp` can actually switch into NFL mode yet - `drawScheduleList()` exists and is tested in isolation, but nothing calls it from the live render loop. This needs a real sport-selection UX decision (menu? toggle? both at once?) that hasn't been discussed, so it wasn't built speculatively.
- **Kickoff time display.** `NflMatchup.kickoffIso` captures the raw ISO datetime but nothing formats/displays it yet - the requested screen only asked for matchup + odds.
- **Bye weeks.** Not handled - a team simply won't appear in `events[]` that week, which `parseNflSchedule()` already handles fine (it doesn't assume a fixed team list), but nothing calls that out specially.
- **Paging trigger.** `pageIndex` is a plain parameter - no auto-advance timer or button-driven paging exists yet.

## Confirmed real JSON fields (live-checked, not secondhand)

- Team abbreviations are 3-letter (mostly - `GB`, `LV`, `NE`, `NO`, `SF`, `TB` are 2), no collision with NBA.
- `competitors[].team.color` / `.alternateColor` are real hex (no `#`), confirmed for all 32 teams via `site.api.espn.com/apis/site/v2/sports/football/nfl/teams` - baked into `nfl_team_colors.cpp` as precomputed RGB565.
- `competitions[0].odds[0]`: `spread` (signed float, negative = home favored), `overUnder` (float), `homeTeamOdds.favorite`/`awayTeamOdds.favorite` (bool), `details` (human string, e.g. `"SEA -3.5"`, unused - computed the same thing from the structured fields instead).
- `competitions[0].odds` can be **entirely absent** (not just empty) when no market's posted yet - `parseNflSchedule()` treats this as `hasOdds=false`, not an error.
- `status.displayClock` is `"MM:SS"` — not relevant to schedule-only scope, kept for reference if live tracking comes back later.
