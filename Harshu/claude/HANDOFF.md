# Claude Session Handoff — v2 Implementation

## Picked up from

- **v1 submitted** as abyss-v1 (submission 9003, 2026-09-27T17:07Z)
- v1 went **0-20 online** against blauerdrache (1522) and bongcloud (1586)
- Root causes identified in `evaluations/online/REPORT.md`:
  1. 99/114 own deaths were self-initiated head trades — reckless 1-for-1 exchanges
  2. Opponents build 21-64 dragons; v1 capped at 4 normal
  3. Portal avoidance starved the bot on portal-heavy maps
  4. Long starting bodies caused early kelp deaths (splits/sprints disabled)
  5. Baselines too weak to reveal these problems
- Config was already partially updated for v2 planning: `split_length=4, max_team=48, split_cooldown=1`
- Strategy code was still v1 logic

## Changes made (this session)

### config.hpp
| Constant | v1 (submitted) | v2 | Why |
|---|---|---|---|
| `split_before` | 260 (submitted) / 350 (working) | 420 | Allow splitting deeper into late game against swarms |
| `danger_two` | 48.0 | 36.0 | Reduce penalty for 2-step threats; ubiquitous against swarms |
| `danger_three` | 12.0 | 6.0 | Same — 3-step enemies are everywhere in swarm games |

### strategy.hpp — safe_split()
- Removed hard `!complete` block: emergency splits now allowed with incomplete body
- Added `int(s.body.size()) < 2` guard instead (need at least 2 known segments)
- Emergency splits need only `room >= 2` instead of `room >= 4`

### strategy.hpp — choose() — incomplete-body split
- Proactive split threshold lowered: `length >= 8` → `length >= 5`
- Dragons with long unseen tails now shed them earlier instead of wandering blind

### strategy.hpp — choose() — portal exploration
- Stagnation thresholds loosened: food gap 8→5 rounds, portal gap 12→8 rounds
- Removed `length <= 6` restriction
- Added `low_prospects` trigger: takes portal when best candidate scores < 3.0 and no food attraction, even if not strictly stagnant

### strategy.hpp — choose() — head trades
Old: `all.empty() || (count>=8 && length<=3 && enemy_size[a.kill]>=8)` — took any trade when cornered

New logic:
- **No safe moves**: trade only if `count > 1` (an ally survives) or `count == 1 && only 1 visible enemy` (draw chance)
- **Trapped but has moves**: trade only if `count >= 3` AND `length <= 4` (truly expendable)
- **Has safe moves and not trapped**: never trade

## Local evaluation results

### Seed 101 — 8 maps x 2 sides x 2 opponents = 32 games
- **27 wins, 0 draws, 5 losses (84.4%)**
- Portals: 4/4 wins (was 0/4 in v1 online)
- Slithery_fight: 4/4 wins
- Losses: autarky (2) and stronghold (2) vs hunter, autarky (1) vs greedy

### Seed 42 — same maps = 32 games
- **28 wins, 0 draws, 4 losses (87.5%)**
- Autarky vs hunter now wins (was lost on seed 101)
- Losses: stronghold (2) vs hunter, autarky (2) vs greedy

### v2 vs v1 (sheltered baseline) — 8 maps x 2 sides x 2 seeds = 32 games
- **21 wins, 0 draws, 11 losses (65.6%)**
- Clean sweeps: default_small (4/4), devil (4/4), dilemma (4/4), portals (4/4)
- Arena: 3/4 wins
- Losses concentrate on: autarky (0/4), stronghold (1/4), slithery_fight (1/4)
- All losses are 500-round scoring games — v1's conservative single-dragon growth can outscore v2's swarm on food-sparse maps
- v2 wins through faster elimination and population advantage on resource-rich maps

## Where I left off

- Code changes are complete and compile cleanly
- All 13 unit tests pass
- Local results show clear improvement on portal/slithery_fight maps
- Stronghold remains weakest map against hunter — may need map-specific investigation
- **Not yet done:**
  - No sandbox runs (user preference: learn from local fights)
  - No packaging or submission
  - Gamplan.md not yet updated for v2 changes
  - No v2 evaluation against the actual online opponents (blauerdrache/bongcloud patterns)
  - Stronghold-specific analysis
  - Autarky loss pattern investigation (round-limit losses suggest food collection issue)

## Files changed
- `bot/config.hpp` — 3 constants modified
- `bot/strategy.hpp` — 4 code blocks modified (safe_split, incomplete split, portal, head trades)
- `evaluations/v2-local.json` — seed 101 results
- `evaluations/v2-local-seed42.json` — seed 42 results
