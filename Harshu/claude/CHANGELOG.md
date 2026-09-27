# Abyss changelog

## v2 (in progress) — 2026-09-27

Addresses all five weaknesses from the 0-20 online debut.

### Selective head trades
- Old: any head trade when cornered (`all.empty()`)
- New: requires ally survivors (`count > 1`) or draw opportunity; when not cornered, needs `count >= 3` and `length <= 4`
- Expected impact: dramatically fewer self-inflicted deaths in swarm games

### Swarm expansion
- Config already had split_length=4, max_team=48, split_cooldown=1 (from planning)
- Extended `split_before` to 420 (was 350) for late-game replacement splits
- Emergency splits now work with incomplete body and need less room (2 vs 4 tiles)

### Portal exploration
- Stagnation triggers loosened (food gap 8→5, portal gap 12→8, removed length restriction)
- New `low_prospects` trigger: take portals when local options score poorly (< 3.0) even without strict stagnation
- Result: 4/4 wins on Portals map (was 0/4 online)

### Long-body escape
- Proactive split threshold for incomplete bodies: length 8→5
- Emergency splits now allowed with partial body knowledge
- Result: early kelp deaths should drop on autarky/slithery_fight starts

### Reduced far-threat panic
- `danger_two`: 48→36, `danger_three`: 12→6
- Against 30-64 enemy swarms, distant threats are everywhere; heavy penalties made all moves equally bad

### Local results summary
| Opponent | Seed | W | D | L | Rate |
|---|---|---|---|---|---|
| hunter + greedy | 101 | 27 | 0 | 5 | 84.4% |
| hunter + greedy | 42 | 28 | 0 | 4 | 87.5% |
| v1 (sheltered) | 101+42 | 21 | 0 | 11 | 65.6% |

### Known remaining weaknesses
- **Autarky**: food-sparse map; v2 swarm splits dilute growth, loses on score at round 500
- **Stronghold**: consistent losses vs hunter; narrow corridors may trap more dragons
- **Slithery_fight vs v1**: v1's single large dragon can outscore v2's population in scoring games

## v1 — 2026-09-27 (submitted)
- 28/38 local sandbox wins vs greedy/hunter baselines
- 0/20 online vs blauerdrache/bongcloud
- See `evaluations/REPORT.md` and `evaluations/online/REPORT.md`
