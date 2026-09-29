# V7 validation — exact integrated source

Final source fingerprint: **`0ce4a44b3c76102afe2813fde172cfe4dae61be1c2936931bf08cc181db35186`**. Opponent: frozen **v4.1**, fingerprint `217f6872e1b068b83481e5cc89777406f6615f3a1428ed2ac975c56d289d6aa8`. Tests, current source and real-engine coordination fixtures identify the same final bot source. Toolkit 1.2.2; sandbox compiler is the judge's C++ compiler.

The final [ZIP](../../dist/abyss-v7.zip) is **24,974 bytes**. Its five files match
the working and frozen source byte for byte. Extracted into a clean temporary
directory, it compiled warning-clean. [PACKAGE-CHECK.json](PACKAGE-CHECK.json)
records the ZIP hash and preserved historical source identities; the frozen
[manifest](../../versions/v7/manifest.json) records all validation counts.

## Result and recommendation

**Six wins and two losses in eight sandbox games**, seed 823, all maps on both sides. The set was declared before results: Trophy and Slithery as weak-map checks, Queen and Trauma as preservation checks. No runtime errors or engine notices. This supports packaging v7 as the next candidate; it is not a measured ranked win rate or proof that every addition helped. V7 has not been submitted.

| Map | V7 side | Result | Longest v7 / v4.1 | Total length v7 / v4.1 | Agreed deaths / intended pickups |
|---|---|---|---:|---:|---:|
| trophy | A | Win | 18 / 9 | 123 / 56 | 16 / 16 |
| trophy | B | Loss | 15 / 15 | 53 / 146 | 13 / 13 |
| slithery_fight | A | Win | 35 / 9 | 155 / 235 | 9 / 8 |
| slithery_fight | B | Loss | 12 / 13 | 159 / 214 | 19 / 16 |
| queen_of_spades | A | Win | 16 / 9 | 60 / 53 | 0 / 0 |
| queen_of_spades | B | Win | 17 / 11 | 57 / 41 | 2 / 2 |
| trauma | A | Win | 12 / 10 | 30 / 64 | 2 / 2 |
| trauma | B | Win | 15 / 11 | 30 / 88 | 2 / 2 |

All games reached the round limit. In the Trophy loss the longest lengths tied at fifteen, so total length decided it. Slithery's loss was twelve versus thirteen; its win was thirty-five versus nine despite lower team total. Queen and Trauma both won on both sides. Those results preserve valuable length-focused behavior; maximizing population is not the victory condition.

## Resource and correctness checks

- **72 C++ scenarios**: 41 retained baseline checks and 31 focused v7 scenarios. Warning-clean `g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic`. Covers status/control separation, corrupt/stale messages, ID order, last-round limits, no-agreement refusal, actual promised-food priority, two-turn grid phase, blocked approaches, partial ally sizes, tail-origin sonar, portal probe isolation, affordable food-funded enemy sprints, standoffs, territory ties, capped floods, rescue roles, replacement splitting and enemy closure of a sole exit.
- **Two official-engine coordination fixtures**, one for each ID order. The production chooser/commit code is used, with round headers shifted to 450 to avoid 450 warmup rounds. The engine handles real moves, sonar, deaths and pearls. Both show delivered agreement, donor self-death, intended pickup and the engine-confirmed increase from length eight to nine. These are short mechanical fixtures, not competitive games. Their source fingerprint is in [coordination-engine.json](coordination-engine.json).
- **Twelve timing checks**: includes the authoritative fresh-window marker when newer battles lack version attribution, and refreshes that marker without fabricating completed counts.
- **81,311 final-source sandbox turns**: maximum **25,836,992 CPU points** against the 100,000,000 limit; maximum memory **917,504 bytes** (0.875 MiB) against 48 MiB. These are observed maxima, not proofs for every future board state.
- Reconstructed the eight result replays against **4,008 state snapshots**. [final-sandbox-audit.json](final-sandbox-audit.json) records phase metrics and actual agreement/death/pickup links. Replay files are in [final-sandbox-replays](final-sandbox-replays/).

Commands used from the repository root:

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic Harshu/tests/strategy_test.cpp -o /tmp/abyss-v7-checks
/tmp/abyss-v7-checks
.venv/bin/python Harshu/tests/coordination_engine.py
.venv/bin/python Harshu/tests/submissions_test.py
.venv/bin/python Harshu/tools/evaluate.py --bot Harshu/bot --opponents Harshu/versions/v4.1/bot --maps trophy slithery_fight queen_of_spades trauma --seeds 823 --sandbox --output Harshu/evaluations/v7-review/final-sandbox.json --replays
```

Do not rerun the matches merely to reproduce a favorable number. The [raw result file](final-sandbox.json) records exact source, immutable builds, sides, errors, deaths and budgets.

## Coordination in competitive-sized games

There were **132 unique acknowledgements delivered to their intended donor**, followed by **63 matching own-neck deaths**. Of those, **59 intended collectors ate the promised head pearl within two turns of the acknowledgement**. Receipt did not always lead to sacrifice: changed position, role, threat, route or eligibility can cancel an agreement. The four missed intended pickups are separately traced in [missed-collections.json](missed-collections.json). Death is only labelled agreed when its ID, head square and timing match a delivered acknowledgement; other corpse meals are not called deliberate feeding.

Some maps had no donations, including the Queen A win. A protocol that works mechanically need not dominate every map, and a win cannot be attributed solely to feeding. Soft reservations and local visibility do not guarantee another dragon will leave a meal untouched.

All four misses were on Slithery. Three head pearls went to other allied dragons
(lengths two, five and three), so the team retained the food but the intended
concentration failed. The fourth remained until an enemy collected it 22 rounds
after the donor died; the collector itself survived. The trace proves the failed
pickup, not the collector's internal reason for declining it. A blanket ban on
donations near any other ally has not been added: it would also suppress useful
transfers, and these results do not establish the size of that cost. These are
explicit residual coordination weaknesses for the next attributed online review.

## Remaining competitive weaknesses

Slithery opening collection remained below v4.1 on both sides: 123 versus 371, and 196 versus 295, with fewer worker births. The thirty-five-segment win shows conversion of length can compensate; it does not erase the opening weakness. Trophy was also side-sensitive (88/44 opening food in the win, 53/64 in the loss). Do not describe these tests as a demonstrated universal opening improvement over v4.1 or Lozer.

This test set is one opponent, one seed and four maps. There was no competitive full-length native tuning sweep. Local map variants, unseen tails, hidden portal traffic, public protocol imitation and other human-designed opponents can expose further cases. A later online sample should include wins as well as losses and attribute submissions only when the server supplies evidence.

## Work before the final source

An earlier v7 working source, fingerprint `6f0b61485f9da65358df6281b558cfc87d8a4368126b5a30760d59e9dc6a03e5`, completed two Trophy sandbox wins. Its third game was interrupted when the independent engine fixture exposed the one-turn feeding phase problem. The completed results are retained in [pre-handshake-fix-sandbox.json](pre-handshake-fix-sandbox.json); they are **not** counted as final-source wins. No intermediate release ZIP or submission was made.

Initial tiny engine fixtures failed to arrange a feeding transfer. Tracing them identified the one-turn-only restriction and partial-size report cancellation; after fixes, both fixtures verified actual collection. A reverse-sonar assertion was also corrected because the original fixture forgot that its supposedly departing ray wraps around the torus. The implementation was not weakened to satisfy that mistaken assertion. These failures and fixes are described in the [review](REPORT.md).

V5 and Claude's v6 results remain historical evidence only. Their games are not pooled into v7's eight-game result. No existing release or MainBaselineBuild file was changed.
