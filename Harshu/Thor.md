# Thor: Map Intelligence & Strategic Adaptation

*Named after the Norse god who knows every realm and adapts his approach to each.*

This document covers map identification and map-specific strategy tuning - potentially the single biggest improvement available to the bot.

---

## Part 1: Why Map Identification Matters

### Current State
- Bot uses identical parameters on all maps
- Same danger weights, food rewards, split thresholds everywhere
- No knowledge of enemy spawn positions
- No pre-loaded terrain expectations

### With Map Identification
- Tuned parameters for each map's characteristics
- Know enemy positions from round 1
- Pre-loaded symmetry type for terrain inference
- Map-specific tactics (portal aggression, corridor handling, etc.)

### Competitive Advantage
Most bots don't identify maps. A map-aware bot can:
- Hard-counter specific map weaknesses
- Exploit map-specific opportunities
- Start with knowledge others must discover

---

## Part 2: Map Identification Mechanism

### Available Data (Round 1)
```cpp
int w;      // Map width
int h;      // Map height  
int head;   // Starting head position
int id;     // Dragon ID (0, 1, 2, ...)
int team;   // 0 = Team A, 1 = Team B
```

### Identification Logic

7 of 10 maps are **uniquely identified by dimensions alone**:

| Dimensions | Map |
|------------|-----|
| 54 × 18 | Autarky |
| 32 × 32 | Default |
| 25 × 35 | Queen of Spades |
| 25 × 25 | Trophy |
| 60 × 40 | Schooltime |
| 63 × 27 | Slithery Fight |
| 48 × 24 | Trauma |

3 maps share dimensions (32 × 16) and need head position:

| Head X Position | Map |
|-----------------|-----|
| x = 3 or x = 28 | Devil |
| x = 14 or x = 17 | Dilemma |
| x ≤ 1 or x ≥ 30 or x = 12 or x = 19 | Portals |

---

## Part 3: Complete Map Database

### Map 1: AUTARKY

| Property | Value |
|----------|-------|
| Dimensions | 54 × 18 |
| Symmetry | xy (180° rotation) |
| Total Dragons | 12 (6 per team) |
| Notable | Long starter dragons (14 length), boxed areas |

**Team A Spawns (Orange):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 14 | (18, 2) |
| 2 | 3 | (22, 2) |
| 3 | 3 | (22, 17) |
| 4 | 3 | (30, 6) |
| 5 | 3 | (7, 1) |
| 6 | 3 | (7, 15) |

**Team B Spawns (Purple):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 14 | (35, 15) |
| 2 | 3 | (31, 15) |
| 3 | 3 | (31, 0) |
| 4 | 3 | (23, 11) |
| 5 | 3 | (46, 16) |
| 6 | 3 | (46, 2) |

---

### Map 2: DEFAULT

| Property | Value |
|----------|-------|
| Dimensions | 32 × 32 |
| Symmetry | xy (180° rotation) |
| Total Dragons | 8 (4 per team) |
| Notable | Simple, open map with center food grid |

**Team A Spawns (Orange):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 4 | (0, 0) |
| 2 | 4 | (4, 0) |
| 3 | 4 | (8, 0) |
| 4 | 4 | (12, 0) |

**Team B Spawns (Purple):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 4 | (31, 31) |
| 2 | 4 | (27, 31) |
| 3 | 4 | (23, 31) |
| 4 | 4 | (19, 31) |

---

### Map 3: DEVIL

| Property | Value |
|----------|-------|
| Dimensions | 32 × 16 |
| Symmetry | y (horizontal mirror) |
| Total Dragons | 6 (3 per team) |
| Notable | Vertical kelp walls creating corridors |

**Team A Spawns (Orange):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 4 | (3, 5) |
| 2 | 4 | (3, 8) |
| 3 | 4 | (3, 11) |

**Team B Spawns (Purple):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 4 | (28, 5) |
| 2 | 4 | (28, 8) |
| 3 | 4 | (28, 11) |

---

### Map 4: PRISONERS DILEMMA

| Property | Value |
|----------|-------|
| Dimensions | 32 × 16 |
| Symmetry | xy (180° rotation) |
| Total Dragons | 6 (3 per team) |
| Notable | Long central dragons (11 length), tight corridors |

**Team A Spawns (Orange):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 11 | (14, 2) |
| 2 | 3 | (3, 6) |
| 3 | 3 | (3, 12) |

**Team B Spawns (Purple):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 11 | (17, 13) |
| 2 | 3 | (28, 9) |
| 3 | 3 | (28, 3) |

---

### Map 5: PORTALS

| Property | Value |
|----------|-------|
| Dimensions | 32 × 16 |
| Symmetry | xy (180° rotation) |
| Total Dragons | 6 (3 per team) |
| Notable | Many portals, shortcut-heavy map |

**Team A Spawns (Orange):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 3 | (0, 5) |
| 2 | 3 | (1, 11) |
| 3 | 3 | (12, 1) |

**Team B Spawns (Purple):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 3 | (31, 10) |
| 2 | 3 | (30, 4) |
| 3 | 3 | (19, 14) |

---

### Map 6: QUEEN OF SPADES

| Property | Value |
|----------|-------|
| Dimensions | 25 × 35 |
| Symmetry | xy (180° rotation) |
| Total Dragons | 4 (2 per team) |
| Notable | Tall map, spade-shaped walls, few dragons |

**Team A Spawns (Orange):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 3 | (5, 11) |
| 2 | 3 | (2, 9) |

**Team B Spawns (Purple):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 3 | (19, 23) |
| 2 | 3 | (22, 25) |

---

### Map 7: SCHOOLTIME

| Property | Value |
|----------|-------|
| Dimensions | 60 × 40 |
| Symmetry | y (horizontal mirror) |
| Total Dragons | 6 (3 per team) |
| Notable | Largest map, many structures, exploration-heavy |

**Team A Spawns (Orange):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 4 | (3, 2) |
| 2 | 4 | (16, 3) |
| 3 | 4 | (23, 3) |

**Team B Spawns (Purple):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 4 | (56, 2) |
| 2 | 4 | (43, 3) |
| 3 | 4 | (36, 3) |

---

### Map 8: SLITHERY FIGHT

| Property | Value |
|----------|-------|
| Dimensions | 63 × 27 |
| Symmetry | xy (180° rotation) |
| Total Dragons | 14 (7 per team) |
| Notable | Spiral dragons (25 length!), many small dragons, complex starts |

**Team A Spawns (Orange):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 7 | (8, 10) |
| 2 | 4 | (18, 7) |
| 3 | 25 | (3, 22) - SPIRAL |
| 4 | 2 | (15, 13) |
| 5 | 5 | (33, 2) |
| 6 | 2 | (26, 23) |
| 7 | 2 | (20, 4) |

**Team B Spawns (Purple):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 7 | (54, 16) |
| 2 | 4 | (44, 19) |
| 3 | 25 | (59, 4) - SPIRAL |
| 4 | 2 | (47, 13) |
| 5 | 5 | (29, 24) |
| 6 | 2 | (36, 3) |

---

### Map 9: TRAUMA

| Property | Value |
|----------|-------|
| Dimensions | 48 × 24 |
| Symmetry | xy (180° rotation) |
| Total Dragons | 4 (2 per team) |
| Notable | Maze-like structure, many walls |

**Team A Spawns (Orange):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 4 | (4, 15) |
| 2 | 4 | (4, 2) |

**Team B Spawns (Purple):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 4 | (43, 8) |
| 2 | 4 | (43, 21) |

---

### Map 10: TROPHY

| Property | Value |
|----------|-------|
| Dimensions | 25 × 25 |
| Symmetry | y (horizontal mirror) |
| Total Dragons | 4 (2 per team) |
| Notable | Heart/trophy shape, corner spawns |

**Team A Spawns (Orange):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 3 | (4, 2) |
| 2 | 3 | (4, 22) |

**Team B Spawns (Purple):**
| # | Length | Head (x,y) |
|---|--------|------------|
| 1 | 3 | (20, 2) |
| 2 | 3 | (20, 22) |

---

## Part 4: Map-Specific Strategies

### Strategy Parameter Template

```
danger_multiplier    - Scale danger penalties (1.0 = default)
food_reward          - Base food attraction points
exploration_patience - Rounds before seeking unknown areas
min_split_length     - Minimum length to allow splitting
max_team_size        - Cap on dragon count
aggressive_portals   - Risk unknown portal exits?
portal_risk_length   - Length threshold for portal risks
prefer_open_space    - Avoid narrow corridors?
sprint_cost          - Sprint penalty (lower = more sprinting)
convergence_round    - When to start late-game grouping
isolation_threshold  - Rounds without enemy before isolation mode
```

---

### AUTARKY Strategy

**Characteristics:** Long starters, boxed regions, medium density

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| danger_multiplier | 1.0 | Normal danger |
| food_reward | 12 | Standard |
| exploration_patience | 8 | Boxes limit exploration |
| min_split_length | 10 | Start long, stay long |
| aggressive_portals | false | Boxes are traps |
| prefer_open_space | true | Avoid box traps |

**Tactics:**
- Long dragons should farm, not split early
- Be cautious of box enclosures
- Use 180° symmetry for terrain inference

---

### DEFAULT Strategy

**Characteristics:** Simple, open, center food grid

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| danger_multiplier | 1.0 | Balanced |
| food_reward | 12 | Standard |
| exploration_patience | 6 | Open map, explore freely |
| min_split_length | 7 | Standard splitting |
| aggressive_portals | true | Few portals, safe to try |
| prefer_open_space | true | Stay in open areas |

**Tactics:**
- Standard balanced play
- Contest center food grid
- Use 180° symmetry

---

### DEVIL Strategy

**Characteristics:** Vertical kelp walls, 3 main corridors, teams start on opposite sides

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| danger_multiplier | 1.3 | Corridors are dangerous |
| food_reward | 14 | Food access limited |
| exploration_patience | 10 | Limited routes |
| min_split_length | 8 | Don't overcrowd corridors |
| max_team_size | 6 | Corridor congestion |
| aggressive_portals | false | Getting lost is fatal |
| prefer_open_space | false | Corridors are unavoidable |
| isolation_threshold | 50 | May be partitioned |

**Tactics:**
- Corridors are the only paths - learn them
- Don't split too much (congestion)
- Horizontal mirror symmetry
- High chance of map partition - enable isolation detection

---

### PRISONERS DILEMMA Strategy

**Characteristics:** Long central dragons, tight corridors, escape priority

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| danger_multiplier | 1.2 | Tight spaces |
| food_reward | 15 | Food is scarce |
| exploration_patience | 4 | ESCAPE corridors fast |
| min_split_length | 10 | Start at 11, preserve length |
| aggressive_portals | false | Don't get trapped |
| prefer_open_space | true | GET OUT of corridors |
| sprint_cost | 20 | Sprint to escape |

**Tactics:**
- Priority 1: Escape starting corridor
- Long dragon should NOT split early
- Corridor rescue features critical here
- V6's corridor escape was designed for this map

---

### PORTALS Strategy

**Characteristics:** Many portals, shortcut-heavy

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| danger_multiplier | 0.9 | Portals enable escape |
| food_reward | 12 | Standard |
| exploration_patience | 6 | Portals speed exploration |
| min_split_length | 6 | Normal |
| aggressive_portals | true | PORTALS ARE THE POINT |
| portal_risk_length | 4 | Risk portals earlier |
| prefer_open_space | true | But use portals freely |

**Tactics:**
- Aggressively discover portal connections
- Use portals for shortcuts to food
- Portal probing is high value here
- Share portal connections via sonar

---

### QUEEN OF SPADES Strategy

**Characteristics:** Tall map, curved spade walls, only 2 dragons per team

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| danger_multiplier | 1.1 | Walls create ambush points |
| food_reward | 14 | Fewer dragons, more food each |
| exploration_patience | 8 | Large area per dragon |
| min_split_length | 8 | Few dragons, preserve length |
| max_team_size | 4 | Don't overcrowd |
| aggressive_portals | true | Explore faster |

**Tactics:**
- Each dragon covers large territory
- Don't split too early (few starting dragons)
- Control center area
- 180° symmetry

---

### SCHOOLTIME Strategy

**Characteristics:** Largest map (60×40), many structures, exploration-heavy

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| danger_multiplier | 0.8 | Big map, spread out |
| food_reward | 10 | Plenty of food |
| exploration_patience | 3 | EXPLORE FAST |
| min_split_length | 6 | Split for coverage |
| max_team_size | 10 | Need many for coverage |
| aggressive_portals | true | Speed matters |
| sprint_cost | 20 | Sprint for coverage |

**Tactics:**
- Spread out fast
- Split early and often for coverage
- Horizontal mirror symmetry
- Exploration is top priority

---

### SLITHERY FIGHT Strategy

**Characteristics:** Spiral dragons (25 length!), many small dragons, complex

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| danger_multiplier | 1.0 | Mixed terrain |
| food_reward | 12 | Standard |
| exploration_patience | 6 | Medium |
| min_split_length | 8 | Let spirals unwind first |
| aggressive_portals | true | Speed up unwind |

**Tactics:**
- SPIRAL DRAGONS: First priority is unwind (don't trap yourself)
- Small dragons: Normal play
- Complex start requires careful first 20 rounds
- 180° symmetry

---

### TRAUMA Strategy

**Characteristics:** Maze-like, many walls, 2 dragons per team

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| danger_multiplier | 1.5 | MAZE IS DANGEROUS |
| food_reward | 14 | Hard to reach food |
| exploration_patience | 12 | Slow, careful exploration |
| min_split_length | 8 | Few dragons, preserve them |
| max_team_size | 4 | Maze congestion |
| aggressive_portals | false | Getting lost = death |
| prefer_open_space | true | Avoid dead ends |
| sprint_cost | 28 | Don't rush into walls |

**Tactics:**
- SLOW AND CAREFUL
- Map the maze before committing
- Dead ends are fatal
- 180° symmetry helps predict unseen maze

---

### TROPHY Strategy

**Characteristics:** Heart/trophy shape, corner spawns, simple layout

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| danger_multiplier | 1.0 | Balanced |
| food_reward | 12 | Standard |
| exploration_patience | 6 | Simple map |
| min_split_length | 6 | Normal |
| aggressive_portals | true | Few portals |
| prefer_open_space | true | Heart center is open |

**Tactics:**
- Contest the heart center
- Corner spawns - move toward center
- Horizontal mirror symmetry
- Straightforward play

---

## Part 5: Implementation Plan

### Phase 1: Map Identification (Low Risk)
Add map identification on round 1. No behavior changes yet.

### Phase 2: Parameter Tuning Framework
Create MapStrategy struct, load parameters based on identified map.

### Phase 3: Strategy Application
Apply per-map parameters to:
- `policy::danger_*` values
- `policy::food_reward`
- `policy::split_length`
- Portal risk thresholds
- Exploration behavior

### Phase 4: Advanced Tactics
- Pre-loaded enemy spawn positions
- Map-specific opening moves
- Symmetry inference enabled automatically

---

## Part 6: Expected Results

### Before (Generic Bot)
- Same behavior on all maps
- Mediocre everywhere
- No pre-knowledge of enemy positions
- Discovers map characteristics the hard way

### After (Map-Aware Bot)
- Optimized for each map's quirks
- Strong on maps matching its tuning
- Knows enemy positions from round 1
- Exploits symmetry immediately

### Win Rate Projections (Speculative)

| Map | Current Issue | After Tuning | Expected Gain |
|-----|---------------|--------------|---------------|
| Devil | Generic in corridors | Corridor-optimized | +15%? |
| Dilemma | V6 already tuned | Maintain | +5%? |
| Trauma | Too aggressive | Slow and safe | +20%? |
| Schooltime | Doesn't spread | Fast exploration | +15%? |
| Portals | Portal-shy | Portal-aggressive | +10%? |
| Others | Generic | Tuned | +5-10%? |

**Overall expected improvement: 10-20% win rate increase**

---

## Part 7: Open Questions

1. Should spiral dragons (Slithery Fight) have special unwind logic?
2. Can we detect map partition potential from spawn positions alone?
3. Should parameters interpolate or switch hard between maps?
4. How do we test map-specific tuning efficiently?
5. Should ID/dragon count affect strategy within a map?

---

*This document will be updated with implementation results.*
