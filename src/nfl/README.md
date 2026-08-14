# NFL support (schedule + odds screen built, not wired into main.cpp yet)

See `../../NFL_SUPPORT_ROADMAP.md` at the project root for full status.

## What's real

- `nfl_api_client.h/.cpp` - `fetchNflWeekScheduleJson()` is a real, working call against ESPN's scoreboard endpoint (`TEST_SERVER`-gated, same pattern as `nba/nba_api_client.cpp`).
- `nfl_schedule_parser.cpp` - `parseNflSchedule()` really parses schedule + embedded odds (confirmed: odds arrive in the same response, no second call needed). Unit tested in `test/test_nfl_schedule/`.
- `nfl_team_colors.h/.cpp` - real RGB565 accent colors for all 32 teams, sourced from ESPN's own `team.color`/`team.alternateColor`. Unit tested in `test/test_nfl_team_colors/`.
- `nfl_menu.h/.cpp` - `drawScheduleList()` renders one matchup per page on the confirmed 64x64 panel: team-colored abbreviation chips with a white border, "VS." separator, favorite/spread and over/under below. First-draft layout, not yet checked on real hardware.
- `TestServer.py`'s `/fake_nfl_week` endpoint, for offline testing against a realistic fixture.

## What's still stubbed / not started

- `nfl_teams.h/.cpp`, `nfl_team_data.cpp` - full pixel-art logo lookup, same shape as `nba/nba_teams.h`'s `TeamSprite`. Still returns `nullptr` / empty - this is a separate, much larger manual task than the color-chip approach `nfl_menu.cpp` actually uses today, and isn't needed for the schedule screen.
- Nothing in `main.cpp`/`nba/control_server.cpp` can switch into NFL mode yet - `drawScheduleList()` works and is tested standalone, but there's no runtime path that calls it. Needs a sport-selection UX decision that hasn't been made.
- No auto-paging/timer to cycle `drawScheduleList()`'s `pageIndex` through a full week's games - caller-driven, nothing drives it yet.
- Kickoff time isn't displayed (data's captured in `NflMatchup.kickoffIso`, just not rendered).

Drawing (`common/draw_tools.h`, including the newer `drawText`/`drawCharColored` colored-text primitives), glyph rendering (`common/glyph_data.h` - now includes F/J/V/period, added for NFL abbreviations and odds decimals), display init (`common/display.h`), generic HTTP fetch (`common/http_fetch.h`), and NTP time (`common/ntp_time.h`) are all sport-agnostic and used as-is.
