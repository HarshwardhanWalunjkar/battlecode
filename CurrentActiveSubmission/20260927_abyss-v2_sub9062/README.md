# abyss-v2 — Active Submission

| Field | Value |
|---|---|
| Name | abyss-v2 |
| Submission ID | 9062 |
| Team ID | 422 |
| Uploaded | 2026-09-27 18:06:18 UTC / 2026-09-27 23:36 IST |
| Fingerprint | `28cbd6c113a585daf64b7f2b99c3cf37ce1be06fb98f2dfc92965aa1ca11fce8` |
| Language | C++20 |
| Toolkit | 1.2.2 |
| Replaces | abyss-v1 (submission 9003) |

---

## Complete game plan

### 1. What we are trying to achieve

We want the best rank and rating we can earn. Inside a game, keeping at least one dragon alive comes first. If neither team is eliminated, the longest living dragon decides the winner; total team length matters only if those longest lengths tie.

We collect food while keeping enough room to continue moving. We aggressively create additional dragons to build a population that can absorb losses, contest territory, and replace eliminated members. We allow up to 48 dragons through normal splitting and continue splitting through round 420. We do not try to fill all 64 team slots, but we aim for a substantial population rather than relying on a few large dragons.

Every dragon runs the same rules independently. There is no all-seeing team captain. A dragon knows its own complete history when that history can be checked, its current view, its remembered map, and the recent messages it accepts.

### 2. What we remember and what we do not assume

We remember walls and discovered portal connections for the whole game. We update food, bodies and heads whenever a tile comes into view. Another dragon's last known body remains an obstacle for the current round and one additional round. Older body observations are no longer treated as certain obstacles.

We remember our own expected body after each action. On the next turn, we check that its length, head and visible segments agree with what the game reports. If they disagree, we rebuild the body from visible segments that point towards one another. A fresh child does the same reconstruction. If we cannot recover the complete body, we avoid guessing which unseen tail tiles have become free and avoid sprinting. However, we proactively split off the unseen tail when our length is at least five, keeping a known two-segment parent and giving the rear its own process. In an emergency, we split even earlier. This prevents long-body starts from becoming death traps.

Old food sightings remain potential destinations for at most 15 rounds. Their appeal falls with age. A previously empty unseen tile is never assumed safe for an action we are about to take. We normally commit only to destinations currently visible, including every step of a sprint. Longer imagined routes may go through remembered terrain; we reconsider them after the next observation.

We currently do not infer unexplored terrain from map symmetry. This avoids committing to an unconfirmed map shape. Unknown terrain can still encourage exploration through known safe steps.

### 3. How we choose a move

We list legal routes of one, two, and three steps for the current turn. We check every step separately: its wall or portal, the destination, our entire known body, other bodies, food, and the length cost of extra steps. Going into the current tail is forbidden, even when it would otherwise move away.

We give each route an initial score for food, room to move, exits, nearby enemy threats, repeated visits, and lost length. If at least one route has an exit, at least six reachable tiles, and no enemy attack within three steps, we discard routes with an enemy attack within three steps. This prevents tempting food from overriding an available sheltered move. We keep the best 14 routes for closer examination. For each, we imagine up to 10 further turns, normally one step per turn. At each imagined turn we keep the seven most promising continuations. We stop imagining turns beyond the end of the actual game.

Other dragons do not move inside these imagined continuations. Their current bodies are obstacles, and their nearby heads add danger. Future food appearances are also uncertain: we use their timing to choose attractive destinations, but do not pretend they have already fed us.

We check room again at the ends of the imagined routes. A route that collects food and then has no continuation receives a large penalty. We choose the highest-scoring current action, then throw away the rest of its imagined sequence and plan again next turn.

The search stops after 0.052 seconds on the game's own clock, measured after input parsing (about 52 million work points in the judge).

### 4. Food and growth

A pearl collected on the current action adds 12 preference points. Food in later continuations is discounted by 0.94 per imagined turn.

We consider up to 12 food destinations ranked by reachability through known terrain. A seen pearl has confidence `1 / (1 + 0.09 * rounds_since_seen)`. A tile due to spawn within six rounds has confidence `0.45 / (1 + 0.15 * rounds_until_attempt)`. A destination's attraction is `9 * confidence / (1 + 0.38 * steps + sharing_adjustment)`. The sharing adjustment of 2 applies when a reported longer ally is closer to that food.

### 5. Room, loops and trapped positions

Reachable tiles around a proposed head add `2 * ln(1 + tiles)` preference points. Enclosed regions smaller than body length plus five lose 4 points per missing tile. No exit costs 180 points; one exit costs 9. Recently visited tiles lose 0.7 points per round under an 8-round gap.

In imagined turns: no exit costs 150, one exit costs 5. Revisits within 6 rounds lose 0.18 per missing round. Nearby-enemy danger is reduced to 13% and divided by turn distance. At search end, enclosed positions smaller than body plus 3 lose 3 points per tile. Complete dead ends contribute minus 200 plus 12 times the depth they occurred.

### 6. Sprinting

We consider up to three steps per action. Extra steps cost 14 preference points (22 after round 450). We check affordability before each extra step. A pearl at the destination cannot pay for an already-unaffordable step. We update the body after every step. No blind multi-step sprints; if the body is incomplete, only single steps.

### 7. Enemy threats and deliberate attacks

Enemy routes to our head within three steps are scored: one-step = 130 penalty, two-step = 36, three-step = 6. The two and three step penalties are deliberately low because against swarm opponents, distant threats are everywhere. Heavy penalties for ubiquitous danger made all moves equally bad.

Head trades are handled selectively:
- **No safe moves**: take the trade only if at least one ally survives, or if we are the last dragon and only one enemy head is visible (draw chance)
- **Trapped but moves exist**: take only if we have 3+ living dragons and length 4 or less (expendable)
- **Good safe moves available**: never trade

Action priority: split if conditions pass > incomplete-body split > exploratory portal > qualifying head trade > best searched move > avoid ally head > move forward.

### 8. Splitting

Normal expansion starts at length four. We split exactly two rear segments. We allow up to 48 dragons through round 420, with one round cooldown. A dragon that collects two pearls immediately splits into two length-two dragons.

Normal splits require the child to have one exit and four reachable tiles, no immediate enemy threat on either parent or child head.

Emergency splits (when trapped) need only two reachable tiles and one exit. They ignore the 48-dragon target, cooldown, and round limits. They also work with partially known bodies (needing only two reconstructed segments). The game's team cap of 64 and minimum length of 2 always apply.

### 9. Sonar and sharing food

We send head position, length, dragon number, team, and round in two opposite directions (rotating by round and dragon number). Messages include a check value. We accept only matching-team messages with valid coordinates, lengths, check values, and timestamps within two rounds. Accepted reports reduce food competition when a longer ally is closer. They never override visible obstacles.

### 10. Portals and last resorts

Normal portal moves require a visible destination. We take blind portals (unseen exit) in three situations:
1. No safe local route exists
2. No food for 5 rounds and no portal for 8 rounds, with no immediate food on any candidate
3. All candidates score below 3.0 with no food attraction, after round 3

After a blind portal, we forget the expected body and reconstruct from the next observation.

### 11. End of the game

Rounds are 0 to 499. Normal splitting stops at round 420. Late sprints cost 22 instead of 14 after round 450. The imagined future stops at round 499. Survival, food and length preservation remain priorities.

### 12. Concealment

Close move choices receive a small deterministic variation (under 0.5 points) based on dragon, round, and destination. The bot does not print scores or targets into replays. It does not weaken play to hide strategy.

---

## What v2 fixes and how

### Problem 1: Reckless head trades (99/114 own deaths were self-initiated)

**Root cause in v1:** The bot took any available head trade when cornered (`all.empty()`), regardless of team population. Against opponents with 30-64 dragons, our bot with 4 dragons lost a member on every trade while the opponent barely noticed.

**v2 fix:** Head trades now require `count > 1` (an ally survives us) or a genuine draw chance (we are the last dragon and only one enemy is visible). When trapped but moves exist, we only trade if we have 3+ dragons and are expendable (length <= 4). We never trade when safe moves are available.

**Code location:** `strategy.hpp` lines 381-389, the `attacks` loop in `choose()`

**Measured impact:** The v1 replay audit showed 99 self-initiated head collisions across 20 games. The selective logic should reduce this to trades that genuinely benefit the team.

### Problem 2: Population disadvantage (our 4 dragons vs their 21-64)

**Root cause in v1:** `max_team = 4`, `split_length = 6`, `split_cooldown = 12`, `split_before = 260`. The bot intentionally limited itself to four dragons, split only from length 6, waited 12 rounds between splits, and stopped after round 260.

**v2 fix:** `max_team = 48`, `split_length = 4`, `split_cooldown = 1`, `split_before = 420`. Dragons now split as soon as they reach length 4 (after 2 pearls), creating a multiplication cycle: eat two pearls, split into 2+2, repeat. The population cap of 48 allows us to compete with swarm opponents. Splitting continues through round 420 to replace losses.

**Code location:** `config.hpp` lines 7-10

**Measured impact:** Local games show substantially higher dragon counts and territory control. Win rate against hunter baseline improved.

### Problem 3: Portal avoidance (zero portal transitions, starvation on portal maps)

**Root cause in v1:** The bot only took blind portals when `round - last_food >= 8 AND round - last_portal >= 12 AND length <= 6`. This meant it effectively never took portals, cycling endlessly through explored local territory.

**v2 fix:** Three portal triggers:
- Stagnation: food gap reduced from 8 to 5 rounds, portal gap from 12 to 8, length restriction removed
- Low prospects: portals taken when all candidates score below 3.0 with no food attraction (even if not starving)
- No moves: unchanged (always take portal over guaranteed death)

**Code location:** `strategy.hpp` lines 374-379, the `portal`/`stagnant`/`low_prospects` block in `choose()`

**Measured impact:** 4/4 wins on Portals map (both seeds, both sides) where v1 lost every online Portals game.

### Problem 4: Long-body startup deaths (kelp collisions in round 2-3)

**Root cause in v1:** When the body extended beyond vision (common on maps with starting lengths > 7), the bot set `complete = false` and disabled all splits and sprints. With no split option and limited single-step moves, dragons walked into kelp.

**v2 fix:**
- Proactive incomplete-body split threshold lowered from length 8 to length 5: dragons shed their unseen tail earlier, creating a known two-segment parent and an independent child
- Emergency splits now work with incomplete bodies (only need 2 reconstructed segments)
- Emergency split room requirement lowered from 4 tiles to 2

**Code location:** `strategy.hpp` lines 322-333 (`safe_split`) and lines 370-373 (incomplete body split in `choose()`)

**Measured impact:** 4/4 wins on Slithery Fight (was a problem map in v1 online). Early kelp deaths on Autarky reduced.

### Problem 5: Far-threat panic (all moves scored equally against swarms)

**Root cause in v1:** `danger_two = 48.0`, `danger_three = 12.0`. Against swarms with enemies everywhere, nearly every tile had a 2-3 step threat, making the danger penalty dominate all other scoring factors. The bot couldn't differentiate food-rich safe moves from dead-end safe moves because both had similar threat scores.

**v2 fix:** `danger_two = 36.0`, `danger_three = 6.0`. The one-step penalty (130) remains high since immediate threats are real. But 2-step and 3-step penalties are reduced so food, room, and exit scoring can differentiate moves when distant threats are unavoidable.

**Code location:** `config.hpp` lines 14-15

**Measured impact:** Better food collection and positioning in games with high enemy density.

---

## Local evaluation results

### v2 vs hunter + greedy baselines

| Seed | Games | Wins | Losses | Rate |
|---|---|---|---|---|
| 101 | 32 | 27 | 5 | 84.4% |
| 42 | 32 | 28 | 4 | 87.5% |
| **Total** | **64** | **55** | **9** | **85.9%** |

### v2 vs v1 (sheltered baseline) head-to-head

| Seed | Games | Wins | Losses | Rate |
|---|---|---|---|---|
| 101 + 42 | 32 | 21 | 11 | 65.6% |

Clean sweeps on: default_small (4/4), devil (4/4), dilemma (4/4), portals (4/4).

### Known remaining weaknesses

- **Autarky**: food-sparse map where aggressive splitting dilutes individual growth; tends to lose on score at round 500
- **Stronghold**: narrow corridors may trap more dragons than they protect; consistent losses vs hunter
- **Scoring games vs conservative opponents**: v2's swarm can be outscored by a single large dragon on food-sparse maps

---

## Config values

```cpp
beam_width      = 7       // lookahead beam width
lookahead       = 10      // turns to search ahead
candidate_limit = 14      // top routes to deep-search
max_sprint      = 3       // max steps per action
split_length    = 4       // min length for normal split
max_team        = 48      // normal split population cap
split_before    = 420     // last round for normal splits
split_cooldown  = 1       // min rounds between splits
food_reward     = 12.0    // points for eating a pearl
sprint_cost     = 14.0    // penalty per extra sprint step
danger_one      = 130.0   // 1-step enemy penalty
danger_two      = 36.0    // 2-step enemy penalty
danger_three    = 6.0     // 3-step enemy penalty
search_seconds  = 0.052   // time budget per turn
```

## Files

| File | Purpose |
|---|---|
| `bot/main.cpp` | Game loop: init, observe, choose, commit |
| `bot/strategy.hpp` | Complete brain: terrain, search, splitting, portals, sonar |
| `bot/config.hpp` | All tunable constants |
| `bot/helper.hpp` | Official SDK helper (unmodified) |
| `bot/bot.toml` | Project manifest |
| `dist/abyss-v2.zip` | Exact submission archive |
| `dist/manifest.json` | File list and fingerprint |
| `evaluations/` | Local evaluation results (JSON) |
