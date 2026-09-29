# Eight-game review and scoped v5 candidate

Reviewed 28 September 2026. All five games in the requested UnderTheC series were decoded turn by turn. Three additional ranked games were selected before inspecting their replay tactics: two losses and one win, on different maps against different opponents. This is an explanatory sample, not an estimate of ranked win rate. No submission was made in this task.

## Conclusions that survived the win check

1. **Food access and concentration are separate problems.** Trophy and Devil collapsed economically early. Trauma and the newer Portals loss had ample team length but no sufficiently long surviving scorer. Increasing population or food reward alone cannot fix both.
2. **Preserve growers, useful farms and portal body memory.** The Queen of Spades win collected fewer pearls than the opponent but retained a longer scorer. The opponent repeatedly broke down its own long dragon and suffered portal self-collisions. We should not copy its activity merely because it looks aggressive.
3. **Repeated rescue loops need a different response from an isolated rescue.** Dilemma repeatedly saved an eleven-segment child while sacrificing the front. An isolated large rear escape on Slithery preserved valuable length and should remain available.
4. **Feeding is useful only when the recipient can collect and survive.** There is no merge action, and death returns only roughly half the sacrificed body as pearls. A fixed length-ten ceiling would lose to length-thirteen or longer opponents.
5. **A real scoring defect discouraged immediate collection.** The planner counted the attraction of a pearl now and the reward for eating it later. Removing the duplicate is a targeted fix; broadly weakening danger penalties did not pass the comparison checks.

The broad first implementation was rejected after regressions. The final scoped policy and all experiments are distinguished below and in [Gamplan](../../Gamplan.md).

## Sources and method

Primary sources are the official match details and binary replays, obtained through the authenticated read-only API. The replay decoder uses the official viewer's format. Side attribution uses Borgor team ID 422, not a fixed A/B assumption. Body reconstruction is checked against every round snapshot; movements are checked against actual walls and portal connections. Analyses are local `analysis.json`, `tactics.json`, `competition.json`, and `../v5-crosscheck/*.events.audit.json` / `*.events.tactics.json`. Raw and large decoded files are retained locally, excluded from Git.

The original five games validated 203/501/501/501/501 snapshots and 5,753/6,209/6,229/34,861/11,451 movements respectively. The new three each validated 501 snapshots. No Borgor timeouts or instruction-budget failures occurred in these online games. Opponent instruction counts are redacted and are not meaningful comparisons.

Downloaded initial map food timers are redacted. Actual food collections and natural appearances come from replay events. Geometric center occupancy is descriptive, not proof that heading toward the center is always right. A legal adjacent pearl skipped by the bot is a diagnostic, not proof that collecting it was safe. Opponent intent is inferred only cautiously; explicit suicide actions provide stronger evidence than collecting a body after an accidental collision.

The API does not expose submission IDs for these matches. Timing and the user's reported upload associate this period with v4.1, but exact per-game source identity is unverified. [Timing evidence](TIMING.md) separately records active submission 10120 and its server fresh-window marker. Server version 5 is **local v4.1**, not this new local v5.

## 1. Trophy — early territory and hunger

[Game 481660](https://game.battlecode.au/visualiser?match=481660), Borgor A versus UnderTheC B. Elimination after round 201, 202 rounds played.

| Measure | Borgor | Opponent |
|---|---:|---:|
| Pearls, first 100 rounds | 42 | 120 |
| Turns, first 100 rounds | 584 | 1,689 |
| Central head turns, first 100 | 23 | 812 |
| Central pearls, first 100 | 5 | 63 |
| Pearls, whole game | 67 | 296 |
| Splits | 24 | 127 |
| Population at round 100 | 6 | 26 |
| Longest / total at round 100 | 3 / 14 | 4 / 61 |
| Population at round 200 | 1 | 35 |

The user correctly identified a sustained resource-access failure, not merely a bad final collision. The central region mattered, though it contained 84 of 223 natural food appearances across the whole game, not all food. Enemy initiation accounted for 14 of 25 head-collision pairs; we initiated 11. Our voluntary-looking exchanges frequently involved length-two against length-three enemies, a small gain while heavily outnumbered. Equal-size exchanges can be forced last resorts, so this does not prove eleven reckless voluntary choices.

Examples warranting inspection include round 8 dragon 2 passing a westward pearl at tile 26, and round 23 dragon 0 passing a southward pearl at tile 90. Nearby enemy routes make hindsight alone insufficient. The controlled scoring fixture below independently establishes a collection bug without asserting those exact positions were safe.

**Response:** fix collection scoring, reject marginal voluntary trades, retain direct danger protection. A broader resource-area attraction and reduced distant-threat penalties were implemented but rejected after regressions. No unconditional central invasion is in the final build.

## 2. Trauma — winning the economy, losing the scorer

[Game 481661](https://game.battlecode.au/visualiser?match=481661).

| Round | Borgor count / longest / total | Opponent count / longest / total |
|---|---|---|
| 300 | 10 / 6 / 34 | 6 / 3 / 15 |
| 400 | 9 / 9 / 28 | 3 / 6 / 15 |
| 450 | 13 / 9 / 42 | 2 / 10 / 14 |
| 500 | 15 / 9 / 49 | 1 / 13 / 13 |

We collected 125 pearls to 46. The opponent explicitly suicided dragon 13, length five, at round 413 despite a legal westward move; dragon 24 collected its pearls at rounds 413 and 415. At round 497, another length-five donor, dragon 20, suicided despite a legal westward move; dragon 24 ate at rounds 497 and 499 and finished at thirteen. This supports the user's feeding observation directly.

Old feeding required a recipient of at least eight, a donor of at most four, at least eight teammates, round 400, full visible recipient body and no larger reported ally anywhere. Those restrictions excluded useful local opportunities, including building a grower from length four to seven.

**Response:** earlier bounded feeding, locally eligible recipients, lower late population floor, and actual recipient-choice simulation. Before round 425 prefer eligible recipients below ten; afterwards prefer the largest. Ten is not a cap. There is still no rendezvous protocol, and only recipients acting later in the same round qualify.

## 3. Prisoners Dilemma — repeated large-child rescue

[Game 481662](https://game.battlecode.au/visualiser?match=481662).

Food was almost equal, 337 versus 345, yet we finished with one length-thirteen dragon against five enemies with maximum thirty-six and total sixty. We split 166 times and suffered 167 wall deaths. The recurring sequence was length thirteen → parent two plus child eleven approximately every three rounds: original dragon 0 at round 2, child 13 at round 5, child 17 at round 8, child 18 at round 11, and onward.

The starting long body extends outside vision. Repeated growth while moving up the narrow column prevents recovering the full tail before another trap. Many cuts therefore use the **uncertain partial-body fallback**, not the full-body checked rescue. Changing only the checked rescue would miss the main loop.

The opponent initially used 11 → 6+5, then 5 → 3+2. At round 300 it had fourteen dragons and maximum four; by 450 it had four dragons and maximum twenty-two, then reached thirty-six. Its full internal policy is unknown.

**Response:** carry recent-rescue history to children; before round 350, repeated rescues of length at least eight try balanced cuts. Release suitably small children to ordinary worker rules while retaining a protected parent. A narrow fertile round-zero partial-body seed also uses a balanced cut, requiring a visible two-step parent route. Its unseen child remains a risk, explicitly documented. Isolated and late rescues still maximize saved length.

## 4. Slithery Fight — preserve our large survivor, improve distribution

[Game 481663](https://game.battlecode.au/visualiser?match=481663).

| Round | Borgor count / longest / total | Opponent count / longest / total |
|---|---|---|
| 100 | 41 / 24 / 128 | 46 / 4 / 114 |
| 200 | 44 / 26 / 133 | 54 / 4 / 132 |
| 300 | 30 / 23 / 110 | 64 / 6 / 160 |
| 350 | 21 / 21 / 91 | 40 / 14 / 207 |
| 400 | 19 / 21 / 80 | 48 / 12 / 271 |
| 450 | 17 / 19 / 67 | 21 / 25 / 252 |
| 500 | 18 / 24 / 90 | 18 / 35 / 269 |

The user correctly observed a population-to-length transition around 300–400. Opponent food collection was 2,207 to our 1,115, so concentration alone was not the entire difference. It split 763 times versus our 384. Many opponent deaths were explicit suicide actions, but not every body collection proves a deliberate feeding plan.

Its own corpse pearls collected within three rounds by recipients already length seven or more numbered fourteen in the 300s and forty-three in the 400s. Examples: round-401 donor 972 length two fed recipient 950, already length seven, at 403; donor 1023 fed length-eleven recipient 1033 at 403. Our own long-body collection total includes recycling trapped remnants and cannot all be called coordinated feeding.

**Preserve:** our isolated large rear escape kept a roughly length-23 scorer alive. Unconditionally halving that dragon would discard a working strength. We initiated zero portal collisions here, versus eighteen opponent portal collisions. Keep portal body continuity and direct occupancy checks.

## 5. Devil — population loss precedes the endgame

[Game 481664](https://game.battlecode.au/visualiser?match=481664).

We collected 53 versus 155 pearls before round 100, and 76 versus 1,272 overall. Our populations at rounds 100/200/300/400 were 8/3/2/1, versus 24/29/33/27. We finished with one length-two dragon versus six enemies, maximum thirty-one and total 113. Enemy initiation accounted for eleven of eighteen head-collision pairs; we initiated seven.

This supports the user's impression of displacement, but attributing everything to retreat logic overstates what replay events prove. The economic failure was established long before late feeding could matter. Preserve survival options while improving collection; do not try to repair this only with sacrifices at round 450.

## Two newer losses and one win

Selection used the latest completed ranked records, varied opponents/maps, and was recorded before tactical inspection. Queued games and the newer 10–0 unranked series were not substituted for ranked evidence.

| Game | Opponent / map | Our food vs theirs | Our final count / longest / total | Their final count / longest / total |
|---|---|---:|---|---|
| [495301](https://game.battlecode.au/visualiser?match=495301), loss | Gilligan’s crew / Portals | 659 / 352 | 17 / 6 / 56 | 3 / 12 / 17 |
| [493949](https://game.battlecode.au/visualiser?match=493949), loss | Madda Guddu / Trauma | 162 / 14 | 13 / 12 / 64 | 1 / 14 / 14 |
| [487046](https://game.battlecode.au/visualiser?match=487046), win | not gra / Queen of Spades | 207 / 251 | 8 / 7 / 31 | 13 / 3 / 29 |

**Portals:** we already had fifteen dragons and total forty-one at round 100 against two and seven. At round 450 our maximum was four, theirs five; they reached twelve at the end. We split 212 times and had 177 wall deaths. The largest direct death was only length six: growth was repeatedly broken up by rescues rather than one catastrophic long-dragon death. There were no prompt own-corpse meals to recipients already length seven in this game. One own blind portal collision occurred; 207 distant portal steps also show we were not simply refusing portals. This loss strengthens the concentration diagnosis, not a demand for indiscriminate portal risk.

**Trauma:** the opponent reached length thirteen with one survivor by round 300 while we had ten dragons, maximum six, total thirty-one. We reached length fourteen earlier but later gave away segments in rescues; final maximum twelve lost to fourteen. No own portal collision occurred. Food quantity was emphatically not the limiting resource. Only two prompt corpse meals reached our existing length-seven-or-longer recipients.

**Queen win:** at round 200 the opponent had length fifteen, while our maximum was four. From rounds 240–245 its dragon 0 repeatedly split from sixteen down to four. Our maximum reached seven by round 400 and remained seven through the end. The opponent also repeatedly reversed into its own body through portals; we had one blind allied-body portal collision and no portal self-collision. We won with fewer dragons and fewer pearls. Preserve our grower protection and portal memory; do not replace productive farming with forced exploration or automatic repeated splitting. This is a real strength, though a win does not imply every move was good.

## Controlled collection-scoring defect

[food-score-audit.cpp](food-score-audit.cpp) runs the same candidate enumeration and lookahead against frozen v4.1 with unchanged original weights. The old planner chooses north, score 29.8413. Removing the duplicate root attraction chooses west onto the free pearl, score 26.103. Scores across the two formulas are not directly comparable; the chosen action is the relevant result.

The final fix removes current-position attraction only after a continuation has been searched. Unsearched candidates retain their shortlist heuristic. A focused scenario also checks immediate eating. This demonstrates a defect in a controlled position, not proof that every skipped online pearl was safe.

## Implemented scope and rejected experiments

Final policy details, including all thresholds, are in [Gamplan](../../Gamplan.md). Retained changes: collection scoring, remove redundant sharing penalty, no soft reservation near a three-step enemy threat, stronger minimum value for voluntary head trades, recent-rescue handoff and balanced repeated escapes, limited fertile opening, and checked local feeding from round 350.

Retained baseline strengths: round-200 grower protection; round-225 breeding stop; 64-slot allowance with crowding checks; existing persistent exploration; 12-point food reward; danger weights 130/36/6; largest isolated checked rear escape; portal neck/body continuity, warnings and direct-occupancy vetoes. No fixed scorer length cap.

The broad experiment added resource-area targets, raised food reward to eighteen, weakened distant worker threats and extended breeding to 300. Delaying growers to 250 won three of four native Trophy/Dilemma checks but lost both sandbox Trauma checks. Restoring growers to 200 alone lost both Trauma and both Queen checks. These are **rejected experiments**, not final-build successes. The narrow build has its own validation below. No automated parameter sweep was run.

## Remaining strategic limits

Feeding still requires an adjacent higher-ID recipient and uses a model with less memory than the real recipient. It does not arrange meetings or guarantee collection. Rescue lookahead holds other bodies still, and unseen-tail opening/rescue decisions cannot be certified. The bot has no exact global enemy-length estimate, territorial team orders, or exact endgame solver. Short fixed-seed checks against v4.1 cannot establish performance against UnderTheC or general ranked improvement. Do not upload a candidate merely because it is newer.

## Exact validation and release decision

**Decision: hold v5 as an experimental build; do not recommend replacing active v4.1.** It fixes specific scenarios but did not demonstrate a net competitive improvement. This is the consequence of weighting our winning behavior alongside the losses, rather than claiming every requested aggression change is beneficial.

| Build fingerprint | Changes / tests | Results versus frozen v4.1 |
|---|---|---|
| `b48a37ba07eafd7742609e094949346b5a6b484d6aad59ff083b286fcda64143` | Broad experiment, growers 250; `v5-native.json`, `v5-sandbox.json` | Trophy 1/2, Dilemma 2/2 native wins; Trauma 0/2 sandbox |
| `6027bd014feb0a8f54bf154a55bab947dcc558bb0789bfc669b7655df0f5131a` | Broad experiment, growers restored 200; misleading historical filenames `v5-final-*` | Queen 0/2 native; Trauma 0/2 sandbox; rejected |
| `53fbd1fc2c0f6b5c2d8f76971df564bc12031096bc9421017638de1a6b68335c` | Scoped v5; `v5-scoped-native.json`, `v5-scoped-sandbox.json` | Queen 1/2 native; Trauma 0/2 sandbox; held |

All matches used seed 509, both sides, toolkit 1.2.2. Fourteen matches total across three explicitly recorded revisions, without a parameter sweep. The initial Dilemma wins do **not** validate the final scoped fingerprint on Dilemma.

Final scoped results (count / longest / total):

| Mode / map / v5 side | V5 | V4.1 | Result |
|---|---|---|---|
| Native Queen / A | 9 / 7 / 43 | 9 / 7 / 37 | Win on total-length tiebreak |
| Native Queen / B | 8 / 4 / 22 | 17 / 9 / 69 | Loss |
| Sandbox Trauma / A | 12 / 13 / 46 | 23 / 14 / 90 | Loss |
| Sandbox Trauma / B | 10 / 7 / 32 | 17 / 14 / 67 | Loss |

The final source passes **60 warning-clean C++ strategy scenarios**. All four final matches completed without runtime errors or engine notices. Its 7,669 measured sandbox turns peaked at **25,802,662 CPU points** and **720,896 bytes**. This checks resource headroom on these games, not every possible map. Native performance does not substitute for judge measurements.

Final replay audit found no initiated portal collisions in Queen; one enemy-head collision through an unseen portal in Trauma, with no final-build allied or self portal collision. Only one prompt own-corpse pearl reached a recipient already length seven (Trauma A, donor 4 at round 382, recipient 6 at 384). The recipient did not take it immediately. The feeding policy is therefore not established as effective team-wide concentration. Fixture success and a safe opportunity are not enough.

The frozen [v5 manifest](../../versions/v5/manifest.json), [source](../../versions/v5/bot/strategy.hpp), and [ZIP](../../dist/abyss-v5.zip) preserve this exact experiment, not an endorsed upload. The working Gameplan describes it accurately. The next useful investigation is recipient/donor coordination and grower route choice before loss of length, using these saved replays; another broad aggression sweep is not justified.
