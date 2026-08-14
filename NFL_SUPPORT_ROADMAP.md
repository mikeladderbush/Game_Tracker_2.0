# NFL Schedule + Odds To-Do

Scope: weekly scheduled matchups with betting odds (favorite, spread, over/under). Live game tracking is deprioritized for now.

1. Confirm from your own machine whether the weekly scoreboard response embeds odds inline, or if odds always require a separate per-game call.
2. Based on that, decide polling scope — odds for every game in the week (many HTTPS round trips) vs. just the currently-displayed matchup.
3. **[stub in place]** `fetchNflWeekScheduleJson()` in `src/nfl/nfl_api_client.cpp` — real, working call against the confirmed URL. Uses the existing shared HTTP helpers.
4. **[stub in place, unimplemented]** `parseNflSchedule()` in `src/nfl/nfl_api_client.cpp` — empty body, returns a zeroed `NflWeekSchedule`. Blocked on confirming the regular-season response shape (see step 1).
5. **[stub in place]** `fetchNflOddsJson(eventId)` in `src/nfl/nfl_api_client.cpp` — real, working call against the confirmed URL. Only needed at all if step 1/2 concludes odds require a separate call.
6. **[stub in place, unimplemented]** `parseNflOdds()` in `src/nfl/nfl_api_client.cpp` — empty body, leaves `NflMatchup.hasOdds` false. Field names are confirmed (see below) but not yet wired up.
7. **[draft in place]** `NflMatchup`/`NflWeekSchedule` structs in `src/nfl/nfl_api_client.h` — combines schedule + odds into one struct per game; revisit once steps 4/6 are real.
8. **[placeholder signature only]** `drawScheduleList()` in `src/nfl/nfl_menu.h/.cpp` — no layout designed yet, this is a new kind of view (multiple games at once), not an extension of the existing single-game display.
9. Add a way to page/scroll through the week's matchups since they won't all fit on a 64x32 display at once. (`drawScheduleList()`'s `scrollOffset` param is a placeholder guess for this, not a committed design.)
10. **[landing file created]** `src/nfl/nfl_team_data.cpp` exists but is empty — authoring the 32 teams' sprite/palette data is still real, undone work.
11. Add a `TestServer.py`-style mock for the schedule + odds endpoints so this can be developed without hammering ESPN's API.
12. Add native unit tests for the new schedule/odds parsing functions once steps 4/6 have real logic to test.

## Confirmed real JSON fields

- Team abbreviations are 3-letter (`ARI`, `CAR`), no collision with NBA.
- `competitors[].score` is a string, needs parsing.
- `status.displayClock` is `"MM:SS"` — not relevant to schedule-only scope, kept for reference if live tracking comes back later.
- Odds endpoint (`sports.core.api.espn.com/.../odds`) confirmed working, returns: `awayTeamOdds.favorite`/`homeTeamOdds.underdog` (bool), `spread` (numeric) plus `awayTeamOdds.current.pointSpread.american`/`homeTeamOdds.current.pointSpread.american` (e.g. `"-1.5"`/`"+1.5"`), `overUnder` (numeric, e.g. `34.5`), `awayTeamOdds.moneyLine`/`homeTeamOdds.moneyLine`. Provider was DraftKings (id 100).
- Open question: does the scoreboard response itself ever include an `odds` array per-event (saving a second call), or is this event's Hall-of-Fame-game response simply missing it because it's preseason/no market posted? Check a regular-season week's response for this before committing to an architecture in step 1/2.
