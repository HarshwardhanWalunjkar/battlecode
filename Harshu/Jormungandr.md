# Jormungandr: Advanced Communication Architecture

*Named after the World Serpent of Norse mythology - a dragon so vast it encircles the world and grasps its own tail.*

This document captures ongoing design discussions for sophisticated inter-dragon communication systems.

---

## Part 1: The Child Education Problem

### The Problem

When a dragon splits, the child is born "blind" - it inherits nothing from the parent's accumulated knowledge:

| Parent Has | Child Gets |
|------------|------------|
| Full map memory | Nothing |
| Known portal connections | Nothing |
| Food spawn patterns | Nothing |
| Enemy tracking | Nothing |
| Exploration progress | Nothing |

The child must rediscover everything from scratch, wasting turns that could be spent growing. This is especially costly in the early-mid game when efficient food collection matters most.

### Current Sonar Capabilities

- 4 directional sonar slots per turn (N, E, S, W)
- 64 bits per message
- Ray-based delivery (only entities on the ray receive the message)
- Currently used for: status broadcasts, rescue role messages, portal probing

### Proposed Solution: Birth-Turn Training Package

On the turn a dragon splits, the parent sends a "training package" to the child containing critical survival information:

**Priority information to transmit (64-bit budget):**
1. Nearest food position (12 bits) - immediate growth target
2. Second nearest food position (12 bits) - backup target
3. Danger direction (2 bits) - which way enemies are
4. Terrain mode (2 bits) - is child in corridor/trapped/open?
5. Nearest known portal exit (12 bits) - navigation shortcut
6. Timestamp and validation bits (remaining)

### The Ray-Based Delivery Challenge

Sonar travels in straight lines. The child's head position after split may not be on any cardinal ray from the parent's head:

```
Parent head at (3,2)
Child head at (5,4) after split

     0 1 2 3 4 5 6
   ┌───────────────
 2 │       P → → →   EAST ray (row 2) misses child
 3 │       ↓
 4 │       ↓   C     Child at row 4, col 5
           ↓
         SOUTH ray (col 3) also misses
```

If child head is diagonal from parent head, no single sonar ray can reach them.

### Solution: Educated Split Selection

**Key insight**: We know our own body positions exactly. We can predict precisely where the child's head will be after any split.

**The optimization**: When deciding to split, search for a split point where the child's head lands on a cardinal ray from our head, enabling direct sonar training.

**Constraints on educated splits:**
- Child length must remain viable (minimum ~3 segments)
- Parent length must remain viable
- Neither dragon should end up trapped (space check)
- Length variance from "default" split should be small (±2 segments)

**Decision hierarchy:**
1. If an educated AND safe split exists within ±2 of default → use it
2. If only non-educated safe options exist → use default, employ backup strategy
3. If no safe options → use default anyway (survival > education)

### Backup Strategy: Turn N+1 Training

If educated split isn't possible, parent can attempt training on the next turn:
- Parent remembers it just split and where the child approximately is
- Next turn, parent sends training toward child's expected position
- Child receives on their Turn 2 instead of Turn 1
- Slightly delayed but still valuable

### Open Question: Sonar Timing

Does the child's inbox on their first turn include sonar sent by the parent on the split turn?

- If sonar delivery happens AFTER split processing → child can receive
- If sonar delivery happens BEFORE split processing → child doesn't exist yet as recipient

Need to verify actual game mechanics. The belt-and-suspenders approach (send on both Turn N and Turn N+1) handles uncertainty.

---

## Part 2: The Map Sharing Problem

### The Problem

Each dragon explores independently and builds local map knowledge. This is inefficient:
- Dragon A discovers a portal connection, Dragon B has to rediscover it
- Dragon A maps a dead-end corridor, Dragon B walks into the same trap
- Collective map knowledge is fragmented across the team

### Map Structure

The map is approximately 60×28 cells (~1680 total). Terrain consists of:
- Open passages
- Walls (impassable)
- Portals (teleport to paired location)
- Unknown edges (not yet seen)

### Proposed Solution: Chunk-Based Map Sharing

Divide the map into 4×4 cell chunks (15×7 = 105 chunks total). Each chunk can be encoded and transmitted:

**Chunk encoding (64-bit message):**
- 24 internal edges × 2 bits = 48 bits (edge states: open/wall/unknown/portal)
- Chunk X coordinate (4 bits, 0-15)
- Chunk Y coordinate (3 bits, 0-7)
- Team bit (1 bit)
- Tag/checksum (8 bits)

When a dragon receives a map chunk message, it merges the information:
- If local edge is UNKNOWN and received is KNOWN → update local
- If both known → verify consistency (conflicts indicate issues)
- Never downgrade KNOWN to UNKNOWN

### Bandwidth Allocation

Current sonar usage sends the same status message in all 4 directions - redundant for allies not in multiple directions.

**Proposed allocation by round:**
```
Round % 4 == 0: Status in all 4 directions (full redundancy)
Round % 4 == 1: Status N+S, Map chunk E+W
Round % 4 == 2: Status in all 4 directions
Round % 4 == 3: Status N+S, Intel E+W
```

This maintains status coverage (rescue chains still work) while adding map/intel sharing.

### Chunk Priority Selection

Don't send random chunks. Prioritize:
1. Chunks containing portal exits (high navigation value)
2. Recently explored chunks (fresh, accurate data)
3. Chunks near known allies (they might need this info)
4. Chunks with food spawns
5. Avoid re-sending same chunk repeatedly

### Map Convergence

With 105 chunks and ~10 dragons sharing, how fast does the team build a complete map?

- Each dragon sends ~1 chunk per 4 rounds
- 10 dragons × 1 chunk/4 rounds = 2.5 chunks/round team-wide
- Full map coverage in ~42 rounds if no overlap
- Realistically 100-150 rounds accounting for redundancy and packet loss

This is fast enough to matter - by mid-game, dragons have collective knowledge far exceeding individual exploration.

---

## Part 3: Message Type Architecture

### The Challenge

Multiple message types must coexist:
- Status (existing, critical for rescue chains)
- Rescue role (existing, for child role assignment)
- Child training (new)
- Map chunks (new)
- Enemy intel (future)
- Food broadcasts (future)

All must share the 64-bit message format and be distinguishable by receivers.

### Solution: Tag-Based Message Types

Use different tag values XOR'd with the checksum to identify message types:

| Type | Purpose | Tag Pattern |
|------|---------|-------------|
| STATUS | Position, length, flags | 0x0000 |
| RESCUE | Child role assignment | 0x3C00 |
| TRAIN | Child education | 0x2D00 |
| MAP | Terrain chunk | 0x1E00 |
| ENEMY | Enemy position | 0x3300 |
| FOOD | Food broadcast | 0x0F00 |

Receiver tries each tag pattern against the checksum. Valid match indicates message type.

### Graceful Degradation

If a receiver doesn't understand a message type (old bot version, corrupted message), it simply ignores it. Status messages remain the universal fallback that all versions understand.

---

## Part 4: Combined System Benefits

### Expected Impact

| Metric | Before | After |
|--------|--------|-------|
| Child's first food | Random search | Directed (known location) |
| Child early survival | ~70%? | ~90%? |
| Map knowledge at birth | 0% | ~5% (inherited terrain) |
| Team map coverage by round 150 | Individual only | ~80% shared |
| Portal discovery efficiency | O(n) per dragon | O(1) via sharing |

### Risk Assessment

| Component | Risk | Mitigation |
|-----------|------|------------|
| Reduced status redundancy | LOW | N+S always sends status |
| Training message missed | MEDIUM | Turn N+1 backup |
| Map sync conflicts | LOW | Only update unknown→known |
| Checksum collisions | VERY LOW | 8-14 bits still robust |

---

## Part 5: Open Questions

1. **Sonar timing on split turn** - Does child receive parent's sonar on birth turn or Turn 2?

2. **Split point flexibility** - Does the game allow choosing where to split, or is it always midpoint?

3. **Enemy sonar interception** - Can we decode enemy sonar to learn their positions/intentions?

4. **Map message validation** - How do we handle potentially corrupted or malicious map data?

5. **Bandwidth optimization** - Could we compress map chunks further, or send partial updates?

6. **Child training content** - Is food + danger + portal the right priority, or should we include something else?

---

## Part 6: Future Directions

### Pair Hunting Coordination
Once map sharing works, dragons could coordinate hunting:
- Share enemy positions via ENEMY messages
- Converge on isolated enemies
- Trap enemies between allies using shared map knowledge

### Predictive Food Spawning
Track food spawn patterns across the team:
- Share food sighting locations and times
- Build probability model of spawn points
- Direct hungry dragons toward likely spawns

### Collective Danger Mapping
Share death locations and circumstances:
- "Dragon died at (x,y) on round R"
- Build heat map of dangerous areas
- Route planning avoids high-death zones

### Portal Network Optimization
Once all portal connections are known team-wide:
- Calculate shortest paths using portals
- Share optimal routes for food collection
- Coordinate portal usage to avoid collisions

---

## Part 7: Isolation Detection

### The Problem

Some maps may have partitioned regions where teams cannot reach each other. In such cases, dragons waste resources on combat-oriented behaviors (danger avoidance, defensive positioning) when no enemy threat exists.

Additionally, if all enemies have been eliminated or are unreachable, continuing to play defensively is suboptimal.

### Proposed Heuristic

Track how long since any enemy was last observed:

```
IF rounds_since_enemy_seen > ISOLATION_THRESHOLD:
    → Enter ISOLATED mode
    
IF enemy spotted again:
    → Immediately exit ISOLATED mode
    → Reset counter
```

**Suggested threshold**: 60 rounds (unverified whether this aligns with any official rule, but serves as a reasonable heuristic for "we're probably alone").

### Evidence Sources for Isolation

Multiple signals can contribute to isolation confidence:

| Signal | Weight | Notes |
|--------|--------|-------|
| No enemy in vision | Primary | Direct observation |
| No enemy sonar received | Supporting | They're not broadcasting nearby |
| Round > 100 | Context | Gave time to encounter |
| Map appears partitioned | Supporting | No paths to other region |

### Behavior Changes in Isolated Mode

| Aspect | Normal Mode | Isolated Mode |
|--------|-------------|---------------|
| Danger penalty | Full (130/36/6) | Heavily reduced or zero |
| Portal risk tolerance | Conservative | Aggressive exploration |
| Split threshold | Standard | Higher (grow longer, fewer dragons) |
| Exploration priority | Balanced | Maximum map coverage |
| Sprint usage | Conservative | Liberal (speed matters for food) |
| Corridor entry | Cautious | Enter freely |

### What We Still Avoid in Isolated Mode

Even without enemies, these remain dangerous:
- Self-collision (still fatal)
- Walls (still impassable)
- Getting trapped in dead ends (can't collect more food)
- Reckless portal entry without escape route

### Exiting Isolated Mode

If an enemy is spotted after entering isolated mode:
1. Immediately exit isolated mode
2. Restore full danger penalties
3. Re-evaluate current path for safety
4. Don't panic-split (assess the threat first)

### Strategic Implications

In isolated mode, the win condition becomes pure length comparison at round 500:
- Maximize food collection efficiency
- Grow the longest possible dragon
- Explore aggressively to find all food sources
- No need for combat-oriented positioning

### Open Questions

1. Is there an official game rule about map partitions? (Someone mentioned 60 rounds but unverified in documentation)
2. Should isolated mode affect splitting strategy? (Fewer, longer dragons vs. many short ones)
3. How quickly should we transition behaviors when entering/exiting isolated mode?

---

*This document will be updated as the design evolves.*
