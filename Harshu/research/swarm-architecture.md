# Coordinated swarm attack — architecture research

## The goal

Small dragons form attack groups that corner and kill enemy heads. Large dragons
play safe, grow, and feed the team's endgame score. Sonar coordinates roles and
targets. New children are recruited into the swarm when needed.

## Hard constraints we cannot change

### Per-dragon isolation
Each dragon is a **separate OS process**. There is no shared memory, no file
system, no sockets. A child starts with zero bytes of parent state. The only
inter-dragon channel is sonar.

### Sonar bandwidth
- **4 messages per turn**, one per cardinal direction (repeated direction overwrites)
- Each message is an unsigned **64-bit integer**
- Rays wrap the torus, pass through portals, stop at **kelp** or the **first dragon hit**
- Either team can receive — **no sender/team metadata** on the wire
- Delivery timing: higher IDs receive this round, lower IDs receive next round
- A dead dragon does not send

### Sonar echoes (receive side)
After your next turn begins, you get **one aggregate echo** for the directions
you sent last turn:
```
SonarEchoes { kelp, ally, ally_head, enemy, enemy_head }
```
These are **total counts** across all your sent directions, not per-direction.
No distance information. If you sent N and S, you know "2 enemy bodies were hit
somewhere along those two rays" but not which ray or how far.

### Map size
Width and height each 10–64. Coordinates use 6 bits each. Max board is 4096
tiles.

### ID assignment
IDs are assigned sequentially. A child gets the next unused ID. Lower IDs act
first in each round.

## Current message format (v2)

```
bits  0-5:   x coordinate (6 bits, max 63)
bits  6-11:  y coordinate (6 bits, max 63)
bits 12-23:  length, capped at 4095 (12 bits)
bits 24-32:  round number (9 bits, max 511)
bits 33-48:  dragon ID (16 bits)
bits 49:     team (1 bit)
bits 50-63:  checksum (14 bits)
```

**Total: 64 bits fully used.** The checksum validates the entire payload. This
format sends position, length, identity, and freshness. It has **zero spare
bits** for new fields.

## The fundamental design problem

We need to encode role assignments, enemy target locations, and swarm
instructions in a channel that:
1. Has no spare bits in the current format
2. Cannot be directed to a specific recipient
3. Can be intercepted by enemies
4. Has no guaranteed delivery (kelp/body blocks)
5. Provides no sender identity to the receiver
6. Delivers asynchronously (1-round delay for lower IDs)

## Proposed architecture: implicit roles + typed messages

### Core insight: roles should be inferred, not assigned

Instead of trying to tell each dragon its role (which requires per-recipient
addressing we don't have), each dragon **infers its own role** from local state:

```
IF length <= 3 AND count >= 4:
    role = ATTACKER
ELSE IF length >= 6:
    role = GROWER
ELSE:
    role = FLEXIBLE (default behavior)
```

This requires **zero communication**. A child born at length 2 is automatically
an attacker. A dragon that eats its way to length 6 transitions to grower. The
thresholds are tunable constants in config.hpp.

### Why this works better than assignment

- No message needed to assign roles
- Children inherit the correct role instantly (they're born small = attacker)
- No confusion from lost messages
- No way for the enemy to disrupt role assignment
- Transitions are automatic as dragons grow or shrink

### What roles change about behavior

**ATTACKER (length <= 3, count >= 4):**
- Reduced danger penalties (willing to approach enemies)
- Actively seeks positions adjacent to enemy heads
- Takes head trades more liberally (count > 1 is enough, no count >= 3 requirement)
- Does not split (stays small and expendable)
- Lower food attraction (doesn't compete with growers)

**GROWER (length >= 6):**
- Current v2 behavior: cautious, food-focused, splits when reaching length threshold
- Higher danger penalties (protect the long dragon)
- Prefers routes away from enemy heads
- Splits at length threshold to produce new attackers

**FLEXIBLE (length 4-5):**
- Current v2 behavior exactly
- Will naturally split down to length 2-3 (becoming attacker) or grow to 6+

## Message protocol redesign: two message types

Since we need 64 bits total and the current format uses all of them, we need a
**type bit** to distinguish message kinds. This costs us one bit elsewhere.

### Option A: steal from checksum (14 → 13 bits)

```
bit 63:      message type (0 = STATUS, 1 = THREAT)
bits 50-62:  checksum (13 bits, still 8192 values — collision rate ~0.01%)
```

**STATUS message (type 0) — same as current, minus 1 checksum bit:**
```
bits  0-5:   x (6)
bits  6-11:  y (6)
bits 12-23:  length (12)
bits 24-32:  round (9)
bits 33-48:  dragon ID (16)
bit  49:     team (1)
bits 50-62:  checksum (13)
bit  63:     0
```

**THREAT message (type 1) — reports an enemy location:**
```
bits  0-5:   enemy x (6)
bits  6-11:  enemy y (6)
bits 12-17:  enemy visible segments (6 bits, max 63)
bits 18-19:  enemy heading (2 bits, NESW)
bits 20-25:  reporter x (6)
bits 26-31:  reporter y (6)
bits 32-40:  round (9)
bit  41:     team (1)
bits 42-48:  reporter ID low bits (7)
bits 49-62:  checksum (14)
bit  63:     1
```

This lets a dragon that sees an enemy head broadcast its location so allies
(including those who can't see it) know where to converge.

### Option B: steal from ID (16 → 14 bits) and add 2 type bits

```
bits 62-63:  message type (2 bits = 4 types)
```

Four types: STATUS, THREAT, FOOD, RETREAT. But 14-bit IDs still cover 16384
values, far more than the 64-dragon cap. This is viable.

### Recommendation: Option A

Two message types (STATUS + THREAT) cover the essential coordination. Option B's
extra types (FOOD, RETREAT) can be approximated by what's already implicit:
- FOOD: allies already reduce competition via the sharing adjustment
- RETREAT: a dragon in trouble just stops sending (absence = possible death)

## Delivery strategy: which directions to send

Currently we send in two opposite directions, rotating by round and ID. With
coordination, we need to be smarter:

### STATUS messages: broadcast wide (current approach, keep it)
Send STATUS in two opposite directions, rotating. This maximizes the chance that
at least one ally receives our position.

### THREAT messages: target toward allies
When we see an enemy head, send the THREAT message toward the **direction where
allies are likely to be**. Use echoes from previous turns: if we got `ally > 0`
from the north, send the THREAT north.

### Sending schedule per turn (4 message slots)
```
Direction 1: STATUS message (rotating pair element 1)
Direction 2: STATUS message (rotating pair element 2)
Direction 3: THREAT message toward best-guess ally direction (if enemy visible)
Direction 4: THREAT message toward second-best-guess ally direction
```

If no enemy is visible, send STATUS in all 4 directions for better coverage.

## Attacker behavior: converging on threats

When an ATTACKER receives a THREAT message:
1. Validate checksum and freshness (within 2 rounds)
2. Record the enemy position in a `threats` table (similar to `allies`)
3. When choosing moves, add **attraction toward the reported enemy head**
4. The attraction decays with age (same as food decay) and distance

The attacker doesn't blindly rush — it still avoids lethal positions. But its
`root_score` now includes a positive term for being near a reported enemy head.
This creates **convergence without explicit coordination**: multiple small
dragons independently move toward the same reported threat.

### Convergence scoring

```cpp
double threat_attraction(State const& s) const {
    double best = 0;
    for (auto const& t : reported_threats) {
        if (round - t.round > 3) continue;  // stale
        int dist = distance(s.body[0], t.pos);
        double freshness = 1.0 / (1 + 0.3 * (round - t.round));
        double value = t.size * freshness / (1 + 0.5 * dist);
        best = std::max(best, value);
    }
    return best;
}
```

Add this to `root_score` for ATTACKER-role dragons. GROWER dragons ignore it or
subtract it (move away from threats).

## Recruiting new swarm members via splitting

No explicit "recruit" message needed. The natural flow:

1. A GROWER eats pearls and reaches split threshold (length 4)
2. It splits → child born at length 2
3. Child infers ATTACKER role (length 2, count presumably >= 4)
4. Child receives THREAT messages from nearby allies
5. Child moves toward reported enemies

The "split if valid and ordered to join the swarm" from the suggestion happens
**automatically**: every split produces a new attacker. The split frequency
already controls swarm recruitment rate.

If we want *emergency recruitment* (swarm is losing badly), a dragon could
split below the normal threshold. Trigger: many THREAT messages received but
declining ally count. Implementation:

```cpp
bool swarm_needs_help = reported_threats.size() >= 3 && count < prev_count;
if (swarm_needs_help && length >= 4) {
    // Emergency split even if conditions aren't ideal
    // Overrides normal split timing
}
```

## Sonar echo exploitation for mapping

Echoes are aggregated across all sent directions, which limits their usefulness.
To get directional information:

### Single-direction probing
Send sonar in only ONE direction per turn. The echo then tells you exactly
what's along that ray.

**Tradeoff:** You sacrifice 3 message slots (can only send 1 STATUS instead of
2). Worth it for a GROWER dragon that wants to map safely, not for an ATTACKER
that needs to broadcast threats.

### Echo-based enemy detection
```
Turn N: send sonar EAST only
Turn N+1: echoes say { enemy: 2, enemy_head: 1 }
→ "There is at least one enemy head somewhere east of me"
```

This is coarse but useful for:
- GROWER dragons choosing which direction to explore (avoid enemy-heavy dirs)
- ATTACKER dragons choosing which direction to patrol (seek enemy-heavy dirs)

### Implementation
```cpp
struct EchoMemory {
    int direction;    // which direction we probed
    int round;        // when
    SonarEchoes result;
};
std::vector<EchoMemory> echo_history;  // last 4 probes (one per direction)
```

Rotate probe direction each turn: N, E, S, W, N, E, S, W...
After 4 turns you have a coarse "enemy density by direction" map.

## Implementation phases

### Phase 1: Implicit roles (no protocol change needed)
- Add role inference to `Brain`: attacker/grower/flexible based on length+count
- Modify `root_score` for attackers: reduced danger penalties, slight attraction
  toward visible enemy heads
- Modify `root_score` for growers: increased danger penalties, stronger food focus
- **Test immediately against baselines**
- Estimated changes: ~30 lines in strategy.hpp, ~3 lines in config.hpp

### Phase 2: THREAT messages (protocol redesign)
- Implement Option A (1-bit type field, 13-bit checksum)
- Add THREAT message encoding/decoding alongside STATUS
- Add `reported_threats` table (like `allies` table)
- Attackers add threat_attraction to scoring
- Growers add threat_repulsion to scoring
- Send THREAT when enemy head is visible, directed toward last-known ally
- **Test convergence: do small dragons actually find and kill enemies?**
- Estimated changes: ~80 lines in strategy.hpp

### Phase 3: Echo probing (optional, independent of phases 1-2)
- GROWER dragons probe one direction per turn
- Store echo history, build directional enemy/kelp density model
- Use density model in exploration scoring (prefer low-enemy directions)
- **Test on large maps where beyond-vision awareness matters most**
- Estimated changes: ~40 lines in strategy.hpp

### Phase 4: Adaptive recruitment (depends on phases 1-2)
- Track swarm success (how many threats resolved, ally count trend)
- Emergency split triggers when swarm is depleted
- Growers protect scoring dragons more when team is winning
- Attackers become more conservative when team is losing (preserve numbers)
- Estimated changes: ~25 lines in strategy.hpp

## Risk analysis

### What could go wrong

| Risk | Severity | Mitigation |
|---|---|---|
| Attackers die too fast, waste population | High | Tune role thresholds; attacker still avoids 1-step threats |
| THREAT messages intercepted by enemy | Medium | They learn where we think they are — but they already know that |
| Echo probing reduces message coverage | Medium | Only growers probe; attackers broadcast fully |
| Enemy mimics our THREAT format | Low | Checksum + team bit; deliberate forgery requires knowing our hash |
| Convergence creates traffic jams | Medium | Limit max attackers converging on same threat (e.g., 3) |
| Roles create predictable replay patterns | Low | Small variation still applies; roles change as length changes |

### What definitely works

- Implicit roles require no communication and can't fail
- THREAT messages are a strict improvement (more information, same cost)
- Each phase is independently testable and rollback-safe

## Comparison with current v2

| Aspect | v2 now | With swarm coordination |
|---|---|---|
| Role differentiation | None — all dragons use same scoring | Attackers seek enemies, growers seek food |
| Enemy information sharing | None — each dragon sees only its own 7x7 | THREAT messages broadcast enemy locations |
| Attack coordination | Accidental — dragons trade when cornered | Deliberate — attackers converge on reported targets |
| Beyond-vision awareness | None | Echo probing maps enemy density by direction |
| Child behavior | Identical to parent logic | Born as attackers, naturally aggressive |
| Population management | Fixed split threshold | Adaptive: emergency splits when swarm depleted |

## Estimated total cost

- Phase 1: 1-2 hours implementation, immediately testable
- Phase 2: 3-4 hours implementation, needs careful message testing
- Phase 3: 1-2 hours implementation, independent
- Phase 4: 1 hour implementation, depends on phase 1-2 results

Phase 1 alone would be a meaningful improvement. Each subsequent phase adds
value independently.
