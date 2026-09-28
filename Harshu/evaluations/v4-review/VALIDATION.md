# V4 and v4.1 local validation — 28 September 2026

The user authorized a conservative set of native and sandbox games after the
implementation review. Nothing was uploaded. Native games exercise the official
engine; sandbox games additionally measure the judge's CPU points and memory.
All completed games use toolkit 1.2.2, seed 509 and frozen v3 as the opponent.
There is no seed sweep, online evaluation or claim of a ranked win rate.

## Exact sources

| Build | Source fingerprint |
|---|---|
| v3 opponent | `52ffbaf11ec9244f9dc7333b81a4e18c1ed85b29f0028c21ca837599b070535d` |
| Original v4 | `9ab45c60f07dc9f6e14bdbeee225a870b765ce9e6328a80abfae627f5db02497` |
| Revised v4.1 | `217f6872e1b068b83481e5cc89777406f6615f3a1428ed2ac975c56d289d6aa8` |

Original v4 source and ZIP remain frozen. The revised candidate has its own
18,869-byte [ZIP](../../dist/abyss-v4.1.zip). Evaluations compile immutable copies
named by these fingerprints. `MainBaselineBuild` and earlier bot releases are
unchanged.

## Completed matches

Lengths and population below are from the final replay state. "Us" means the
candidate in that row; both sides were exercised for each map/mode pair.

| Build | Mode | Map | Side | Result | Longest us / v3 | Population us / v3 |
|---|---|---|---|---|---|---|
| v4 | Native | Slithery Fight | A | Win | 40 / 8 | 62 / 40 |
| v4 | Native | Slithery Fight | B | Win | 32 / 9 | 35 / 62 |
| v4 | Native | Portals | A | Win | 6 / 6 | 21 / 15 |
| v4 | Native | Portals | B | Win | 6 / 5 | 21 / 14 |
| v4 | Sandbox | Slithery Fight | A | Win | 12 / 11 | 64 / 22 |
| v4 | Sandbox | Slithery Fight | B | Win | 13 / 8 | 52 / 39 |
| v4.1 | Native | Portals | A | Win | 6 / 6 | 20 / 15 |
| v4.1 | Native | Portals | B | Win | 6 / 5 | 21 / 14 |
| v4.1 | Sandbox | Slithery Fight | A | Win | 12 / 12 | 58 / 21 |
| v4.1 | Sandbox | Slithery Fight | B | Win | 13 / 8 | 52 / 39 |

All ten games reached round 499 without bot runtime errors or engine notices.
The planned validation is complete; no additional automatic games are needed.
Native and sandbox decision paths can differ
because their clocks and search cutoffs differ; native longest lengths must not
be presented as sandbox results.

## Concrete mistake fixed in v4.1

In original v4's Portals game on side A, dragon 4 reached round 36 with four
segments. North and east were walls; south was its own neck. Its only possible
escape was west through portal 18. The hidden exit was empty in the replay, but
an ally's previous-round report placed that ally one square beside the exit.
The broad nearby-ally veto rejected the portal, and the bot selected the north
wall as its fatal fallback.

A targeted replay of just the first 37 turns captured that dragon's real engine
inputs. Inspecting its actual memory confirmed: complete body, no occupied exit,
no busy warning, and one adjacent ally report. The original reply was `MOVE N`;
the revised source replies `MOVE W` on that same captured input. In the full
revised native game, it crosses successfully and survives until round 45.

The exception is deliberately narrow: only when no legal ordinary move exists,
after trying rescue splits. It relaxes a report **beside** the exit. A reported
head directly on the exit, recent known body occupancy, own-body occupancy,
blind neck reversal and active portal warning remain disqualifying. Normal
exploration retains the larger exclusion distance.

In the side-A Portals rerun, wall deaths with an actually empty hidden exit
dropped from 42 to 13. Side B stayed at 34. These whole-game trajectories differ;
the drop is supporting evidence, not a count of independently proven fixes.
Some remaining vetoes are intentional protection of another dragon, and an
empty exit known from the full replay is not necessarily knowable to the bot.

## Rescue, portal and feeding checks

- Both native Slithery Fight openings preserve a 23-segment child from the
  initial 25-segment coil. Frozen v3 peels off two-segment children. V4 also
  performs later large rear rescues, rather than repeatedly shredding a long
  trapped parent into small workers.
- Original v4 initiated zero portal collisions across the six completed games;
  opposing v3 initiated 140. Revised v4.1 also had zero in all four of its games.
  This sample demonstrates that the earlier failure pattern was absent here;
  it does not guarantee a clear hidden exit in future games.
- The full movement reconstruction validates each successful replay step against
  map walls and portal destinations. Wall deaths mostly occur with no empty
  first step in the full state, often among small remnants. We separately counted
  empty hidden portal exits to expose the rule above.
- Late feeding is mixed. In original v4's sandbox side-A game, length-two donors
  315 and 339 died in rounds 413 and 457, and length-eight teammates collected
  their pearls in the same round. In side B, donor 278's pearl reached a
  length-eight teammate in round 497, while donor 356's reached a length-twelve
  teammate two rounds later. Native side A also contains diverted or uncollected
  food. We do not label every small self-collision as a planned donation or claim
  that recipient choice is guaranteed.

## CPU, memory and focused checks

Original v4's two sandbox games covered 54,597 candidate turns. The highest
observed CPU use was **25,426,661 / 100,000,000 points**; maximum linear memory was
**786,432 bytes**, below the 48 MB limit. Both sides had zero runtime errors.
V4.1's two sandbox games covered **54,589 candidate turns**, with maxima of
**25,426,672 points (25.43% of the limit)** and **786,432 bytes (0.75 MiB)**.
Its per-side 99th-percentile CPU values were 20,899,567 and 20,359,294 points.
This is substantial observed headroom, not a worst-case proof for every map.

V4.1 passes 47 strategy scenarios compiled with
`g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic`, with no warnings. The three added
scenarios check last-resort escape beside an ally report, retained exact-head
and busy-warning vetoes, and retained caution when an ordinary move exists.

## Reproduction and retained evidence

Results with exact source identities:
[v4 native](../v4-native.json), [v4 sandbox](../v4-sandbox.json),
[v4.1 native](../v4.1-native.json), [v4.1 sandbox](../v4.1-sandbox.json).
Each result file has a sibling `*-replays/` directory locally, including decoded
states. Derived `*-tactics.json` files retain the detailed audit. Large raw and
derived replay files stay local; summaries and match result JSON remain trackable.
The targeted engine input capture is retained locally as
`portal-fallback-inputs.json` alongside this report.

`tools/decode_replays.py` now accepts raw engine replays as well as gzip-compressed
site downloads. `tools/audit_tactics.py --local-results <results.json>` verifies
the replay's map/index pairing and attributes sides to the recorded local
sources, without inventing online submission IDs. These changes concern the
analysis tools and are not part of the submitted bot.
