# NFL support (not yet implemented)

This folder is the landing zone for NFL-specific code, mirroring `src/nba/`'s
structure. See `../../NFL_SUPPORT_ROADMAP.md` at the project root for current
research/status - as of this pass, only the schedule/odds API research has
been done, no code exists here yet.

Expected mirror structure, once built:

- `nfl_teams.h/.cpp` - team lookup tables, using `common::TeamSprite` (same
  struct NBA uses - see `../common/team_sprite.h`)
- `nfl_team_data.cpp` - the 32 teams' sprite/palette data
- `nfl_api_client.h/.cpp` - ESPN API fetch/parse, built on
  `../common/http_fetch.h`'s generic `fetchJsonPlain`/`fetchJsonSecure`
  (same pattern `nba/nba_api_client.cpp` uses)
- `nfl_menu.h/.cpp` - team-selection menu UI (32 teams needs its own grid
  layout, not a reuse of `nba/nba_menu.cpp`'s 30-team one)
- A game-state/schedule module - NBA's `game_state.h/.cpp` is currently
  written directly against NBA's fetch functions rather than through an
  injected interface, since there was no second implementation yet to
  validate a shared interface against. Once this file exists, that's the
  right time to decide whether/how much of game_state.h's state-machine
  shape (poll live vs. scheduled, clock catch-up smoothing) can lift into
  `common/` behind a real interface, or whether NFL's shape (schedule/odds
  list, not a single live clock) is different enough to warrant its own
  independent module instead.

Drawing (`common/draw_tools.h`), glyph rendering (`common/glyph_data.h`),
display init (`common/display.h`), generic HTTP fetch (`common/http_fetch.h`),
NTP time (`common/ntp_time.h`), and generic formatting
(`common/time_formatting.h`, `common/draw_formatting.h`) are all already
sport-agnostic and ready to be used as-is - no changes needed there to add
NFL support.
