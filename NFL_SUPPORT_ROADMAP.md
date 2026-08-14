# NFL Schedule + Odds To-Do

Scope: weekly scheduled matchups with betting odds (favorite, spread, over/under). Live game tracking is deprioritized for now.

1. Confirm from your own machine whether the weekly scoreboard response embeds odds inline, or if odds always require a separate per-game call.
2. Based on that, decide polling scope — odds for every game in the week (many HTTPS round trips) vs. just the currently-displayed matchup.
3. Add `fetchNflWeekScheduleJson()` against `site.api.espn.com/.../scoreboard?week=&seasontype=&dates=`, using the existing shared HTTP helpers.
4. Add `parseNflSchedule()` to turn that response into a list of matchups (teams, date/time).
5. If odds aren't embedded, add `fetchNflOddsJson(eventId)` against `sports.core.api.espn.com/.../events/{id}/competitions/{id}/odds`.
6. Add `parseNflOdds()` to pull favorite, spread, and over/under out of that response.
7. Design a data structure to hold a full week's matchups + odds (up to ~16 games).
8. Design a new display screen for a scheduled-games list (this is a new view, not an extension of the existing single-game clock display).
9. Add a way to page/scroll through the week's matchups since they won't all fit on a 64x32 display at once.
10. Author NFL team sprite/palette data (still needed, but lower urgency without live tracking).
11. Add a `TestServer.py`-style mock for the schedule + odds endpoints so this can be developed without hammering ESPN's API.
12. Add native unit tests for the new schedule/odds parsing functions.

## Confirmed real JSON fields

- Team abbreviations are 3-letter (`ARI`, `CAR`), no collision with NBA.
- `competitors[].score` is a string, needs parsing.
- `status.displayClock` is `"MM:SS"` — not relevant to schedule-only scope, kept for reference if live tracking comes back later.
- Odds endpoint (`sports.core.api.espn.com/.../odds`) confirmed working, returns: `awayTeamOdds.favorite`/`homeTeamOdds.underdog` (bool), `spread` (numeric) plus `awayTeamOdds.current.pointSpread.american`/`homeTeamOdds.current.pointSpread.american` (e.g. `"-1.5"`/`"+1.5"`), `overUnder` (numeric, e.g. `34.5`), `awayTeamOdds.moneyLine`/`homeTeamOdds.moneyLine`. Provider was DraftKings (id 100).
- Open question: does the scoreboard response itself ever include an `odds` array per-event (saving a second call), or is this event's Hall-of-Fame-game response simply missing it because it's preseason/no market posted? Check a regular-season week's response for this before committing to an architecture in step 1/2.
