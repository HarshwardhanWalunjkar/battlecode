# V4 — portal safety, rescue splits and growth

Reviewed 28 September 2026. V4 is implemented and packaged, not submitted.
The complete behavior is in [Gamplan.md](../../Gamplan.md). The
[work tracker](../../current%20planned%20work.md) records the handoff state.
At the time of this implementation review, no v4 local matches or sandbox games
had been run: the user asked to finish implementation first. The subsequent
authorized [validation and v4.1 correction](VALIDATION.md) records ten completed
games, resource use and the narrow portal fix. The 44 original logic checks
were separate from matches; v4.1 has 47.

## Evidence examined

- Sixteen additional completed games were downloaded after the preceding audit:
  six v2 games and ten v3 games. The archive now contains 200 v2 games (78 wins)
  and ten version-attributed v3 games (five wins), plus the older v1 records.
- The ten-game Borgor series referenced through game 464372 was also downloaded,
  including Prisoners Dilemma 464370 and Slithery Fight 464373.
- The reference games were [Trauma 459157](https://game.battlecode.au/visualiser?match=459157)
  and [Autarky 464013](https://game.battlecode.au/visualiser?match=464013).
- Hampter's five-game series includes [Slithery Fight 450663](https://game.battlecode.au/visualiser?match=450663)
  and [Autarky 450664](https://game.battlecode.au/visualiser?match=450664).

`tools/audit_tactics.py` reconstructs both teams' bodies, portal transitions,
splits, deaths and collection of death pearls. Every successful destination is
checked against the official viewer's event. `tactics.json` and `own-analysis.json`
retain the detailed evidence locally alongside the raw replays. The newer API
responses omit submission identity, including for the named Borgor series; this
report does not invent its version. Older archived v3 games explicitly name 9700.

## Recurring portal bug: confirmed and fixed

| Sample | Initiated portal collisions | Own-body collisions | Other allied victims | Successful movements checked |
|---|---:|---:|---:|---:|
| Ten explicitly attributed v3 games, 454913–454922 | 157 | 105 | 35 | 185,520 |
| Ten newly named Borgor games, 464366–464375 | 175 | 119 | 42 | 198,780 |

Every counted collision had an exit outside the mover's turn-start vision.
Counts are collision attempts, not all individual head-death events; a head
collision can kill two dragons. Own-body and other-ally counts are separate.
There were also enemy victims. This is a repeated problem, not one visualizer clip.

The concrete defect was discarding the entire expected body after blind portal
travel. In the Portals game 454916 alone, the audit counted 82 portal collisions,
including repeated immediate self reversals. V4 preserves the previous body,
attaches the newly observed head, accounts for arrival food and forbids blind
neck reversals even for newborns. Split-off segments also remain recent allied
obstacles instead of being mistaken for departed own-body segments.

Additional protections are short-lived portal-busy messages, attributable
single-ray probes, recent allied-head checks, an eight-round blind-crossing
cooldown and removal of the old low-score bypass. Protected growers use an
unseen exit only as a last resort. Currently visible destinations still use the
ordinary exact collision checks. Sonar misses and stale information cannot
guarantee that a hidden exit is clear.

## Long snakes and the named opening failures

In game 450663, Hampter's length-25 dragon became a length-2 parent plus a
length-23 rear child at round zero. That child moved from the old tail. Hampter eventually finished with a longest
length of 37; Borgor finished with 7.
Our original parent repeatedly shed two-segment children while its head remained
trapped. The same pattern appears in v3 game 454920 and named game 464373.

V4 checks escape routes for different cuts, prioritizing the largest surviving
half. A focused check using the actual Slithery Fight opening body and visible
terrain selects the 23-segment rear child. Rescue children receive a protection
message so a saved length-five or length-six child need not immediately breed
again. The parent can be sacrificed when that is the way to save the long child.

In Prisoners Dilemma 464370, Borgor cut 11 into 2+9 and the child immediately cut
9 into 2+7 in the same round. Both had an empty first-step destination in the
full replay state. The old automatic opening rule did not require a real trap.
V4 removes that automatic trigger: incomplete knowledge alone is not a reason
to split a long starting body. An actually blocked partial body can still make
an explicitly uncertain rear rescue. Full replay knowledge is not given to the bot.

The engine forbids a one-segment half: the minimum on both sides is two.
See the [official split rules](https://game.battlecode.au/docs/splitting).
Children move later in that same round, while their parent has already used its
action. The checks distinguish those horizons, including a legal parent wait on
the last round. [Official execution order](https://game.battlecode.au/docs/execution-order).

## Expansion and maze exploration

The accepted compromise is early reproduction by length-four-to-six dragons,
with length seven and above protected immediately. From round 200, length six
and selected smaller growers are protected too; ordinary births stop at 225.
All 64 slots remain available, but local congestion, food supply and the ability
of both halves to continue constrain births. Emergency rescues remain available.

In Trauma 459157, round snapshots show Cutlery and calc heads on 581 and 626
distinct squares. Borgor appears on 297 in the later Trauma 464374. These are
whole-map snapshot coverage counts, which omit intermediate sprint steps, with
different opponents and seeds; they do not prove a specific maze algorithm.
They support investigating broader exploration; the replays do not reveal the
opponents' source code. V4 gives about a third of early small workers a shorter
foodless wait, uses persistent reachable frontier targets, and retains visible
destination checks. Long saved dragons do not become reckless scouts.

## Food control and expected return from a pocket

In Autarky 464013, the winning Settlers of Battlecode really did split its
length-14 starter into 8+6, then split again. That establishes that a balanced
opening can work there; it does not establish that halving every long dragon is
best. V4 includes a balanced rescue candidate but selects by checked survival
and saved length, not a fixed cut copied from one opponent.

V4's route search can value a pearl pocket followed by a rear split only if the
observed food gains cover the two-segment parent and still leave a strictly
longer child, with a checked exit. It does not count unseen future spawns as
already collected. This is a bounded four-move exit check, not an exhaustive
solver for every chamber or an assertion that every L-shaped pocket is profitable.

Food choices now include the wait until a known spawn attempt and avoid giving
full credit to a spawn our own body would block. Longer allies receive soft
priority using terrain routes, a short clear approach, threat checks and an
eight-round expiry. This replaces straight-line sharing through walls and avoids
leaving an inaccessible pearl reserved indefinitely. Hidden spawn distributions
are not inferred from replay visuals or a single countdown.

## Sacrificing workers: confirmed tactic, carefully limited

The user's Trauma observation is supported by actions, not just timing:
calc's worker 182 at round 406 and worker 188 at round 413 explicitly chose
`suicide`. Cutlery's worker 166 at round 322 used an illegal split of one while
having empty moves. For length-at-most-six workers whose pearls were collected
within three rounds by a same-team length-at-least-eight dragon, the audit finds:

| Reference | Side | Collected death pearls | Distinct workers |
|---|---|---:|---:|
| Trauma 459157 | Cutlery | 14 | 12 |
| Trauma 459157 | calc | 17 | 16 |
| Autarky 464013 | Settlers of Battlecode | 20 | 17 |

These aggregate counts do not label every death intentional. Explicit suicide
actions establish that deliberate feeding is present in the Trauma game. Its
winner had longest length 30 versus 20 despite total length 40 versus 60.

V4 implements donations only from round 400, using complete length-two-to-four
non-growers after eight foodless rounds and with at least eight allies alive.
The recipient must be the largest ally currently known, length at least eight,
one open-edge step away, due to act later this round, with an exact reported
length and a fully visible reconstructable body. Nearby threats, existing nearby
food, insufficient time, or an unsafe collection/continuation cancel the donation.
The donor collides once with its own verified neck, killing only itself and
leaving the head pearl in place. This never relies on an invalid action.

The rule verifies a collection opportunity; it cannot force the recipient's
choice or prevent every unseen intervention. Its strict population floor and
nearby-food check limit worker losses and repeated donations. Competitive gain
has not been measured yet.

## Implementation and verification

- All strategy changes are in `Harshu/bot/`. `MainBaselineBuild` and the frozen
  prior releases were not modified. The user note files and `research/` are untouched.
- Forty-four C++ strategy scenarios pass with `-O2 -Wall -Wextra -Wpedantic`.
  They include the actual opening coil, portal body continuity with/without food,
  unseen-neck rejection, warning expiry, rescue roles, partial opening movement,
  population/phase limits, last-round waits, pocket return and donation exclusions.
- Ordinary search stops at 0.036 seconds on the game clock, reserving work for
  rescues up to 0.052 seconds. These are search cutoffs, not a measured v4 judge
  budget result. Buffered messages still share the turn's final output flush.
- No v4 matches or sandbox runs were launched. There is no claimed v4 win rate,
  performance superiority, or verified judge-resource peak.

The remaining validation is a small comparison against frozen v3 on Slithery
Fight, Autarky, Portals and Prisoners Dilemma, plus a small sandbox budget check.
Run those only after the user elects to proceed. The versioned package is ready
for that validation; it has not been uploaded.
