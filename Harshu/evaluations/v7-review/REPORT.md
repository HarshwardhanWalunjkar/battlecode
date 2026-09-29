# V7 independent review and implemented response

V7 is built directly from frozen v4.1, as requested. Claude's intervening v6 and its results were inspected as evidence; its working source was saved in [claude-working-snapshot](claude-working-snapshot/). V5 remains a held historical experiment. [WakeUpLeviathan](../../WakeUpLeviathan) is implemented as one integrated v7, not staged releases. MainBaselineBuild, historical releases, user notes and research/ are outside this change.

The complete implemented policy is in [Gamplan](../../Gamplan.md), with all thresholds and exceptions. [Validation](VALIDATION.md) identifies the exact tested source and every comparison, including interrupted work. [Timing](TIMING.md) separates the live submission from the local candidate.

## Evidence and limits of reconstruction

The independent [event audit](../../tools/reassess_v6.py) reconstructed bodies, splits, deaths, food, vision and sonar across **17 games**, matching **7,613 round snapshots**. The [evidence index](evidence-index.json) records IDs, sides, maps and phase totals. Sources are the original five UnderTheC games, the earlier two-loss/one-win crosscheck, four later games selected to include wins on strong maps, one high-ranked Trauma reference and four new Lozer games.

This is event-level analysis, not running our current bot with forbidden full-board information. Downloaded replay maps redact spawn timings, and old bot memory is unavailable. A legal skipped pearl is not automatically safe to eat. A nearby worker/grower pair is a hindsight opportunity, not proof that both can see each other, deliver sonar or complete a transfer. Corpse collection does not by itself prove deliberate sacrifice. All such metrics are labelled accordingly.

Game details currently omit submission IDs. The latest Lozer series occurred after the v6 upload, but that is insufficient to assign it to a source version: users can reactivate an older bot. We use it as observed team/opponent behavior, not as a proven v4.1-versus-v6 comparison.

At the initial review snapshot, v4.1 had 64 wins and 51 losses. At the 21:04 UTC refresh it had 73 wins and 62 losses; these mix opponents and ranked/unranked play. Trophy was 1/13, Slithery 2/13 and Devil 3/12, while Trauma was 13/16, Queen 9/13 and Dilemma 11/15. This weighs opening/territory failures heavily while making Trauma, Queen and long-body rescues preservation checks. A few selected Trauma losses do not justify rewriting all strong-map behavior.

## Original five games, reconsidered

| Game | Verified evidence | Interpretation and response |
|---|---|---|
| [481660 Trophy](https://game.battlecode.au/visualiser?match=481660) | First 100 rounds: our food 42 versus 120, splits 16 versus 52, turns 584 versus 1,689. Team vision already covered 551/625 tiles. Center head-turns were 23 versus 812. We were eliminated around round 201. | Access and effective pressure failed, not merely discovery of the map. Correct food postponement and impossible sprint fears; value reachable contested room; permit qualified workers to hold ground. Do not hard-code the geometric center. |
| [481661 Trauma](https://game.battlecode.au/visualiser?match=481661) | We collected 125 pearls versus 46 and finished with 15 dragons, maximum nine and total 49. The opponent finished with one dragon of length thirteen. Explicit opponent donation actions occur at rounds 413 and 497. | A clear concentration failure: total food/population did not win the primary longest-dragon comparison. Implement collector agreement and real pickup priority, including lower-ID collectors. Preserve strong-map farming and growers. |
| [481662 Dilemma](https://game.battlecode.au/visualiser?match=481662) | Repeated 13-to-2+11 rescues recur about every three rounds. Food totals 337 versus 345 do not explain our maximum thirteen versus their thirty-six. Long starting chains are partly outside vision. | Repeated rear reversals can consume the working economy. Use early rescue lineage to distinguish a repeated trap from an isolated score-saving rescue; prefer a checked balanced division only under limited conditions. An unseen tail is still unverified. |
| [481663 Slithery](https://game.battlecode.au/visualiser?match=481663) | First-100 food was actually favorable, 308 versus 299. Our 102 splits and 2,268 turns lagged 133 splits and 3,547 turns; coverage 947 versus 1,232 of 1,701 cells. Whole-game food was 1,115 versus 2,207. | Not uniformly a slow-food opening. Distribution, continued useful worker activity and later concentration matter. Keep the useful 23-segment coil rescue. Later corpse meals alone cannot establish that the opponent deliberately merged. |
| [481664 Devil](https://game.battlecode.au/visualiser?match=481664) | First-100 food 53 versus 155, splits 12 versus 65. Whole-game food 76 versus 1,272 and population collapsed. | Sustained access/worker economy failure. More informative frontier targets and affordable threat checks are justified; universal retreats and a fixed center target are not. |

## Wins and newer losses that constrain the diagnosis

The earlier crosscheck used [495301 Portals loss](https://game.battlecode.au/visualiser?match=495301), [493949 Trauma loss](https://game.battlecode.au/visualiser?match=493949), and [487046 Queen win](https://game.battlecode.au/visualiser?match=487046). The Queen win finished with our maximum seven against their three even though the opponent collected more food. Their length-sixteen dragon was broken down around rounds 240–245. Preserving a useful scorer can beat maximizing worker population. Existing portal neck memory and last-resort escape behavior therefore remain.

The later four-game check was selected before implementation: [503086 Trophy loss](https://game.battlecode.au/visualiser?match=503086), [504446 Slithery loss](https://game.battlecode.au/visualiser?match=504446), [504448 Trauma win](https://game.battlecode.au/visualiser?match=504448), and [503089 Queen win](https://game.battlecode.au/visualiser?match=503089).

- Trophy: first-100 food 15/128 and splits 4/49, despite already seeing 571/625 cells as a team. Generic extra exploration does not explain everything.
- Slithery: first-100 food 167/449, splits 63/206 and turns 1,064/3,982. The deficit recurs but is not identical in every replay.
- Trauma: we trailed first-100 food 3/100 and splits 3/49, yet won with maximum thirteen against nine. An aggressive opening is useful only if converted into a surviving scorer.
- Queen: we won with one dragon of length five against 55 dragons with maximum three and total 135. Forcing all productive pocket dragons to roam, or filling every slot regardless of length, can destroy a strength.

In [459157, the high-ranked Trauma reference](https://game.battlecode.au/visualiser?match=459157), 171 of 188 late worker-near-grower turns on the winning side involved a collector that had already acted. Our earlier higher-ID-only feeding could not use that class of opportunity. These are hindsight counts, not 171 proven missed donations. Sonar is a substantial constraint: in the original Trophy's first 100 rounds only 434 outgoing rays reached another ally, while 446 hit self and 1,191 hit kelp.

## Lozer: what aggression actually accomplished

Downloaded and independently reconstructed [518063 Trophy](https://game.battlecode.au/visualiser?match=518063), [518061 Slithery](https://game.battlecode.au/visualiser?match=518061), [518056 Devil](https://game.battlecode.au/visualiser?match=518056) and the [518062 Trauma win](https://game.battlecode.au/visualiser?match=518062). Borgor is side B in these games.

| Map | Our first-100 food / Lozer | Our splits / Lozer | What happened |
|---|---:|---:|---|
| Trophy | 99 / 56 | 33 / 23 | We led: 29 dragons and total eighty at round 100 versus nineteen and forty-six. At round 200 we had nineteen/forty-three versus thirty-one/seventy-five. We were eliminated by round 289. |
| Slithery | 66 / 665 | 19 / 243 | Lozer had 47 dragons by round 100 against thirteen. Both sides retained a long opening coil; Lozer also built workers across the map. Final maximum twelve versus our nine. |
| Devil | 3 / 141 | 3 / 55 | Both teams split their three starting length-four dragons at round zero. The divergence came afterwards: we had three length-two survivors at round 100; Lozer had 21 dragons and total 52. |
| Trauma | 6 / 5 | 4 / 3 | Similar restrained openings. We eventually won with maximum thirteen against eight, despite lower whole-game food (111/141). |

This contradicts the blanket diagnosis that we always lose because we split too late. In Trophy we started ahead, then lost productive territory and could not rebuild. In Devil the initial splitting decision was already equally prompt. In Slithery, distributing workers while retaining the coil was a major distinction.

Lozer requested **no sprint steps** in the first 100 rounds of these four games. Its pressure did not require expensive movement. We spent twelve extra steps in Trophy and 23 in Slithery. That is descriptive, not proof that every sprint was wasteful; escaping death can justify the price. V7 prices affordable enemy threats more accurately rather than globally disabling sprint escape.

Food counts also include recycling. In Slithery's first 100 rounds, 284 of Lozer's 665 collections were traced to its own corpses, with another eleven from ours. It incurred 140 self-collision deaths in that phase. The remaining roughly 370 collections were not traced to corpses, still far above our roughly 56. In Devil, 48 of its first 141 collections came from its own corpses and three from ours; the non-corpse deficit remains large. Copying its death rate without its replenishment and pickup behavior would be harmful. The replays do not prove all those self-collisions were intentional.

Implemented response: bounded worker food urgency, local territory scoring, earlier informed scouting, threat affordability, selective standoffs/interceptions, and **replacement splitting only on productive contested fronts in rounds 225–324, below 24 allies**. Protected growers and quiet farms retain their existing timing. Endgame agreements concentrate resources without using a blind sacrifice schedule.

## Prioritized fixes and preservation constraints

| Target | Actual change | What remains protected / unresolved |
|---|---|---|
| Retreat from threats the enemy cannot afford | Certify full visible enemy bodies; include food-funded sprint steps; uncertain bodies retain conservative capacity. Qualified short workers may hold room against expensive heads. | Direct body/wall/neck collisions stay forbidden. Cheap enemy heads and split tails still invalidate the standoff. |
| Postponing accessible food and abandoning useful fronts | Remove double-counted initial/future attraction and redundant sharing penalty; add bounded food urgency and separate enemy/own arrival estimates. Replace depleted productive workers in the limited middle phase. | Local legal food is not automatically safe. Grower food value is not lowered. There is no blind global center charge. |
| Fragmented late length with no operational coordination | Local circulation areas, offers, exact donor/collector coordinates, post-action sonar checks, acknowledgement before death, one/two-turn pickup priority and renewed soft reservation. | Missed/stale messages never authorize death. Population remains at least three. Enemy interference and unseen traffic cannot be ruled out globally. |
| Repeated large-child rescue consumes the economy | Early repeated-lineage balanced rescue, only favoring both halves when their routes are checked; limited partly seen fertile-opening division. | Keep isolated maximum-length coil saves and late survival. An unseen child is explicitly an opening risk, not certified safe. |
| One-size exploration and choke blindness | Confidence from seen walls/portals/region openness, useful-frontier targets, honest capped-flood uncertainty, shallow exact enemy replies in critical local cases. | No unsupported named-map classification; no fictional unknown-portal links; no broad minimax claim. |

## Concrete faults found in the intervening implementation and our verification

Claude v6 encoded rally information using values that overlapped legitimate length-two status messages, and discarded length-three status reports. V7 uses disjoint check-value tags, with ordinary status for both lengths. Native 5/8 and sandbox 3/8 against v4.1 were small local results, not a 62.5% ranked win rate. Prior experiments changed several features together and reused small fixed-seed comparisons; they did not isolate causal contributions.

We independently reproduced the food-postponement scoring issue before retaining the correction. Existing portal-aware paths and flood-fill room checks already existed in v4.1; copying another pathfinder would not fix the diagnosis. Queue-order ownership of equal-distance tiles is not valid territory scoring; V7 computes separate arrival distances and records ties.

Our first actual-engine feeding fixture exposed an additional design defect that unit checks missed. With ordinary one-step movement, some donor/collector pairs preserve a relative checkerboard phase in which a one-turn-only agreement cannot put the collector adjacent after moving. V7 now checks one **or two future ordinary turns**, including the real final-round limit. It avoids spending a sprint segment to collect a single donor pearl. The engine fixtures confirm agreement, the donor's own-neck death and actual recipient growth for both ID orders. This is local mechanical verification, not evidence that feeding will occur frequently on every competitive map.

We also fixed partial visible ally counts wrongly overriding a prior larger station report, duplicate acknowledgement accumulation, and multi-ray coordination incorrectly sharing a supposedly single-ray portal probe. The donor's final route check excludes the future neck as an exit. These are correctness fixes with focused checks, not tuning to win a particular seed.

## WakeUpLeviathan suggestions: disposition

- **Territory estimates:** implemented locally with explicit ties, body blocking, known portal connections and strict work limits. It is ordinary-move reach, separate from sprint danger.
- **Adaptive food urgency:** implemented, capped at 35% for workers. No invented starvation mechanic and no reduction of scorer food rewards.
- **Flood fill/chokes:** retain existing body-aware room search, stop treating a capped search as proof of enclosure, and check whether a nearby enemy's legal reply closes the sole exit. Do not duplicate the same room code under a new name.
- **Portal paths:** existing mapped portals already cost one move. Preserve them and improve protocol interaction; an unknown-portal self-loop cannot predict a useful destination.
- **Critical minimax:** implemented a bounded enemy-reply/best-escape assessment for one fully seen opponent, including sprint payment. It is not full multiplayer minimax or an exact solver.
- **Late coordination:** implemented explicit local areas and agreements for both ID orders, with soft approaches and actual pickup priority. Fixed-ID suicide schedules were rejected because a recipient may have moved, died or become unsafe.
- **MCTS:** not included. In this partial-information game, adding large random future searches without a validated opponent/world model would consume budget without an established benefit. The targeted reply checks address a demonstrated failure with bounded cost.

## What this review does not establish

No source change is claimed to be the globally best strategy. Replay symptoms can have several causes, and combined implementation cannot attribute a win to one feature. The controlled opponent is frozen v4.1, not Lozer's unavailable source. Wins on a small fixed-seed set are regression evidence, not an estimate of future Elo. Hidden portal arrivals, unseen tails, enemy moves between agreements and public sonar exposure remain limitations. Final recommendation and exact results belong to [VALIDATION](VALIDATION.md), not selectively chosen preliminary wins.

Targeted rule verification used [Sonar](https://game.battlecode.au/docs/sonar), [Execution Order](https://game.battlecode.au/docs/execution-order), the local master and the installed 1.2.2 engine. No complete-documentation reread was performed.
