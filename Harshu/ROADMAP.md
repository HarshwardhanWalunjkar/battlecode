# Development Roadmap: V6 → V7+

Based on research from competitive snake AI implementations (snek-two, battlesnake repos) and our V6 debugging results.

## Current State (V6)
- **Win rate**: 62.5% native vs V4.1
- **Strengths**: Corridor escape, room-aware rescue, terrain assessment
- **Weaknesses**: Trophy/Devil losses (food access), late-game concentration
- **Infrastructure**: Phase/role/rally systems exist but behavioral mods disabled

---

## Phase 1: Voronoi Territory Scoring (V6.1)
**Priority: HIGH | Complexity: MEDIUM | Risk: LOW**

### Problem
Current `space()` counts reachable tiles but ignores enemies. A dragon might have 50 reachable tiles, but if an enemy reaches 40 of them first, effective territory is only 10.

### Implementation
```cpp
std::pair<int,int> territory(State const& s) const {
    // BFS from our head and all enemy heads simultaneously
    // Count tiles we reach before any enemy
    std::vector<int> owner(n, -1);  // -1=unclaimed, 0=us, 1+=enemy
    std::vector<int> dist(n, INF);
    std::queue<std::pair<int,int>> q;  // (pos, owner_id)
    
    // Start from our head
    q.push({s.body[0], 0}); dist[s.body[0]] = 0; owner[s.body[0]] = 0;
    
    // Start from enemy heads
    for (int i = 0; i < (int)enemy_heads.size(); i++) {
        int eh = enemy_heads[i];
        q.push({eh, i+1}); dist[eh] = 0; owner[eh] = i+1;
    }
    
    // BFS expanding all fronts equally
    while (!q.empty()) {
        auto [p, o] = q.front(); q.pop();
        for (int v : next[p]) {
            if (v < 0 || s.occupied[v] || external_block(v)) continue;
            if (dist[v] == INF) {
                dist[v] = dist[p] + 1;
                owner[v] = o;
                q.push({v, o});
            }
        }
    }
    
    int ours = 0, contested = 0;
    for (int p = 0; p < n; p++) {
        if (owner[p] == 0) ours++;
        else if (owner[p] > 0 && dist[p] == dist[enemy_heads[owner[p]-1]]) contested++;
    }
    return {ours, contested};
}
```

### Integration
- Replace or augment `space()` calls in `root_score()` with territory
- Use territory ratio (ours vs total claimed) as a scoring factor
- Test on Trophy/Devil maps where food access is the issue

### Test Criteria
- Should improve Trophy/Devil win rate
- Should not regress on Queen/Dilemma

---

## Phase 2: Adaptive Food Urgency (V6.2)
**Priority: HIGH | Complexity: LOW | Risk: LOW**

### Problem
Fixed 12-point food reward regardless of situation. Short dragons should value food more; long growers should value it less.

### Implementation
```cpp
double food_urgency() const {
    // Short dragons need food more
    // Long dragons (growers) need it less
    // Near starvation (exploration timeout) need it desperately
    
    double base = 1.0;
    
    // Length factor: shorter = higher urgency
    if (length <= 4) base *= 1.5;
    else if (length <= 6) base *= 1.2;
    else if (length >= 10) base *= 0.8;
    
    // Stagnation factor: no food for a while = higher urgency
    int foodless = round - last_food;
    if (foodless >= 15) base *= 1.5;
    else if (foodless >= 10) base *= 1.2;
    
    // Population factor: few allies = preserve self
    if (count <= 3) base *= 1.3;
    
    return base;
}

// In step():
if(food) { after.reward += policy::food_reward * food_share[dest] * food_urgency(); }
```

### Test Criteria
- Short dragons should chase food more aggressively
- Growers should be more conservative
- Overall food collection should increase on low-food maps

---

## Phase 3: Flood Fill Space Evaluation (V6.3)
**Priority: MEDIUM | Complexity: LOW | Risk: LOW**

### Problem
Current `space()` uses BFS with a cap. Flood fill can identify if a region is completely enclosed (no escape).

### Implementation
```cpp
struct FloodResult {
    int area;
    int exits;      // edges to unexplored terrain
    bool enclosed;  // no exits at all
    int choke;      // narrowest point to exit
};

FloodResult flood_fill(State const& s, int cap = 200) const {
    Bits seen = s.occupied;
    std::vector<int> q{s.body[0]};
    seen.set(s.body[0]);
    int exits = 0, area = 0;
    
    while (!q.empty() && area < cap) {
        int p = q.back(); q.pop_back();
        area++;
        for (int d = 0; d < 4; d++) {
            if (cells[p].kind[d] < 0) { exits++; continue; }
            int v = next[p][d];
            if (v < 0 || seen[v]) continue;
            if (external_block(v)) continue;
            seen.set(v); q.push_back(v);
        }
    }
    
    return {area, exits, exits == 0, /* choke calculation */ 0};
}
```

### Integration
- Use `enclosed` flag to heavily penalize trapped routes
- Use `exits` count for exploration decisions

---

## Phase 4: Minimax Lookahead for Critical Situations (V7.0)
**Priority: MEDIUM | Complexity: HIGH | Risk: MEDIUM**

### Problem
Current lookahead assumes enemies stay still. In contested situations, this leads to suboptimal moves.

### Implementation
Only use minimax in specific situations to avoid computational cost:
- When enemy head is within 3 tiles
- When only 2-3 viable moves exist
- In endgame (round 450+)

```cpp
double minimax(State const& s, int depth, bool maximizing, double alpha, double beta) const {
    if (depth == 0 || expired(0.030)) return evaluate(s);
    
    if (maximizing) {
        double best = -INF;
        for (int d = 0; d < 4; d++) {
            State t; int hit;
            if (!step(s, d, false, t, hit, false)) continue;
            double val = minimax(t, depth-1, false, alpha, beta);
            best = std::max(best, val);
            alpha = std::max(alpha, val);
            if (beta <= alpha) break;
        }
        return best;
    } else {
        // Simulate best enemy move
        double best = INF;
        for (int eh : enemy_heads) {
            // ... enemy move simulation
        }
        return best;
    }
}
```

### Caution
- Very expensive computationally
- Only enable when clearly beneficial
- Must not exceed time budget

---

## Phase 5: Monte Carlo Tree Search (V7.1)
**Priority: LOW | Complexity: VERY HIGH | Risk: HIGH**

### Problem
Minimax is deterministic and expensive. MCTS can explore more scenarios probabilistically.

### When to Consider
- Only after phases 1-3 are stable
- Only if minimax proves insufficient
- Requires significant testing

### Note
MCTS is powerful but complex. The 100M instruction budget and 0.1s time limit may make this impractical. Consider only if other approaches plateau.

---

## Phase 6: Portal-Aware Pathfinding (V6.4)
**Priority: MEDIUM | Complexity: MEDIUM | Risk: LOW**

### Problem
Current distance calculations sometimes ignore portal shortcuts.

### Implementation
```cpp
int portal_distance(int from, int to) const {
    // Modified BFS that properly weights portal traversal
    // Unknown portal exits get estimated distance
    std::vector<int> dist(n, INF);
    std::priority_queue<std::pair<int,int>, std::vector<std::pair<int,int>>, std::greater<>> pq;
    
    dist[from] = 0;
    pq.push({0, from});
    
    while (!pq.empty()) {
        auto [d, p] = pq.top(); pq.pop();
        if (p == to) return d;
        if (d > dist[p]) continue;
        
        for (int dir = 0; dir < 4; dir++) {
            int v = next[p][dir];
            int cost = 1;
            
            // Portal with unknown exit: estimate
            if (cells[p].kind[dir] == 2 && v < 0) {
                cost = 5;  // Unknown portal penalty
                v = p;     // Stay in place for estimation
            }
            
            if (v >= 0 && dist[p] + cost < dist[v]) {
                dist[v] = dist[p] + cost;
                pq.push({dist[v], v});
            }
        }
    }
    return INF;
}
```

---

## Phase 7: Improved Late-Game Coordination (V7.2)
**Priority: MEDIUM | Complexity: HIGH | Risk: MEDIUM**

### Problem
V6 convergence attraction hurt performance. Need a different approach.

### Alternative Approaches to Test

#### A. Scheduled Donations
Instead of convergence attraction, use turn-based donations:
- After round 450, if `id % 10 == (round - 450) % 10`, consider donating
- This spreads donations over time without movement disruption

#### B. Local Feeding Only
Only donate if recipient is already adjacent (no movement toward rally)
- Keeps normal food collection behavior
- Opportunistic feeding when naturally adjacent

#### C. Designated Feeders by ID
- IDs 0-9: Always feeders after round 400
- IDs 10+: Potential receivers
- No convergence needed, just wait for natural encounters

---

## Implementation Order

1. **V6.1**: Voronoi territory (highest impact, addresses food access)
2. **V6.2**: Adaptive food urgency (low risk, easy to test)
3. **V6.3**: Flood fill improvements (low risk, builds on existing)
4. **V6.4**: Portal-aware pathfinding (medium effort, medium reward)
5. **V7.0**: Minimax for critical situations (high effort, test carefully)
6. **V7.2**: Late-game coordination rework (requires new approach)
7. **V7.1**: MCTS (only if needed, very high complexity)

---

## Testing Protocol

For each phase:
1. Run 8-game native comparison vs previous version (seed 509)
2. Run 8-game sandbox comparison
3. Check specific maps: Trophy/Devil (food), Dilemma (corridor), Queen (farming)
4. Only merge if win rate >= previous version
5. Document fingerprint and results

---

## Key Metrics to Track

| Metric | Current V6 | Target |
|--------|------------|--------|
| Native win rate vs V4.1 | 62.5% | 70%+ |
| Sandbox win rate vs V4.1 | 37.5% | 50%+ |
| Dilemma win rate | 100% | Maintain |
| Trophy win rate | 0% | 50%+ |
| Devil win rate | untested | 50%+ |
