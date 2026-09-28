# Current planned work — series 481660 review and next bot

The user requests a detailed five-game review and a concrete improvement to v4.1.
Read MASTER first (done), then only targeted documentation. Preserve MainBaselineBuild,
all frozen sources/ZIPs, user notes and research/. Do not submit automatically.

## Work planned

1. Archive the linked series and all five replays, verify maps, sides, upload identity
   and timing from account records. Decode every turn, not just final scores.
2. Measure food collection, map coverage, contests/retreats, deaths, split lineages,
   growth and death-pearl transfers in each phase. Evaluate all five observations
   against what our bot could see. Separate opponent intent from observed actions.
3. Implement supported changes for resource competition, expansion versus rescue,
   and deliberate late concentration of length; avoid unconditional half splits or
   arbitrary length caps that reduce the primary longest-dragon score.
4. Maintain a detailed evidence report and plain-English Gamplan. Freeze and package
   a new version, preserving v4.1. Run focused scenario checks and only a small
   relevant native/sandbox comparison if necessary, with no tuning sweep.
5. Update this file during work so another model can resume. Nothing is complete yet.

## Evidence and implementation decisions now in progress

All five replays are archived/decoded in evaluations/series-481660. analysis.json,
tactics.json and competition.json cover all actions and round snapshots. Trophy:
first 100 rounds food 42 vs 120, central head turns 23/584 vs 812/1689. Devil:
53 vs 155 food before round 100. Not all skipped pearls are safe opportunities.
Dilemma: 166 splits, chiefly 13 -> 2+11 every three rounds, 167 wall deaths;
food 337 vs 345 but final longest 13 vs 36. Trauma: final total 49 vs 13 but
longest 9 vs 13; explicit opponent donations at rounds 413 and 497. Slithery:
opponent population peaks 64, late longest rises from 6 at 300 to 35 at 500.

Implement v5: stronger worker food preference and milder distant-threat penalties;
remember productive areas and seek richer reachable patches rather than assume
geometric center; avoid premature food yielding and low-value voluntary trades;
allow useful normal expansion until 300, transition length-six growers at 250;
carry rescue-chain history across children and try a balanced cut after repeated
early rescues, retaining the large-child default for isolated rescues; begin
bounded feeding at 350 with locally available recipients, smaller late population
floor, and recipient-choice simulation. No fixed maximum scorer length of 9–10:
longest wins, so build secondary growers early but continue raising a scorer late.
Critical additional finding: the recipient-choice fixture walked past its
immediate free pearl. Root attraction plus future food reward double-counted the
promise of delaying collection. Once a continuation is searched, remove root
attraction and keep the continuation's terminal attraction. Add a dedicated
immediate-food regression scenario. This addresses hunger and feeding at their
source, rather than just increasing numerical food weights.
Implementation now passes 63 warning-clean logic scenarios. The root-score bug
was reproduced against frozen v4.1 with its original weights: old choice N,
corrected choice W onto the pearl. A narrow fertile-opening balanced seed is
also implemented for long partial bodies, with a checked two-step parent route
and explicit child-role handoff; the unseen tail remains a stated gamble.
Next: four native comparisons (Trophy/Dilemma, both sides, seed 509) and two
sandbox Trauma comparisons against frozen v4.1. No tuning sweep. Results pending.
Local release is now v5, distinct from server version number 5 (active v4.1).

V4.1 upload receipts: server 10118 (version 4) and 10120 (version 5), identical
source. 10120 is active; fresh marker 2026-09-28T10:52:22.078Z. API match details
omit submission IDs, so exact series attribution remains unexposed. Do not label
server version 5 as the forthcoming local v5. No new uploads in this task.

## Prior state (historical; submission state refreshed above)

# Current planned work — v4.1 validation complete

Updated 28 September 2026. This is the handoff entry point. Work is in `Harshu/`.
Do not alter `MainBaselineBuild`, frozen earlier releases, the user's two note
files, or `research/` as part of this task.

## Current objective and user constraint

The requested v4 implementation and review of `suggestions.txt` plus
`Observations.txt` are complete. The user has now explicitly authorized a small
set of sandbox games and local matches to check for mistakes. Those checks are
complete: six original v4 games followed by four targeted v4.1 games, all against
frozen v3 at seed 509, with both sides exercised. All ten were wins without
runtime errors or engine notices. No further automatic matches or uploads are
planned. Forty-seven focused logic scenarios also pass.

Validation found one concrete corner case after the four native wins: a fresh
ally report beside a hidden portal exit vetoed the only possible escape. The
reproduced Portals turn (seed 509, side A, dragon 4, round 36) chose a wall despite
an empty exit. V4.1 now has a narrow last-resort exception for adjacent-head
reports when no ordinary move exists. Keep exact reported exit heads, known
occupancy, neck reversals and active busy warnings blocked. Frozen v4 is intact.
All 47 logic scenarios, two native Portals games and two sandbox Slithery Fight
games pass for the changed source. Their replays are decoded and audited, and
the report plus frozen v4.1 snapshot are saved. Original v4 passed all four
native and both sandbox games. Keep the two identities separate. Do not expand
to a tuning sweep.

Read `documentation/MASTER.md` first for rules, then only relevant focused pages.
Do not reread all documentation without the user's explicit request. Read
`Gamplan.md` for the complete actual policy, not this shortened handoff.

## Exact release state

- Working source: `bot/`; local release: `versions/current.json`, v4.1 / abyss-v4.1.
- ZIP: `dist/abyss-v4.1.zip`, 18,869 bytes; packaged and checked.
- Fingerprint: `217f6872e1b068b83481e5cc89777406f6615f3a1428ed2ac975c56d289d6aa8`.
- Frozen v4 remains in `versions/v4/`; v4.1 source, strategy, test snapshot and
  validation metadata are in `versions/v4.1/`.
- Neither v4 nor v4.1 is **submitted**. V3 / submission 9700 remains active in the latest
  server snapshot at 2026-09-28 10:12:57 UTC (15:42:57 India time).
- MainBaselineBuild and prior frozen bot sources were not changed.

## Implemented decisions

1. Preserve the known body through blind portals, including an arrival pearl;
   reject blind neck reversals even for newborns. Retain split-off child bodies
   as recent allied obstacles. Add short-lived traffic warnings and single-ray
   probes without treating a sonar miss as proof of a clear exit.
2. Try multiple legal rescue cuts with both ends checked. Prefer the largest
   surviving half, including the actual 25-to-23 rear rescue in Slithery Fight.
   Remove unconditional partial-body opening cuts. Pass a protected role to a
   saved child so it does not immediately shred itself again.
3. Let short lengths four through six breed before round 200; protect seven and
   above immediately. Protect six from round 200, stop normal births at 225, and
   retain the actual 64-slot allowance with local room/resource checks.
4. Send about a third of early small dragons exploring sooner, using persistent
   known routes. Account for spawn waiting and occupied spawn squares. Yield
   food softly to a nearby larger ally, with clear-route and eight-round limits.
5. Consider a pearl pocket plus rear escape only if known food pays the split
   cost and leaves a strictly longer child. Include the last-round parent wait.
6. Allow strictly bounded donations from round 400: a complete small worker,
   population at least eight, a visible larger later-moving recipient with exact
   known body, no nearby threat or alternative food, and a checked collection
   route. The donor hits only its own neck. Receipt is an opportunity, not a
   guaranteed teammate action; no ranked benefit has yet been measured.
7. Ordinary search stops at 0.036 seconds, leaving time for rescue work up to
   0.052. Donation checks do not start past 0.044. Original v4 sandbox maximum was
   25,426,661 points and 786,432 bytes. V4.1's own 54,589 sandbox turns peaked at
   25,426,672 points and 786,432 bytes (0.75 MiB).
   The buffered output convention is unchanged.
8. V4.1 allows a blind portal beside a recent ally report only when no ordinary
   move is legal, after rescue attempts. Exact reported exit heads and all other
   existing safety blocks remain. The original failing turn now chooses west
   through the portal instead of north into the wall; its dragon survives until
   round 45 instead of dying in round 36.

## Verified evidence and locations

- `evaluations/v4-review/REPORT.md` evaluates every major suggestion and the
  named reference games; `tactics.json`, `own-analysis.json` and raw replays
  retain the detailed evidence locally.
- Ten attributed v3 games: 157 initiated portal collisions, including 105 self
  collisions and 35 other-allied victims. Named Borgor series 464366–464375:
  175 portal collisions, 119 self and 42 other-allied victims. All those exits
  were outside the mover's turn-start view. Newer API responses omit the named
  series' exact submission ID; do not label it as verified v3.
- Hampter 450663 and the later Slithery Fight games show repeated long-parent
  peeling versus a large rear escape. Prisoners Dilemma 464370 shows an 11-to-9
  then 9-to-7 same-round cascade despite an empty first step in full replay state.
- Trauma 459157 contains explicit worker suicide actions and subsequent feeding
  of long teammates. Autarky 464013 confirms balanced opening cuts and late
  corpse collection. Do not infer every death or an opponent's hidden algorithm
  from these examples.
- Expanded earlier archive: 200 v2 games, 78 wins; ten explicitly attributed v3
  games, five wins. The latter is only one opponent and is not a controlled
  version comparison. Local v4/v4.1 results are separate from those online games.

## Verification completed

- `tests/strategy_test.cpp`: 47 passing logic scenarios with
  `g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic`; no compiler warnings.
- Includes the real visible opening coil, blind-portal body retention, birth-role
  acceptance, partial opening movement, final-round rescue, split-cap limits,
  pocket return, food-sharing expiry and donation cancellation conditions.
- Original v4 ZIP remains byte-for-byte identical to its frozen source. V4.1
  has its own exact package, working source and frozen snapshot.
- Replay movement reconstruction checked against the official viewer's events.
- Original v4: 4/4 native wins and 2/2 sandbox wins against frozen v3, no errors.
- V4.1: 2/2 native wins on Portals and 2/2 sandbox wins on Slithery Fight, with
  no errors, engine notices or initiated portal collisions. These are regression
  checks, not a ranked win-rate estimate.
- No new broad engine-contract suite. The targeted portal reproduction stopped
  after 37 turns of one dragon rather than running another complete game.
- `evaluations/v4-review/VALIDATION.md` is the current result report. Result JSON
  files are `evaluations/v4-{native,sandbox}.json` and
  `evaluations/v4.1-{native,sandbox}.json`.
- `tools/decode_replays.py` now handles raw local and gzip site replays.
  `tools/audit_tactics.py --local-results` adds local attribution and empty-exit
  checks. Raw replay directories and derived tactics JSON are ignored by Git.

## Remaining work and stopping point

1. No implementation or match work is pending for this request. V4.1 is the
   packaged candidate for the user's next decision. Do not upload automatically.
2. Future evaluation should inspect diverse opponents/maps and particularly
   unreliable donation receipt and remaining hidden-exit vetoes. These are
   limitations, not instructions to start more games now.
3. If an upload is requested, refresh timing first where needed and use the
   upload wrapper so exact bytes and server receipt are recorded. Do not imply
   native outcomes are sandbox outcomes; their clocks can produce different
   decisions. Do not claim the small local sample predicts ranked win rate.

## Timing and commands

`submissions/history.json` preserves all upload receipts. Latest server data
reports v3 kStart=10 and no new kFreshAt marker, so its K is already 24. The
original fresh-window deadline was 28 September 12:04:02.102 IST and has passed.
Six completed ranked series cannot be attributed because the API omits version
IDs; they are retained separately rather than guessed. Two series were queued
at the latest snapshot. Refresh before later advice; upload time is not first
ranked play.

From the repository root:

```sh
.venv/bin/python Harshu/tools/submissions.py status
.venv/bin/python Harshu/tools/submissions.py package
```

Only when an upload is requested:

```sh
.venv/bin/python Harshu/tools/submissions.py submit
```

`tools/refresh_timing.py` now handles missing attribution and no longer hardcodes
that v3 is unsubmitted. `tools/audit_tactics.py` is the repeatable tactical audit.
