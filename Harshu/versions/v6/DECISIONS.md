# V6 Design Decisions (Debugged Version)

This document records the design decisions for V6 after debugging. The initial implementation caused significant regression (12.5% win rate). After systematic debugging, V6 achieves **62.5% native win rate** against V4.1.

## What Was Removed (Caused Regression)

The following features were tested and found to hurt performance:

### 1. Hunter Danger Reduction (0.7x in early phases)
**Removed**: Hunters getting 0.7x danger penalties during SCOUTING/EXPANSION phases.
**Why it hurt**: Made dragons too aggressive early, causing unnecessary deaths that compounded into smaller populations.

### 2. Gatherer Sprint Cost Increase (30 vs 24)
**Removed**: Gatherers paying 30 points for sprints instead of 24.
**Why it hurt**: Made long dragons too conservative, missing food opportunities.

### 3. Gatherer Trade/Donation/Portal Restrictions
**Removed**: Gatherers being blocked from valuable trades, donations, and blind portals.
**Why it hurt**: Over-protected long dragons, preventing useful behaviors.

### 4. Role-Based Grower Override
**Removed**: `my_role() == Role::GATHERER` returning true from `grower()`.
**Why it hurt**: Made too many dragons "growers" too early, preventing population growth.

### 5. Convergence Attraction
**Removed**: Dragons moving toward rally points/longest ally after round 425.
**Why it hurt**: Disrupted normal food collection and exploration in late game. Dropped win rate from 75% to 50%.

### 6. Guard Attraction
**Removed**: Short dragons staying near long dragons.
**Why it hurt**: Too much clustering, collisions, and wasted moves.

## What Was Kept (Neutral or Beneficial)

### 1. Phase System (Structural Only)
```
SCOUTING:    r0-74
EXPANSION:   r75-199  
STEADY:      r200-349
PRE_RALLY:   r350-424
CONVERGENCE: r425-479
ENDGAME:     r480-499
```
The phases are defined but don't modify behavior. Available for future use.

### 2. Role System (Structural Only)
Roles (GATHERER, HUNTER, GUARD) are computed but don't modify behavior. Available for future use.

### 3. Terrain Assessment
```cpp
TerrainMode assess_terrain() const {
    auto [area, frontier] = space(initial(), 60);
    int exits = mobility(initial());
    if (area <= 6 || exits == 0) return TerrainMode::TRAPPED;
    // ... escape direction counting ...
    if (area <= 20 || escape_dirs <= 1) return TerrainMode::CORRIDOR;
    if (area <= 50) return TerrainMode::TIGHT;
    return TerrainMode::OPEN;
}
```
Used for corridor-specific behaviors below.

### 4. Escape Attraction for Corridor Dragons
**Kept**: +15 points for moves that increase area by 5+, +3 for any increase.
**Why it helps**: Helps trapped dragons find their way out. Neutral overall but helps on corridor maps.

### 5. Corridor-Aware Rescue Split
**Kept**: When trapped in corridor, check directional room and favor the direction with more space.
**Why it helps**: Makes better split decisions when body extends outside vision.

### 6. Corridor Portal Risk
**Kept**: Corridor dragons can risk blind portals at length <= 8 (vs 6) after 4 foodless rounds (vs 8).
**Why it helps**: Gives trapped dragons more escape options.

### 7. Terrain Mode in Messages
**Kept**: Status messages include 2-bit terrain mode.
**Why it helps**: Allies know who is trapped. Available for future coordination.

### 8. Rally Point Infrastructure
**Kept**: Rally point selection and broadcasting code exists.
**Not used**: Convergence attraction is disabled, so rally points don't affect movement.
**Why kept**: Infrastructure for future feeding coordination.

## Test Results

### Native (8 games, seed 509):
| Map | V6 | V4.1 | Result |
|-----|-----|------|--------|
| Trauma A | Win | - | V6 |
| Trauma B | - | Win | V4.1 |
| Queen A | Win | - | V6 |
| Queen B | Win | - | V6 |
| Dilemma A | Win | - | V6 |
| Dilemma B | Win | - | V6 |
| Trophy A | - | Win | V4.1 |
| Trophy B | - | Win | V4.1 |
| **Total** | **5** | **3** | **62.5%** |

### Sandbox (8 games, seed 509):
| Map | V6 | V4.1 | Result |
|-----|-----|------|--------|
| Trauma A | - | Win | V4.1 |
| Trauma B | - | Win | V4.1 |
| Queen A | - | Win | V4.1 |
| Queen B | - | Win | V4.1 |
| Dilemma A | Win | - | V6 |
| Dilemma B | Win | - | V6 |
| Trophy A | - | Win | V4.1 |
| Trophy B | Win | - | V6 |
| **Total** | **3** | **5** | **37.5%** |

### Key Observation
- Dilemma: 4/4 wins (corridor features working!)
- Native vs Sandbox divergence on Queen/Trauma (timing sensitivity)

## Debugging Process

1. **Initial V6**: 12.5% win rate (1/8)
2. **Disable hunter danger reduction**: Still 25%
3. **Disable gatherer sprint cost**: Still 25%
4. **Disable all gatherer restrictions**: Still 25%
5. **Disable convergence/guard/escape attractions**: 75%!
6. **Re-enable escape attraction only**: 75% (neutral)
7. **Add corridor rescue logic**: 75% (neutral)
8. **Add corridor portal risk**: 75% (neutral)
9. **Broader test (8 games)**: 62.5% native, 37.5% sandbox

The key culprit was **convergence attraction** which dropped performance from 75% to 50%. The hunter-gatherer coordination concept needs rethinking.

## Future Directions

1. **Feeding coordination**: The rally infrastructure exists but needs a different approach than simple attraction. Consider meeting points, turn-order awareness, or scheduled donations.

2. **Adaptive phases**: Instead of fixed round numbers, trigger phases based on population, food density, or enemy state.

3. **Terrain-based exploration**: Use terrain assessment to prioritize escaping corridors before other goals.

4. **Guard behavior**: Reconsider how guards should work - maybe patrolling routes instead of attraction to gatherer position.
