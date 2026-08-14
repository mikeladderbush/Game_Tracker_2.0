# NFL support (scaffolded, not implemented)

Mirrors `src/nba/`'s structure. See `../../NFL_SUPPORT_ROADMAP.md` at the
project root for status/research - as of this pass, function signatures and
data structures exist as stubs, but no real fetch/parse/render logic is
implemented yet.

- `nfl_teams.h/.cpp` - team lookup, using `common::TeamSprite` (same struct
  NBA uses). Stubbed: always returns `nullptr`, no team data exists yet.
- `nfl_team_data.cpp` - landing file for the 32 teams' sprite/palette data
  (roadmap step 10). Empty - this is real pixel-art/palette authoring work,
  not fabricated here.
- `nfl_api_client.h/.cpp` - ESPN API fetch/parse, built on
  `../common/http_fetch.h` (same pattern `nba/nba_api_client.cpp` uses).
  `fetchNflWeekScheduleJson()`/`fetchNflOddsJson()` are real, working calls
  against confirmed-working URLs. `parseNflSchedule()`/`parseNflOdds()` are
  stubs (roadmap steps 4 and 6) - deliberately not implemented since the
  exact regular-season response shape hasn't been confirmed yet (only a
  preseason/Hall-of-Fame-game response has been checked so far).
- `nfl_menu.h/.cpp` - `drawScheduleList()` stub for the not-yet-designed
  schedule-list display (roadmap steps 8-9). Signature is a placeholder
  guess, not a committed design - this is a new kind of view (multiple
  games at once), not a reuse of `nba/nba_menu.cpp`'s single-team grid.
- No game-state/schedule module yet - deferred until the above is fleshed
  out enough to know whether NBA's `game_state.h/.cpp` state-machine shape
  (poll live vs. scheduled, clock catch-up) is worth lifting into `common/`,
  or whether NFL's shape (a weekly list, not a single live clock) warrants
  its own independent module instead.

Drawing (`common/draw_tools.h`), glyph rendering (`common/glyph_data.h`),
display init (`common/display.h`), generic HTTP fetch (`common/http_fetch.h`),
NTP time (`common/ntp_time.h`), and generic formatting
(`common/time_formatting.h`, `common/draw_formatting.h`) are all already
sport-agnostic and used as-is by the stubs above - no changes needed there.
