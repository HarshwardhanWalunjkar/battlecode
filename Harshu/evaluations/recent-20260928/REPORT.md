# Online replay audit and v3 response — 28 September 2026

The recurring problem is **insufficient longest-dragon growth**, despite often
having plenty of survivors. Of **194 archived v2 games against 15 opponents**,
we won **75** and lost **119**: **112 losses reached the round limit**, while
only seven were elimination losses. In 108 of those scoring losses our longest
dragon was shorter; four lost the total-length tiebreak. In **56 scoring losses
we had more living dragons**, and in **37 we had greater total length**.

The expanded archive contains **214 newly audited games**: 194 on v2 and 20
later games on v1. The original 20-game v1 debut remains separately archived
in `../online/`. This refresh added **119 replays** after the earlier 95-game
download. Six games in the fetched series inventory had no completed replay
yet. Battle listing cutoff: **2026-09-28 04:47:31 UTC / 10:17:31 IST**; some
individual games finished while details were downloading. Ranked timing uses
the listing cutoff consistently; this game audit includes each downloaded
completed game, including four from still-live series.

## Repeated findings and action taken

| Finding | Evidence across v2 games | v3 response |
|---|---|---|
| Population keeps replacing concentrated length | 14,871 of 19,145 splits started at length four; 5,548 splits occurred at round 350 or later. More population did not prevent 56 scoring losses. | Protect selected growers and every length-six-or-longer dragon once population reaches four. Stop ordinary splitting at round 350. |
| Circling without growth recurs beyond Queen of Spades | A deliberately strict detector found 31 games across eight maps with an individual spending at least 80 consecutive round snapshots at fixed length 2–4 and within 12 head squares. | Keep a travel destination for up to 24 rounds, reward progress toward unexplored edges, portal entrances or long-unvisited ground; preserve productive food loops. |
| Child survival is inadequately checked | 1,546 newborn dragons died in their birth round or following two rounds, across nine maps; 546 died in the birth round itself. | Check the actual tail, adequate room and a two-move continuation. Treat an unseen-tail split as an explicit uncertain rescue, not a verified safe split. |
| Friendly collisions and congestion are frequent | 832 head-collision events between our own dragons across 107 games. | Reject ordinary splits into locally crowded areas; protect growers from frequent blind portal gambles. This will not solve all movement coordination problems. |
| Runtime is healthy | 1,778,010 recorded v2 turns, zero timeout/budget-exceeded flags, zero no-valid-action deaths; peak reported CPU 25,821,908 of 100,000,000 points. | Focus on decisions; retain bounded search and check the changed build in two sandbox games. |

Counts show repeatable symptoms. They do **not** prove every split, small loop,
or newborn death was avoidable. In particular, enemy movement after a split can
invalidate an initially promising child route. The v3 response is a tested
candidate, not proof that each symptom is eliminated.

## Your three suggestions

**Use all 64 slots:** accepted as permission to expand, not a compulsory target.
The previous 48 cap was a heuristic, not a game rule. Filling every slot by
cutting every grower would repeat the main scoring failure. v3 allows all 64
under the map's actual limit, keeps some growers, checks child space, and avoids
ordinary reproduction near three visible allied heads once team population is
at least eight.

**Increase exploration:** accepted. v2 only penalized very recent visits and
could only choose a blind portal when already beside it. v3 can deliberately
travel toward an exit or unexplored region. Scouts start after eight foodless
rounds; growers wait twenty. Nearby credible food delays exploration. Two- and
three-step enemy threats are penalties rather than automatic vetoes on a route.

**Trade a smaller dragon for a larger enemy:** accepted with survival and grower
checks. A non-grower of length at most four can attack a visibly larger enemy
when at least three allies are alive, even if ordinary safe moves exist. It
prefers the largest verified target. Being longer does not protect an enemy
from a head collision. But sacrificing the last survivor loses, and sacrificing
our best scorer can worsen our longest-dragon position. Enemy visible length is
a lower bound, so the decision does not assume knowledge of an unseen tail.

Every threshold and exception is specified in [Gamplan.md](../../Gamplan.md).
The new research folder was not read or modified.

## Queen of Spades: the game you described

**Game 421846, game 2 of series 421845, tridev6509 versus Borgor**, ended at
zero-based round **207**, rather than near round 500. Their final dragon, ID 2,
had length 11. It moved east through a portal and hit our length-three dragon,
ID 29. Both died; we won by elimination with four dragons remaining, longest
three and total length ten. Immediately before that collision, we were behind
on longest length. This was not evidence of a strong final length position.

The two small chambers are not sealed traps: they have portal exits. They are
also relatively fertile food areas. Staying there can be useful. The actual
failure is keeping length low through splitting, sprint costs, and crowding,
or failing to find productive territory elsewhere. A blanket rule to abandon
every loop would throw away useful farms.

This issue repeats across opponents:

| Game | Opponent | Our survivors / total length | Enemy survivors / total length | Longest comparison | Result |
|---:|---|---|---|---|---|
| 419284, Queen of Spades | just bored | 15 / 54 | 2 / 44 | 7 versus 31 | Loss |
| 417187, Queen of Spades | tridev6509 | 9 / 23 | 4 / 77 | 4 versus 33 | Loss |
| 415807, Queen of Spades | Low Cortisol | — | — | 5 versus 12 | Loss |
| 420562, Slithery Fight | Wapow | 64 / 236 | 3 / 55 | 7 versus 24 | Loss |
| 415847, Trauma | Low Cortisol | 11 / 35 | 3 / 16 | 6 versus 12 | Loss |

Across all 17 downloaded v2 Queen of Spades games, we won five and lost twelve,
all twelve losses on final length scoring. The proposed grower protection is
therefore supported by much more than the single fortunate elimination win.

## Review of the submitted v2 code

- Ordinary splitting took precedence over movement and food for every eligible
  length-four dragon below population 48 until round 420. There was no protected
  scorer. v3 explicitly separates growth protection from breeding.
- `safe_split` accepted a partial body in emergencies, then used the end of its
  visible prefix as the actual tail. An isolated compiled example confirmed it
  approved a child with no real exit. v3's checked splitting requires a complete
  body; uncertain rescue follows a separate policy.
- v2's hard filter discarded all positions within three enemy steps whenever
  any more sheltered move existed, even after the numerical danger penalties
  were reduced. v3 only applies that hard filter to immediate one-step danger.
- Previous local results were genuine immutable-build records, but used native
  execution, eight maps and weak baselines; Queen of Spades was omitted. The
  handoff's claim that all eleven v2-versus-v1 losses were scoring losses was
  incorrect: ten were scoring losses and one was elimination.
- The old game plan said partial bodies could not sprint, although v2 permitted
  them. v3's game plan now describes the implemented conservative partial-body
  movement. Ordinary splitting stops at the stated boundary (v3: rounds 0–349),
  rather than including round 350.

## Results by map for archived v2 games

| Map | Games | Wins | Losses at round limit |
|---|---:|---:|---:|
| Autarky | 22 | 3 | 19 |
| Default | 24 | 17 | 7 |
| Devil | 20 | 9 | 10 |
| Portals | 21 | 13 | 8 |
| Prisoners Dilemma | 16 | 5 | 9 |
| Queen Of Spades | 17 | 5 | 12 |
| Schooltime | 18 | 12 | 6 |
| Slithery Fight | 19 | 3 | 16 |
| Trauma | 16 | 3 | 13 |
| Trophy | 21 | 5 | 12 |

Opponents and versions are not controlled between online sets. These results
identify recurring weaknesses; they are not a fair experimental comparison
between v1, v2 and the unsubmitted v3.

## Conservative validation of v3

- **22 targeted C++ scenarios passed**, including use of slot 64, the real cap,
  grower protection, unknown-tail rejection, a child's dead end, upward trades,
  last-survivor protection, persistent exploration and preserving nearby food.
- **Eight native games**, seed 509, both sides on Queen of Spades, Autarky,
  Portals and Slithery Fight against the exact submitted v2: **7 wins, 1 loss**.
- **Two sandbox games**, Queen of Spades seed 509 both sides against v2:
  **2 wins**; **11,025 own turns**, maximum **22,615,866 points**, maximum
  **524,288 bytes**; no runtime errors or no-valid-action deaths.
- Eight submission timing checks passed. The final source fingerprint matches
  both evaluation records and the packaged ZIP. Package contents were checked.

Native and sandbox games can follow different choices because search time is
measured differently. The small sample is encouraging and checks the changed
behaviour; it does not establish a reliable online win rate. No broader tuning
sweep or repeated benchmark campaign was run.

v3 fingerprint: `52ffbaf11ec9244f9dc7333b81a4e18c1ed85b29f0028c21ca837599b070535d`.
The [native results](../v3-targeted-native.json) and
[sandbox results](../v3-targeted-sandbox.json) retain exact opponents and builds.

## Reproducibility and timing

Raw battle JSON, original replays, decoded events, `inventory.json` and
`analysis.json` are retained here and excluded from version control. Parsing
uses the official toolkit viewer. Our reconstructed bodies were checked against
the official viewer at **103,244 round snapshots** across all 214 games.
Team orientation and submission ID are read for every game; we do not assume
Borgor is always team A or that games finishing after an upload used that upload.

Tools: `tools/fetch_recent.py`, `tools/decode_replays.py --states`,
`tools/audit_recent.py`, and `tools/refresh_timing.py` (run from `Harshu/` using
`../.venv/bin/python`). Replay downloads keep authorization on the contest host.

[Latest timing](TIMING-LATEST.md): v2 had 20 completed ranked series at the
listing cutoff, **34 wins / 66 losses**, effective ranked count **22**, and
minimum **K = 24**. Two live series are excluded. The recorded fresh-code
window ends **28 September at 12:04:02.102 IST**. v3 is a local package and
has **not** been uploaded. Refresh timing before later submission decisions.
