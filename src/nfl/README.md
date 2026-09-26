# NFL support (weekly schedule + odds screen, live)

See `../../NFL_SUPPORT_ROADMAP.md` at the project root for full status and history.

## What's real

- `nfl_api_client.h/.cpp` - `fetchNflWeekScheduleJson()` fetches the current week from ESPN's unofficial scoreboard endpoint (`TEST_SERVER`-gated, defaults to live). The response is ~280 KB and nests 15 levels deep, so the fetch uses an ArduinoJson filter (`buildNflScheduleFilter()`) and a raised nesting limit (`JSON_NESTING_LIMIT` in `common/http_fetch.h`); a failed fetch yields an empty document, never a partial one.
- `nfl_schedule_parser.cpp` - `parseNflSchedule()` parses schedule, embedded odds (no second call needed), and whether a game is final. Unit tested in `test/test_nfl_schedule/`, including proof that the fetch filter never changes what the parser produces.
- `nfl_team_colors.h/.cpp` - hand-curated RGB565 primary/secondary colors for all 32 teams. Unit tested in `test/test_nfl_team_colors/`.
- `nfl_menu.h/.cpp` - `drawScheduleList()` renders one matchup per page: away team, "VS.", home team (each abbreviation's letters alternate between the team's two colors), then the favorite+spread and the `+/-` over/under in white. Finished games show `FINAL` (ESPN drops their odds), games without odds yet show `ODDS TBD`, and `NO NFL DATA` appears if nothing has loaded.
- `nfl_state.h/.cpp` - `NflScheduleState` re-fetches hourly (retrying every 60s while nothing is loaded, and keeping the last good week if a refresh fails) and advances to the next game every 7 seconds.
- Wired into `main.cpp` and switchable live via the control server (`/sport?value=NFL`) and the Android app.
- `TestServer.py`'s `/fake_nfl_week` endpoint, for offline testing against a realistic fixture (including a finished game and one with no odds).

## What's still stubbed / not started

- `nfl_teams.h/.cpp`, `nfl_team_data.cpp` - full pixel-art logo lookup, same shape as `nba/nba_teams.h`'s `TeamSprite`. Still returns `nullptr` / empty - a separate, much larger manual task than the color-text approach `nfl_menu.cpp` uses today, and not needed for the schedule screen.
- Kickoff time isn't displayed (the data is captured in `NflMatchup.kickoffIso`, just not rendered).
- Bye weeks aren't called out - a team on bye simply doesn't appear that week.

Drawing (`common/draw_tools.h`, including the colored-text primitives `drawText`/`drawTextAlternating`), the pixel-exact bitmap font (`common/glyph_data.h`, `common/glyph_metrics.h` - includes F/J/V/period/`+`/`/`, added for NFL abbreviations and odds), display init (`common/display.h`), generic HTTP fetch (`common/http_fetch.h`), and NTP time (`common/ntp_time.h`) are all sport-agnostic and used as-is.
