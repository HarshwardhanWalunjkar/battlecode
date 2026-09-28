# Open questions and design tradeoffs

## Q1: What are the right role thresholds?

**attacker_max_length = 3** means only the smallest dragons attack. Born at
length 2, they become attackers immediately. After eating 1 pearl (length 3),
still attacker. After 2 pearls (length 4), they transition to FLEXIBLE and may
soon split back down.

Alternative: **attacker_max_length = 4**. More dragons become attackers, but
they're also big enough to split (length 4 is our split threshold). An attacker
that eats a pearl and reaches 4 would split → producing a length 2 child
(auto-attacker) and a length 2 parent (also auto-attacker). This creates a
self-reinforcing split loop that could drain the team of growers.

**Recommendation:** Start with 3. If testing shows we need more attackers,
increase to 4 but add a rule: attackers don't split (already in phase 1 plan).

## Q2: How many attackers is too many?

If all 48 dragons are length 2-3, we have a massive swarm but zero endgame
scoring power. The game scores on **longest living dragon**, not total count.

**Control mechanism:** The `grower_min_length = 6` threshold means any dragon
that grows past 5 automatically becomes a grower. Growers split at length 4
(producing new attackers) so the system naturally balances. But if food is
scarce, all dragons stay small → all become attackers → nobody grows → we lose
on score at round 500.

**Possible safety valve:** If round > 300 and our longest dragon < 5, reduce
attacker aggression globally. Transition to pure growth mode for survival.

## Q3: Should attackers ignore food entirely?

If attackers eat food, they grow → leave attacker role → become growers/flexible.
This "promotes" them out of the swarm naturally.

If attackers avoid food, they stay small permanently but waste pearl opportunities
that could help the team score.

**Recommendation:** Attackers should eat food normally but with reduced
attraction (lower food_reward for attackers). They'll naturally eat what's
convenient, grow, and be replaced by new split children. This creates a healthy
population cycle: born small → attack → eat → grow → split → born small.

## Q4: Can enemies exploit our THREAT messages?

Enemies receive our messages but can't decode them without knowing our checksum
function and team bit. However:

- The checksum function is in our public source code (published replays
  expose our actions, not our code — but a reverse engineer could study
  message patterns)
- If an enemy decodes THREAT messages, they know which of their dragons
  we've spotted. This is information they already have (they know where
  their own dragons are)
- An enemy CANNOT forge our messages convincingly without matching our
  checksum and team bit

**Real risk:** Negligible. The information in THREAT messages (enemy position)
is already known to the enemy. Our checksum prevents injection.

## Q5: What happens when multiple attackers converge on the same target?

Three attackers approaching the same enemy head from different sides:
- Best case: they surround and trap the enemy, forcing a collision
- Worst case: they collide with each other trying to reach the same square

**Mitigation options:**
1. Cap convergence: only the 2-3 nearest attackers respond to a given threat
2. Direction-based: each attacker approaches from its nearest side, not the
   same direction
3. The existing collision check (own bodies/ally heads block routes) already
   prevents ally head-to-head crashes

Option 3 is already implemented — our search doesn't allow moving into an
ally head. The risk is more about crowding (many small dragons in a tight
space with few exits) than direct collision.

## Q6: Echo probing — is the single-direction cost worth it?

Sending all messages in one direction means allies in other directions don't
hear from us that turn. For a grower sending STATUS every 4th turn in each
direction, allies get 1/4 of the updates.

**But:** Growers are the minority of dragons. Attackers still broadcast fully.
And the coarse enemy density map helps growers avoid dangerous areas, which
means the longest dragon (our scoring asset) survives longer.

**Alternative:** Probe only every 4th turn instead of every turn. Send STATUS
normally 3 turns, probe 1 turn. This gives a stale but still useful density
map while maintaining most STATUS coverage.

## Q7: How do we test convergence actually works?

Running against baselines won't show swarm convergence well because our
baselines don't have 30-64 dragons. We need a "swarm" baseline opponent.

**Options:**
1. Modify the hunter baseline to split aggressively (split_length=4, max_team=48)
   and use it as the opponent
2. Study replay data: add a debug LOG that prints [THREAT_RECEIVED pos=X,Y]
   and [CONVERGING_ON pos=X,Y] — these show in replays but not in score
3. Count: in post-game analysis, track how many enemy deaths happened near
   a reported threat location (within 3 tiles and 5 rounds of the report)

**Recommendation:** Option 1 first (create a swarm-hunter baseline), then
option 3 for measuring. Option 2 costs CPU budget for LOG output.

## Q8: Does the opposite-facing sonar emission from tail matter?

From MASTER.md: "Opposite-facing sonar starts at the tail and travels away
from its body."

This means if we face EAST and send sonar WEST (opposite), the ray starts
at our TAIL (the westernmost segment of our body) and goes WEST. For a long
dragon, this means the ray originates from a very different position than
our head.

**Impact on THREAT routing:** When an attacker sends a THREAT westward,
it actually originates from the attacker's tail, not head. For a length-2
dragon, the tail is 1 tile behind the head — minimal offset. For longer
dragons, the offset grows.

**Impact on echo probing:** The probe ray starts from the tail in the
opposite direction. A grower facing EAST that probes WEST gets data about
what's west of its TAIL, not its head. For growers (length >= 6), this is
a 5+ tile offset.

**Recommendation:** For phase 1, ignore this (attackers are length 2-3,
offset is 1-2 tiles). For phase 3 echo probing, account for the tail
position when interpreting echoes. Store `tail_pos` and use it as the
echo origin when the probe direction was opposite to heading.

## Q9: Budget impact of the new logic

Current peak: 22M / 100M points. The new code adds:
- Role inference: trivial (one comparison)
- Threat decoding: O(messages) per turn — same as STATUS decoding
- Threat attraction in root_score: O(threats * candidates) — threats are
  at most ~5, candidates are 14. Adds ~70 iterations.
- Echo tracking: trivial per turn

**Estimated additional cost:** Under 500K points. Nowhere near the 100M limit.

## Q10: What if the swarm strategy is strictly worse against non-swarm opponents?

Our baselines (greedy, hunter) play with few dragons. Diverting our small
dragons to attack mode means fewer of them collect food. Against a non-swarm
opponent, we're weakening our food collection for attack capability we don't
need.

**The role_min_count threshold handles this.** If count < 4, nobody becomes
an attacker — everyone plays FLEXIBLE (current v2 behavior). We only activate
swarm roles when we have enough dragons to spare. And the threshold is tunable.

Against non-swarm opponents, our dragons grow to length 6+ quickly, become
growers, and play the standard cautious game. Attackers only appear when
frequent splitting creates many small dragons — which happens when food is
plentiful and the population is building.
