# Implementation plan — phased approach

## Phase 1: Implicit roles (no protocol change)

### What changes

Add role inference and role-based scoring adjustments. No message format
changes. Fully backwards compatible — same messages, same sends.

### New constants in config.hpp

```cpp
constexpr int attacker_max_length = 3;
constexpr int grower_min_length = 6;
constexpr int role_min_count = 4;        // need this many dragons before attackers activate
constexpr double attacker_danger_one = 60.0;   // vs 130 — less afraid of nearby enemies
constexpr double attacker_danger_two = 12.0;   // vs 36
constexpr double attacker_danger_three = 0.0;  // ignore distant threats
constexpr double attacker_enemy_attraction = 8.0;  // positive pull toward visible enemy heads
constexpr double grower_danger_bonus = 30.0;   // extra penalty on top of base danger
```

### New members in Brain

```cpp
enum Role { ATTACKER, GROWER, FLEXIBLE };
Role role = FLEXIBLE;

void infer_role() {
    if (length <= policy::attacker_max_length && count >= policy::role_min_count)
        role = ATTACKER;
    else if (length >= policy::grower_min_length)
        role = GROWER;
    else
        role = FLEXIBLE;
}
```

Call `infer_role()` at the end of `observe()`.

### Changes to root_score

```cpp
double root_score(Candidate& c) const {
    // ... existing code ...

    // Role-based danger adjustment
    double d = danger(c.threat);
    if (role == ATTACKER) {
        // Replace standard danger with attacker values
        d = c.threat == 1 ? policy::attacker_danger_one
          : c.threat == 2 ? policy::attacker_danger_two
          : c.threat == 3 ? policy::attacker_danger_three : 0;
        // Attraction toward visible enemy heads
        for (int eh : enemy_heads) {
            int dist = distance(c.state.body[0], eh);
            if (dist <= 5) score += policy::attacker_enemy_attraction / (1 + dist);
        }
    } else if (role == GROWER) {
        d += policy::grower_danger_bonus * (c.threat <= 2 ? 1.0 : 0.3);
    }
    score -= d;

    // ... rest of existing code ...
}
```

### Changes to choose() — attacker head trades

```cpp
// In the attacks loop, replace the current conditions:
for (auto const& a : attacks) {
    bool no_moves = all.empty();
    bool can_survive = count > 1;
    bool draw_chance = count == 1 && enemy_heads.size() <= 1;

    if (no_moves && (can_survive || draw_chance))
        return {a.path, 0, s, false, true};

    // Attackers are more willing to trade
    if (role == ATTACKER && can_survive && (no_moves || trapped))
        return {a.path, 0, s, false, true};

    // Flexible/grower: existing strict condition
    if (!no_moves && trapped && can_survive && count >= 3 && length <= 4)
        return {a.path, 0, s, false, true};
}
```

### Changes to choose() — attacker splitting

Attackers should NOT split (they stay small and expendable):

```cpp
if (role != ATTACKER && safe_split(s, trapped)) {
    return split_decision(s, 2);
}
```

### Changes to sheltered filter

Attackers don't use the sheltered filter — they want to be near enemies:

```cpp
if (role != ATTACKER) {
    bool sheltered = std::any_of(...);
    if (sheltered) all.erase(...);
}
```

### Testing phase 1

Run against baselines with different role threshold configs:
```sh
# Test attacker_max_length = 3 (aggressive small dragons)
# Test attacker_max_length = 4 (more dragons become attackers)
# Test role_min_count = 3 vs 4 vs 6
```

Key metrics to watch:
- Do attackers actually kill enemies? (count enemy deaths)
- Do growers survive longer? (check longest dragon at round 500)
- Does the team win more games overall?

Expected outcome: attackers distract and eliminate enemies while growers score.

---

## Phase 2: THREAT messages

### Prerequisites
Phase 1 working and showing improvement.

### What changes

Add a second message type (THREAT) using the protocol defined in
message-protocol-v3.md. Attackers converge on reported enemy positions.

### New data structures

```cpp
struct Threat {
    int pos, size, heading, round;
    bool approaching;
};

std::vector<Threat> threats;
```

### New methods

```cpp
std::uint64_t threat_packet(int enemy_pos, int enemy_size, int enemy_dir, bool approaching) const {
    int ex = enemy_pos % w, ey = enemy_pos / w;
    int rx = head % w, ry = head / w;
    std::uint64_t v = std::uint64_t(ex)
        | (std::uint64_t(ey) << 6)
        | (std::uint64_t(std::min(enemy_size, 63)) << 12)
        | (std::uint64_t(enemy_dir) << 18)
        | (std::uint64_t(approaching ? 1 : 0) << 20)
        | (std::uint64_t(rx) << 21)
        | (std::uint64_t(ry) << 27)
        | (std::uint64_t(round) << 33)
        | (std::uint64_t(team) << 42)
        | (std::uint64_t(id & 127) << 43);
    return v | (std::uint64_t(checksum(v) & 8191) << 50) | (UINT64_C(1) << 63);
}

void decode_threats(const std::vector<std::uint64_t>& inbox) {
    threats.erase(std::remove_if(threats.begin(), threats.end(),
        [&](auto& t) { return round - t.round > 3; }), threats.end());

    for (auto m : inbox) {
        if (!(m >> 63)) continue;  // STATUS, not THREAT
        auto v = m & ((UINT64_C(1) << 50) - 1);
        if ((checksum(v) & 8191) != ((m >> 50) & 8191)) continue;
        if (int((v >> 42) & 1) != team) continue;

        int ex = v & 63, ey = (v >> 6) & 63;
        int esize = (v >> 12) & 63, edir = (v >> 18) & 3;
        bool approaching = (v >> 20) & 1;
        int r = (v >> 33) & 511, rid = (v >> 43) & 127;

        if (ex >= w || ey >= h || r > round || round - r > 2) continue;
        if ((rid & 127) == (id & 127)) continue;  // from self

        int epos = ey * w + ex;
        // If we can see this tile and no enemy is there, reject stale report
        if (cells[epos].seen == round && (!cells[epos].head || cells[epos].team == team))
            continue;

        Threat t{epos, esize, edir, r, approaching};
        // Deduplicate: update if same position, keep freshest
        auto it = std::find_if(threats.begin(), threats.end(),
            [&](auto& existing) { return existing.pos == epos; });
        if (it == threats.end()) threats.push_back(t);
        else if (r >= it->round) *it = t;
    }
}
```

### Changes to observe()

```cpp
void observe(...) {
    // ... existing code ...
    messages(ct.get_sonar_messages());       // existing STATUS decode
    decode_threats(ct.get_sonar_messages());  // new THREAT decode
    // ...
}
```

### Changes to root_score() for attackers

```cpp
if (role == ATTACKER) {
    // Attraction toward reported threats (not just visible enemies)
    for (auto const& t : threats) {
        int dist = distance(c.state.body[0], t.pos);
        double freshness = 1.0 / (1 + 0.3 * (round - t.round));
        double value = policy::attacker_enemy_attraction * t.size * freshness / (1 + 0.5 * dist);
        score += value;
    }
}
```

### Changes to commit() — sending THREAT messages

```cpp
void commit(Decision const& d, Controller& ct) {
    // ... existing action code ...

    if (!d.dying && !d.after.body.empty()) {
        predicted = d.after.body;
        if (d.after.eaten.any()) last_food = round;

        // Send STATUS
        if (d.exact && (count > 1 || d.split)) {
            auto msg = packet(predicted[0], d.after.length);
            int first = (round + id) % 4;
            ct.send_sonar(Direction(LETTERS[first]), msg);
            ct.send_sonar(Direction(LETTERS[first ^ 2]), msg);
        }

        // Send THREAT for each visible enemy head
        if (d.exact && !enemy_heads.empty()) {
            auto const& eh = enemy_heads[0];  // primary target
            int esize = enemy_size.count(cells[eh].id) ? enemy_size[cells[eh].id] : 1;
            int edir = cells[eh].facing;
            bool approaching = next[eh][edir] >= 0 &&
                distance(next[eh][edir], predicted[0]) < distance(eh, predicted[0]);
            auto tmsg = threat_packet(eh, esize, edir, approaching);

            // Send toward directions where allies might be
            int threat_dir = (first + 1) % 4;
            ct.send_sonar(Direction(LETTERS[threat_dir]), tmsg);
            ct.send_sonar(Direction(LETTERS[threat_dir ^ 2]), tmsg);
        }
    } else predicted.clear();
}
```

### Testing phase 2

Run against hunter baseline on portals and big_empty (large maps where
beyond-vision coordination matters most).

Key metrics:
- Do attackers converge on reported enemy positions?
- How many THREAT messages are successfully received? (add debug counter)
- Does convergence actually result in more enemy kills?

---

## Phase 3: Echo probing (independent of phase 2)

### What changes

GROWER dragons send sonar in a single direction per turn to get attributable
echo data. They build a coarse "enemy density by direction" map.

### New data structures

```cpp
int echo_enemy_count[4] = {};   // enemy + enemy_head by direction
int echo_kelp_count[4] = {};    // kelp by direction
int echo_ally_count[4] = {};    // ally + ally_head by direction
int echo_freshness[4] = {};     // round when each direction was last probed
int last_probe_dir = -1;
bool probing_this_turn = false;
```

### Changes to commit()

Grower dragons send ALL messages in the same direction on probe turns:

```cpp
if (role == GROWER && d.exact) {
    int probe = (round + id) % 4;
    auto msg = packet(predicted[0], d.after.length);
    // All 4 slots in probe direction for clean echo
    ct.send_sonar(Direction(LETTERS[probe]), msg);
    // (don't send in other directions this turn)
    last_probe_dir = probe;
    probing_this_turn = true;
}
```

### Changes to observe()

```cpp
auto echoes = ct.get_sonar_echoes();
if (last_probe_dir >= 0 && !probing_this_turn) {
    // These echoes are from last turn's probe
    echo_enemy_count[last_probe_dir] = echoes.enemy + echoes.enemy_head;
    echo_kelp_count[last_probe_dir] = echoes.kelp;
    echo_ally_count[last_probe_dir] = echoes.ally + echoes.ally_head;
    echo_freshness[last_probe_dir] = round - 1;
    last_probe_dir = -1;
}
probing_this_turn = false;
```

### Changes to root_score() for growers

```cpp
if (role == GROWER) {
    // Prefer directions with fewer enemies and less kelp
    int move_dir = c.state.heading;  // approximate direction of this route
    if (echo_freshness[move_dir] > round - 8) {
        score -= 2.0 * echo_enemy_count[move_dir];  // avoid enemy-heavy dirs
        score -= 0.5 * echo_kelp_count[move_dir];   // avoid kelp-blocked dirs
    }
}
```

### Tradeoff

Grower probing means allies in non-probe directions don't receive STATUS
messages that turn. Acceptable because:
- Growers are few and large — they don't need to send every turn
- Attackers still broadcast fully
- The directional enemy info helps growers survive longer

---

## Phase 4: Adaptive recruitment

### Trigger: swarm depletion

```cpp
int prev_count = count;  // store at end of observe()

// In choose(), before normal split:
bool swarm_depleted = count < prev_count && count < 10 && round < 400;
if (swarm_depleted && role == GROWER && length >= 4 && safe_split(s, true)) {
    return split_decision(s, 2);  // emergency: produce a new attacker
}
```

### Trigger: winning position

If we have more dragons than visible enemies and our longest dragon is longer,
shift attackers to be less aggressive (protect the lead):

```cpp
bool winning = count > int(enemy_heads.size()) * 2 && length >= 8;
if (winning && role == ATTACKER) {
    // Reduce enemy attraction, increase food attraction
    // Effectively transition some attackers to flexible behavior
}
```

---

## File change summary by phase

### Phase 1 (~30 lines)
- `config.hpp`: add 7 role constants
- `strategy.hpp`: add Role enum, infer_role(), modify root_score(), modify
  choose() attack logic and sheltered filter

### Phase 2 (~80 lines)
- `strategy.hpp`: add Threat struct, threat_packet(), decode_threats(),
  modify observe() and commit(), add threat_attraction to root_score()

### Phase 3 (~40 lines)
- `strategy.hpp`: add echo tracking arrays, modify commit() for grower
  probing, modify observe() for echo collection, add echo-based scoring

### Phase 4 (~25 lines)
- `strategy.hpp`: add adaptive split triggers and winning-position detection

### Total: ~175 lines across all 4 phases

Each phase is independently deployable and testable. Phase 1 is the highest
value per line of code. Phase 2 is the most architecturally interesting.
Phases 3-4 are refinements.
