# Message protocol v3 — design specification

## Current protocol (v2 messages)

All 64 bits encode a single STATUS message:

```
 63       50 49  48     33 32    24 23    12 11  6 5   0
┌─────────┬──┬────────┬────────┬────────┬──────┬──────┐
│checksum │tm│ dragon │ round  │ length │  y   │  x   │
│ 14 bits │1 │ 16 bits│ 9 bits │ 12 bits│6 bits│6 bits│
└─────────┴──┴────────┴────────┴────────┴──────┴──────┘
```

**Problem:** No room for message type, enemy reports, or role signals.

## Proposed protocol (v3 messages)

### Key change: 1-bit message type

Steal 1 bit from the checksum (14 → 13 bits). The collision rate goes from
1/16384 (~0.006%) to 1/8192 (~0.012%) — still negligible given our other
validation (team bit, coordinate range, round freshness, ID check).

```
bit 63:     TYPE (0 = STATUS, 1 = THREAT)
bits 50-62: checksum (13 bits)
```

### TYPE 0: STATUS (ally position report)

Identical to current format minus 1 checksum bit.

```
 63 62       50 49  48     33 32    24 23    12 11  6 5   0
┌──┬─────────┬──┬────────┬────────┬────────┬──────┬──────┐
│ 0│checksum │tm│ dragon │ round  │ length │  y   │  x   │
│1 │ 13 bits │1 │ 16 bits│ 9 bits │ 12 bits│6 bits│6 bits│
└──┴─────────┴──┴────────┴────────┴────────┴──────┴──────┘
```

Validation: type==0, checksum match, team match, x<w, y<h, length>=2,
round within 2 of current, ID != self.

### TYPE 1: THREAT (enemy position report)

A dragon that sees an enemy head broadcasts its location.

```
 63 62       50 49     42 41  40     32 31    26 25    20 19  18 17    12 11  6 5   0
┌──┬─────────┬────────┬──┬────────┬──────┬──────┬────┬──────┬──────┬──────┐
│ 1│checksum │rep. ID │tm│ round  │rep.y │rep.x │hdir│ size │ en.y │ en.x │
│1 │ 13 bits │ 7 bits │1 │ 9 bits │6 bits│6 bits│ 2  │6 bits│6 bits│6 bits│
└──┴─────────┴────────┴──┴────────┴──────┴──────┴────┴──────┴──────┴──────┘
```

Fields:
- **en.x, en.y** (6+6 = 12): enemy head position
- **size** (6): visible enemy segments (capped at 63)
- **hdir** (2): enemy heading (N=0, E=1, S=2, W=3)
- **rep.x, rep.y** (6+6 = 12): reporter's position (for distance estimation)
- **round** (9): observation round
- **tm** (1): team bit
- **rep. ID** (7): reporter ID low bits (mod 128, enough to distinguish)
- **checksum** (13): validation hash
- **type** (1): always 1

Total: 12 + 6 + 2 + 12 + 9 + 1 + 7 + 13 + 1 = 63. Wait, that's 63. Let me
recount: 6+6+6+2+6+6+9+1+7+13+1 = 63. We have 1 spare bit.

Use the spare bit for **threat urgency**: 0 = enemy is nearby, 1 = enemy is
approaching (heading toward reporter). This helps prioritize.

Revised:

```
bit  0-5:   enemy x (6)
bit  6-11:  enemy y (6)
bit 12-17:  visible enemy size (6)
bit 18-19:  enemy heading (2)
bit 20:     approaching (1) — enemy heading points toward reporter
bit 21-26:  reporter x (6)
bit 27-32:  reporter y (6)
bit 33-41:  round (9)
bit 42:     team (1)
bit 43-49:  reporter ID mod 128 (7)
bit 50-62:  checksum (13)
bit 63:     1 (type = THREAT)
```

Total: 6+6+6+2+1+6+6+9+1+7+13+1 = 64. Perfect.

### Validation for THREAT messages

```
type == 1
checksum matches bits 0-49
team matches our team
en.x < w, en.y < h
rep.x < w, rep.y < h
round within 2 of current
reporter ID != self (mod 128)
```

### Encoding/decoding

```cpp
// Encode
std::uint64_t threat_packet(int ex, int ey, int esize, int edir,
                             bool approaching, int rx, int ry, int rid) const {
    std::uint64_t v = std::uint64_t(ex)
        | (std::uint64_t(ey) << 6)
        | (std::uint64_t(std::min(esize, 63)) << 12)
        | (std::uint64_t(edir) << 18)
        | (std::uint64_t(approaching ? 1 : 0) << 20)
        | (std::uint64_t(rx) << 21)
        | (std::uint64_t(ry) << 27)
        | (std::uint64_t(round) << 33)
        | (std::uint64_t(team) << 42)
        | (std::uint64_t(rid & 127) << 43);
    return v | (std::uint64_t(checksum(v) & 8191) << 50) | (UINT64_C(1) << 63);
}

// Decode
bool decode_threat(std::uint64_t m, int& ex, int& ey, int& esize, int& edir,
                   bool& approaching, int& rx, int& ry, int& r, int& rid) {
    if (!(m >> 63)) return false;  // not a THREAT
    auto v = m & ((UINT64_C(1) << 50) - 1);
    if ((checksum(v) & 8191) != ((m >> 50) & 8191)) return false;
    if (int((v >> 42) & 1) != team) return false;
    ex = v & 63; ey = (v >> 6) & 63;
    esize = (v >> 12) & 63; edir = (v >> 18) & 3;
    approaching = (v >> 20) & 1;
    rx = (v >> 21) & 63; ry = (v >> 27) & 63;
    r = (v >> 33) & 511; rid = (v >> 43) & 127;
    return ex < w && ey < h && rx < w && ry < h && r <= round && round - r <= 2;
}
```

## Sending strategy

### ATTACKER dragons (length <= 3)

Priority: broadcast threats to help convergence.

```
If enemy head visible:
    D1: THREAT message toward direction with most recent ally echo
    D2: THREAT message toward opposite direction
    D3: STATUS message (rotating direction)
    D4: STATUS message (opposite of D3)
Else:
    D1-D2: STATUS (rotating pair)
    D3-D4: STATUS (other pair) — maximize position coverage
```

### GROWER dragons (length >= 6)

Priority: share position for food deconfliction + probe for map info.

```
If enemy head visible:
    D1: THREAT message (toward likely ally direction)
    D2: STATUS (rotating)
    D3: STATUS (opposite)
    D4: probe direction (send sonar with no message, just for echo)
Else:
    D1: STATUS (rotating)
    D2: STATUS (opposite)
    D3-D4: probe (cycle through N/E/S/W over 4 turns)
```

Growers sacrifice 1-2 message slots for echo probing. Since they're not
attacking, they benefit more from mapping.

### FLEXIBLE dragons

Use current v2 sending behavior (STATUS in two rotating directions).

## Echo probing protocol

A grower that sends sonar in exactly ONE direction per turn gets an
attributable echo next turn.

```cpp
int probe_dir = (round + id) % 4;  // cycle N, E, S, W

// In commit(), after normal sends:
if (role == GROWER) {
    // Send a dummy STATUS in probe direction to get an echo
    ct.send_sonar(Direction(LETTERS[probe_dir]), packet(predicted[0], length));
}

// In observe(), read echoes:
if (role == GROWER && echoes_available) {
    echo_map[last_probe_dir] = ct.get_sonar_echoes();
    echo_round[last_probe_dir] = round - 1;  // echoes are from last turn
}
```

**Problem:** Echoes are aggregate across ALL sent directions, not just the
probe. If we also sent STATUS in two other directions, the echo mixes them.

**Solution:** On the probe turn, send ALL messages in the same direction as
the probe. Then the echo is fully attributable. Cost: allies only receive
from one direction that turn (reduced coverage, acceptable for growers).

Alternative: send only 1 message total on probe turns. Simpler, echo is
clean. STATUS coverage drops but growers don't need high-frequency updates.

## Data structures

```cpp
struct Threat {
    int pos, size, heading, round;
    bool approaching;
    int reporter_pos;
};

// In Brain class:
std::vector<Threat> threats;
int echo_enemy[4] = {};      // enemy body+head count by direction
int echo_kelp[4] = {};       // kelp count by direction
int echo_round[4] = {};      // when each direction was last probed
int last_probe_dir = -1;

enum Role { ATTACKER, GROWER, FLEXIBLE };
Role role = FLEXIBLE;

void infer_role() {
    if (length <= 3 && count >= 4) role = ATTACKER;
    else if (length >= 6) role = GROWER;
    else role = FLEXIBLE;
}
```

## Backwards compatibility

Children inherit Protocol 3 from their parent. The new message types use the
same checksum function and team bit. A v2 dragon receiving a THREAT message
would fail the v2 checksum (because it checks all 14 bits at position 50-63,
but the type bit at 63 corrupts this). It would silently reject the message
— safe, no crash.

This means we can deploy incrementally: new code sends THREAT messages, old
code ignores them. Mixed-version is safe.
