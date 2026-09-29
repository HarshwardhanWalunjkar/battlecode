# Game plan — Jormungandr-v1

This is the complete strategy implemented in `bot/`. Jormungandr-v1 builds on the submitted, frozen abyss-v7; that release remains unchanged. The new name starts a new release series, not a replacement of the old archive. See the [review and suggestion decisions](evaluations/v8-review/REPORT.md), [validation](evaluations/v8-review/VALIDATION.md), [work tracker](current%20planned%20work.md) and [version history](versions/README.md). This describes implemented choices, not a claim that every choice is optimal.

For rules, first read [MASTER](documentation/MASTER.md). Read other rule pages only for a specific unresolved question, unless a complete documentation review is explicitly requested.

Quick references:

- [Roles and winning](#1-winning-and-the-division-of-work) · [Memory](#2-memory-and-uncertain-information) · [Move priorities](#3-decision-order-and-movement)
- [Food](#4-food-and-growth) · [Exploration and maps](#5-exploration-map-structure-and-territory) · [Aggression and traps](#6-opponents-aggression-and-choke-points)
- [Ordinary splitting](#7-ordinary-splitting-and-replacing-workers) · [Rescue splits](#8-rescue-splits-and-long-starting-bodies) · [Sonar](#9-sonar-and-message-reliability)
- [Coordinated feeding](#10-local-collection-areas-and-coordinated-feeding) · [Portals](#11-portals-and-last-resorts) · [Exact preferences and endgame](#12-preference-scales-limits-and-endgame-details)
- [Human opponents and suggestions](#13-human-opponents-exposure-and-future-suggestions) · [Submissions](#14-version-and-submission-discipline)

## 1. Winning and the division of work

We prioritize rank and rating. In a game, eliminating the other team wins. Otherwise, the longest surviving dragon decides the winner after round 499. Total surviving length breaks a tie in longest length. A large population is a means of gathering food, controlling routes and supporting long dragons; it is not the final score.

All dragons run the same program separately. Their memories are separate. They see only the wrapped 7-by-7 square around their head and messages that actually reach them. There is no all-seeing captain or automatically shared map. Sonar can now pass small pieces of navigation knowledge, subject to the limits in section 9.

We call protected dragons **growers**: their job is to retain length and become possible winners. Every dragon of length seven or more is a grower from the beginning. A rescue-protected dragon remains a grower at length four or more. From round 200, length six also qualifies. From that round, if at least four allies are alive, IDs with remainder equal to their team number after division by six also qualify: remainder zero for A, one for B. This selects some smaller future growers without pretending to know the entire team's lengths.

Growers do not split normally, volunteer for head trades or take optional blind portals. They pay a higher preference penalty for sprinting. Emergencies can still require a rescue split. From round 425, a small grower may feed a demonstrably larger collector under the full agreement rules below.

Workers gather food, make safe children, explore and contest useful space. Emergency rescues can use all 64 slots, also respecting the actual limit reported by the map. Ordinary growth leaves two slots free when the map limit is at least sixteen: at the usual limit, it stops at 62 living dragons. This keeps a full worker population from blocking a valuable rescue. Emergency rescues can consume that reserve; it is not a guarantee that two slots will always remain available. There is no 48-dragon cap and no hard cap of ten on a grower's length.

## 2. Memory and uncertain information

We remember observed walls and portal pairs throughout the game. Food and bodies are updated when seen. Another dragon's recorded body blocks planning for the current round and the following round; older sightings are no longer certain obstacles. Food remains a possible destination for at most 15 rounds after its last observation, with decreasing appeal.

We keep our predicted body and compare it with the next observed head, visible segments and reported length. If inconsistent, we rebuild the chain from visible segments that point towards the head. A partial chain is not a complete body, and its last known segment is not assumed to be the tail. As we move, more of our body becomes known.

After a blind portal crossing, prepend the new head reported by the engine to the previous body and trim using the actual length. A pearl on arrival retains the extra segment and updates the last-food time. This preserves the neck on the other side of the portal. Blind reversal opposite the heading is forbidden even for a newborn with an unseen neck. After splitting, visible child segments remain allied obstacles until a newer observation replaces them.

The moves we commit to normally require every destination to be visible now, including each sprint step. Imagined later turns can use remembered terrain, and are replanned on the next real turn. Blind portal escapes and partial-body rescue splits are explicitly uncertain exceptions. We do not infer unknown terrain from symmetry or assume the center contains food. Shared terrain has a separate navigation record: it can suggest where to travel, but cannot make a move legal or prove that a future escape is safe. Direct observation overrides every report.

A visible enemy head and its visible segments give a minimum length. We call it an exact length only when the entire chain can be followed and every possible connection into its last segment is visible and mapped. A chain disappearing through a portal or outside vision is not certified complete.

## 3. Decision order and movement

We enumerate legal moves of one, two and three steps. Every step checks its edge, portal destination, visible occupancy, our body and food. A current tail square remains occupied during collision checking. Extra steps cost length, and each must be affordable at the moment it is taken; food earlier in the same sprint can finance a later step.

Routes receive scores for food, room, exits, threats, exploration, coordinated approaches and recent repetition. If a reasonably sheltered route exists—an exit, at least six reachable tiles and no one-step head attack—ordinary exposed routes are removed. The narrowly defined worker standoff in section 6 is the exception. We retain at most 14 routes and examine up to ten further turns for each, keeping seven promising continuations at each depth. Those future turns normally move one step. Search stops at the actual end of the game.

Other dragons' bodies stay fixed during this ordinary search. Nearby heads add danger. Remembered food can be consumed once inside an imagined route; future spawning is an attraction, not invented food already eaten. This is a bounded planning aid, not an exact prediction of everybody's next ten moves.

If a checked complete-body branch runs out of continuations through known terrain, it is marked as failing. When another searched branch survives, failing branches are discarded. Reaching unknown terrain does not prove a trap. The surviving continuation's attraction replaces the initial attraction; counting both could reward staying near a pearl instead of eating it.

The following special decisions take priority. A promised meal is checked before ordinary routes are shortened to the best fourteen, so an agreed approach cannot disappear just because the ordinary ranking omitted it:

1. Collect, or safely approach, food from a previously agreed donation when it has actually appeared.
2. Carry out a received donation agreement after rechecking all conditions.
3. Take a qualifying valuable head trade or an interception protecting a grower.
4. Make the limited fertile-opening division described in section 8.
5. If trapped, seek a checked rescue split, then the staged junction rescue described below when ordinary moves fail; otherwise consider a safe ordinary split.
6. If a partial body has no usable continuation, consider its uncertain rescue fallback.
7. If not trapped, accept a worker's feeding offer using a safe current move and checked future collection.
8. Consider an allowed blind portal escape, then a last-resort head attack if no movement remains.
9. Choose the best remaining route. With none, attempt a direction avoiding known allied heads; death may be unavoidable.

A route counts as trapped if none exists, the best route exhausts its known continuations, it has no exits and at most two reachable squares, or it exposes a one-step attack without qualifying for a standoff. We execute only the selected current action, then plan again next turn.

## 4. Food and growth

One confirmed pearl normally earns 12 preference points. These are internal comparisons, not game points. Before round 200, a non-grower of length at most three gets an extra 20% when team population is below 32. A non-grower of length at most four gets another 15% after at least 12 foodless rounds. Before round 325, a worker of length at most three gets 15% more when any enemy head is visible, encouraging food that restores an escape reserve. The total increase is capped at 35%, so the maximum is 16.2. A grower's food value is never reduced by this adjustment. Going without food does not itself kill a dragon; this extra urgency supports worker growth.

We consider up to twelve personally observed nearby food destinations, plus up to three separate fresh food reports from allies. Ready food is discounted by its observation age: divide its value by `1 + 0.09 × age`. A visible spawn due within six rounds starts at 0.45, divided by `1 + 0.15 × wait`. Destinations are shortlisted by travel distance divided by this value. Distances follow mapped walls, wrapping and portals, although this broad food shortlist does not itself prove a body-free route.

A destination's attraction is nine times its remaining value, divided by `1 + 0.38 × distance + 0.6 × extra waiting`. Already imagined-eaten food no longer attracts us. If our imagined body would cover a spawn exactly at its next round boundary, that spawn's attraction is cut to one tenth. Bodies block spawning, even when their owner wants that food.

For the first eight rounds after observing an available pearl, a smaller dragon may give a larger ally an advantage. That ally must have a report or visible size from this or the previous round, length at least six and larger than ours, a clear known route within four moves, and be at most one move farther from the food than we are. There must be enough game time left, and no known enemy approach within two steps of the pearl. We reduce our reward for that pearl to one fifth; we do not forbid taking it when safety requires it. The extra duplicate sharing penalty from the old code is removed. After eight unclaimed rounds, this ordinary preference expires.

A current feeding acknowledgement can reserve its exact meal more strongly: other workers reduce that pearl's appeal to one tenth when the named collector's square still contains an allied head and no enemy is within two known steps. The reservation lasts at most one round unless renewed during a two-turn pickup. It is a soft preference, not a physical lock or a broadcast received by everyone.

## 5. Exploration, map structure and territory

We keep an exploration target for up to 24 rounds, replacing it when reached, expired or unreachable. Growers usually wait 20 foodless rounds; ordinary workers wait eight. Before round 225, workers whose ID remainder after division by three equals their team number are early scouts and normally wait four. Nearby worthwhile food within four moves delays departure until eight foodless rounds for early scouts, twenty for others.

An early scout, or any other non-grower with no worthwhile food within four known navigation moves, can begin exploring sooner before round 160 when there are at least three allies, it has personally seen less than three quarters of the map, and its map sample is small or less than 30% of observed edges are walls. This is a modest opening exception, not permission to ignore food or walk into danger.

Targets are scored using known path distance and what the dragon itself has seen. Prefer a frontier with unknown edges; otherwise an unvisited portal approach; otherwise a square not visited for at least 30 rounds. Give value to how many new squares the next 7-by-7 view might reveal, to branching ordinary routes, and to avoiding fresh allied heads within three wrapped steps. Known portal crossings count as one move, just like ordinary edges. A shared complete portal pair may suggest a distant navigation shortcut. It remains untrusted for actual movement or escape until seen directly; an unpaired portal never creates a fictional connection.

After round 100, a dragon that has not directly seen any enemy body or head for sixty rounds treats its local area as quiet. This is not proof that the teams are separated. In this mode workers use four foodless rounds of patience and nearby worthwhile food delays their departure until eight foodless rounds. Growers retain their ordinary twenty-round patience and twenty-round nearby-food allowance. Seeing any enemy immediately restores the ordinary behavior. Children start their own sixty-round clock at birth. Silence does not lower danger penalties, relax wall/body/portal checks, increase sprint spending, or change splitting thresholds.

We measure the fraction of confirmed edges that are walls or portals. The map profile gains confidence up to 128 observed edges. We also compare the middle half of each coordinate axis with the outer part of the map, giving that comparison full weight only after at least 64 observed edges in each region. A small balanced starting count prevents a handful of observations from creating an extreme estimate. We favor the more open observed region; these are confidence-weighted measurements, not probabilities that a named map type is correct. Wrapped boundaries are ordinary connected space, not inherently dangerous borders.

When enemy heads are visible, we separately estimate how many ordinary moves they and a candidate of ours need to reach local squares. Equal arrival times count as contested rather than being awarded to whichever search ran first. Enemy arrivals are considered out to twelve moves. Our estimate looks out to ten moves and at most 128 squares, blocking known bodies. Nearer squares count more: divide their value by `1 + 0.15 × distance`. A modest extra score rewards space reached first and, less strongly, tied space. Sprint threats are checked separately. This is a local estimate from partial information, not ownership of the whole board.

We do not order all workers to the geometric center. Trophy has shown both weak openings and an opening lead followed by collapse. Productive territory, affordable threats and replacing lost workers matter more than a fixed center instruction.

### Recognizing a supplied map safely

Each of the ten supplied maps has its own generated header under `bot/maps/`. It contains the exact full wall/portal layout, every spawn range, every starting body in ID order, graph connections and a resource guide. These come from the official local map files, checked against the supplied images; screenshots are not used to guess pearl timers. Regenerating the catalogue is an explicit development step, never an online action by a dragon.

An original round-zero starter requires exactly one matching entry: dimensions, ordered starting ID, head, length, facing and every visible part of its own starting body must agree. Team colors may be swapped, so color is not the discriminator. All observed canonical edges must match, including exact portal IDs. Every observed tile must agree about whether spawning is enabled; a reported remaining timer greater than the recorded maximum rejects the entry. A smaller remaining timer does not contradict the minimum gap because an existing countdown has already ticked down.

A candidate becomes active only after at least 64 distinct canonical edges and eight landmarks agree. A landmark is a wall/portal edge or a seen tile with spawning disabled. Dimensions and a coincidental head coordinate alone never activate it. A child needs a received map proposal instead of an original-start match, then the same local evidence and contradiction checks. An uninformed child keeps general mode. A fresh positive report can supply a proposal later; conflicting proposals reject map use.

All accumulated direct evidence is checked every turn. Any mismatch permanently disables the catalogue for that dragon, clears its catalogue route target, and makes future birth messages carry fallback. Positive messages cannot undo a rejection. A fallback report is also relayed on map-information turns. Unknown new dimensions or a changed original-start signature use the general strategy immediately. Partial vision cannot prove that the unseen portion of a map is unchanged: catalogue knowledge is therefore restricted to navigation preferences. It never fills actual food, body, sight or collision records, never authorizes a blind portal move, and never certifies a rescue or an enemy location. Initial enemy bodies are stored as map data, not treated as living enemies later.

### Routing toward each map's productive regions

A tile's expected rate is two divided by its minimum plus maximum spawn gap; disabled tiles contribute zero. This is an average, not a promised pearl or next spawn time. For each possible approach square, sum these rates over squares within three graph steps, dividing each contribution by one plus its distance. Divide by the best such sum on that map to obtain a relative resource value from zero to one. Walls and portals determine graph steps. Thus the guide follows reachable farms and openings rather than assuming the geometric center is best.

Use this guide when the map is confirmed. A grower keeps its normal strategy for its first twenty foodless rounds. Any dragon with a worthwhile known food target within four moves also waits twenty foodless rounds before this wider search. A worker already approaching a collector, or any pending pickup, takes priority. Consider reachable approach squares with at least two unblocked outgoing connections and enough remaining turns to reach them. Score each as `12 × log(1 + 16 × relative resource value)`, minus `0.45 × travel steps`, minus the map's crowd penalty for each fresh ally within four wrapped steps. A stable ID-and-square tie preference below one point spreads close choices. Known recent bodies block the route search.

Keep the chosen approach for the map's holding period, unless reached, newly blocked or unreachable. Movement preferences reward progress by the map's pull below, capped at six steps in either direction. This replaces the ordinary exploration pull for that turn, avoiding contradictory travel goals. Actual visible pearls, body safety, rescue and combat checks still apply. These travel preferences never finance a sprint with hypothetical food, change split limits or lower danger merely because a map was recognized.

| Map | Practical approach | Pull per step | Keep target | Crowd penalty |
|---|---|---:|---:|---:|
| Autarky | Reach connected boxed farms; preserve productive long starters. | 1.5 | 20 rounds | 4 |
| Default | Seek the denser food grid through openings and known portal connections. | 2.0 | 24 | 3 |
| Devil | Reach fast food bands through corridor gaps; distribute workers between farms. | 1.8 | 20 | 4 |
| Prisoners Dilemma | Rescue blocked starters first; guide spare workers to reachable central and outer supplies. The guide is long-term; visible countdowns decide immediate food. | 1.8 | 16 | 4 |
| Portals | Use the full paired graph to approach fast fields; unseen exit occupants remain unknown. | 2.0 | 16 | 4 |
| Queen of Spades | Cover productive pockets while retaining successful growers. | 1.6 | 24 | 4 |
| Schooltime | Spread across separate resource bands and their entrances. | 2.2 | 28 | 3 |
| Slithery Fight | Preserve coil/junction rescues, then distribute spare workers across connected fast farms. | 1.8 | 16 | 5 |
| Trauma | Follow maze connections toward productive patches; do not abandon a working farm. | 1.5 | 24 | 4 |
| Trophy | Leave barren corners through wall gaps for richer interior approaches. | 2.2 | 20 | 4 |

No map imposes Thor's proposed four-to-ten population caps or length-eight ordinary split threshold. A threshold above our grower threshold would stop ordinary reproduction. No profile spends more sprint length simply because the map is large, or assumes an unseen portal is safe. These are guarded map-specific foraging strategies, not a claim of solved maps or guaranteed sandbox wins.

## 6. Opponents, aggression and choke points

Head collisions kill both dragons regardless of their lengths. More length does not win a collision. A normal worker of length at most four may initiate a head trade when at least four allies are alive and the visible enemy is at least six long and at least two longer than us. Prefer the largest verified target. Growers never volunteer for this trade. A smaller additional exception allows a non-grower of length at most three to intercept an enemy whose currently visible head has a direct move into a fresh allied head of length at least seven. Population must still be at least four. This is protection of a valuable ally, not general permission for equal trades.

Ordinary threat checks consider visible enemy heads and visible potential split tails from dragons with at least four visible segments. A split tail could create a cheap attacker immediately. For a fully known enemy, sprint threats respect its actual length and visible pearls. A length-two enemy without a first-step pearl cannot finance a second step. For an uncertain body, we keep the conservative possibility of a three-step attack. Known bodies obstruct those routes. We do not extrapolate a visible fragment into an invented exact enemy length.

A worker may sometimes hold a valuable square instead of always retreating from a larger head. This requires a non-grower of length at most three, at least six allies, at least two exits and eight reachable squares. Every one-step threat into that square must be an enemy head with visible length at least six and at least two greater than ours. A threatening tail or smaller head cancels the exception. Its one-step danger penalty becomes 12 instead of 130, and the sheltered-route filter keeps it available. The enemy can still choose the collision; we are accepting an expensive exchange for them, not guaranteeing survival.

We count connected room with bodies blocking it. Stopping because the search reached its size limit never proves enclosure. Few exits, small fully known pockets and future body positions still matter. Food in a pocket can justify entry only when a checked rescue would retain a dragon strictly longer than the original one: at least three new segments must be collected to cover the two retained in front.

For late or very restricted choices we also examine actual enemy replies. This runs whenever exactly one enemy head is visible, including the opening, on at most six choices. The existing late/restricted-choice triggers remain, but the check still requires one fully observed opponent. It requires exactly one visible enemy head within three wrapped steps, a certified complete enemy body no longer than sixteen, and our own complete body. We try legal enemy moves of one to three steps, updating that enemy's body, pearl consumption and sprint costs. Other bodies remain fixed. At most 64 reply positions are processed, and the time limit can stop it sooner. The worst reply adds a penalty for killing our head or closing our exits. Thus a length-two opponent can be recognized as unable to sprint into us but still able to close our only escape square.

This is a short enemy-reply check, not a complete game solver. It does not model all enemies, future splits or unseen routes. Unknown outgoing edges prevent declaring a fully proven enclosure. We did not add a costly random-simulation search whose reliability or budget we could not establish.

## 7. Ordinary splitting and replacing workers

Before round 225, a non-grower of length four, five or six can create a length-two child. The ordinary-growth reserve described in section 1 must be respected, the entire body must be known, and at least one round must have passed since that parent's last split. The child must have at least one exit, eight reachable squares, no direct enemy head attack and a checked two-move continuation with the parent's body fixed. The shortened parent also needs a checked two-move continuation, and its current head must avoid a direct attack.

There is one extra opening/front restriction: at length four, with at least four allies and a visible enemy head within six wrapped steps, hold the split unless the rear has an immediately visible unoccupied pearl next to it. Growing to five allows a three-segment parent and a two-segment child; that parent can afford a second emergency move. The productive replacement exception below also permits length-four splitting. This avoids automatically turning every threatened four-segment worker into two dragons that cannot sprint. All the ordinary safety checks still apply.

Once population is at least eight, avoid splitting when three allied heads are within four wrapped steps of the child's head; also avoid it with two such heads unless at least two worthwhile food destinations are within six moves of the parent. We do not fill every slot when the children would choke each other.

There is a limited replacement period in rounds 225–324. A non-grower of length four or five can still split if population is below 24, an enemy head is visible, at least two worthwhile food destinations are within six moves, and the child has at least two exits. All the usual body, room, crowding and parent/child continuation checks still apply. This helps a productive contested front replace losses. Quiet farms and protected scorers retain length. From round 325 there are no ordinary splits; emergency rescues remain possible.

## 8. Rescue splits and long starting bodies

When trapped with a complete body, at least four segments and a free slot, try several cuts: leave two in front, leave two in back, leave three or four at either end, and roughly halve the body. Duplicate or illegal sizes are skipped. Each half is simulated with the other half still occupying its squares. The child needs up to six checked moves because it acts this round; the parent needs up to six beginning next round. Horizons shrink near round 499. Unknown portal arrivals do not establish a safe rescue.

Normally, a checked child containing all but two segments is accepted promptly to preserve the greatest length. Otherwise choose the cut with the longest verified surviving half, breaking ties in favor of both halves surviving and better room. The parent is allowed to remain still on the split turn. At the very end, that can itself save a score; we do not invent extra turns beyond the game.

For a recently repeated rescue, before round 180, population below sixteen and length at least eight, try a balanced cut first. Prefer it immediately only if both halves have checked continuations. This can create a useful working branch instead of endlessly reversing the same long body and leaving two-segment remnants. An isolated long rescue and late score preservation keep the length-first policy.

If every retained ordinary route fails and no ordinary rescue cut passes, a complete dragon of length four to sixteen can try a **staged junction rescue** before round 490. At least two slots must be free. Try rear sizes two through the smaller of eight or length minus two. For each resulting half, consider up to three ordinary moves using food visible now, then one further split. That later split tries a rear of all but two, roughly half, or two segments. Its rear must have six checked ordinary continuation moves. Every abandoned half stays blocked in this proof; it never assumes that an ally will move, die or leave food. Already eaten pearls cannot be counted again.

This search retains four possible continuations at each stage, examines at most 64 intermediate bodies across the first-cut choices, and stops by its time allowance. It favors the first cut with the greatest sum of independently proven surviving descendant lengths; smaller rear sizes win ties. Both immediate heads must avoid a direct attack. It only justifies an escape possibility: all descendants still recheck their own view on their actual turns. They may obstruct one another later. Small trapped remnants can still die. Slithery's short starters need this kind of cut because their own middle covers the junction leading into open space. The existing 25-to-2-plus-23 coil rescue is checked first and retains priority.

One narrow opening division can use a partly seen starting body. It requires round zero, birth in round zero, no received rescue history, length at least eight, at least four known head segments, at most eight allies and a free slot. The head has at most two ordinary open edges, and at least two currently seen squares have food attempts due within four rounds. The head and a checked two-step parent route through current vision must avoid direct enemy attacks. Give the rear child half the length rounded down and keep the rest in front. The parent is checked; the unseen child is not certified safe. This is a deliberate, limited opening risk.

Otherwise a partial body is split only when no legal ordinary move remains or the retained search exhausts its continuations. Require at least four total segments, two known head segments and a slot. Normally retain two in front and give the rest to the child. For a repeated rescue before round 180 with population below sixteen and length at least eight, use a balanced cut instead. Again, the unseen tail is uncertain.

A rescue from original length seven or more protects a surviving parent of at least four. A rear child of at least four receives its expected length, recent rescue count and role in a special sonar message. Only a newborn in that exact round with matching length accepts it. Counts stop at seven and clear after both birth and the last own split are at least 24 rounds old. Opening children and children of at most six from an early repeated rescue are released to ordinary worker rules; other eligible children remain protected. Ordinary size-based grower rules still apply, and sonar delivery is never guaranteed. No merge action exists.

## 9. Sonar and message reliability

Sonar travels after the action along open edges and through portals, stopping at the first body or kelp. It is not a broadcast. The direction opposite the new heading starts from the actual tail and points away from its preceding segment. Our reach check uses that post-action body and currently visible, mapped ray squares, for at most width plus height steps. A ray we cannot certify is not used to justify an agreement.

When another ally exists or we split, and our post-action body is known exactly, normally send our new head, length, ID, team and round in all four directions. Reject invalid coordinates, future messages, status older than two rounds, wrong-team or invalid check values, our own status and visible contradictions. A visible body count remains a minimum. An exact size report stays exact only while its head position matches and its author has not acted again: this round, or last round for a higher-ID ally.

Length zero encodes a portal traffic warning. The old length-one rescue-role format is still understood, but new split messages use a separate birth type carrying both role and map mode. Collector stations, offers and acknowledgements use separate check-value tags. Length-two and length-three dragons remain normal status reports, fixing Claude v6's ambiguity. An acknowledgement carries the donor ID, exact food square, collector's post-move square and round. Repeated copies are merged. These checks reject mistakes and stale messages; they are not cryptographic protection against an adversary reverse-engineering the protocol.

A newly received station message must satisfy the ordinary two-round arrival limit, then can remain locally useful for twelve rounds after it was sent. Offers and acknowledgements last one round. Visible evidence and body/route checks are still required before a sacrifice. A worker seeing fewer segments does not mistake that lower bound for proof the collector shrank; a fresh exact shorter report can cancel the assignment.

Portal probes use exactly one direction, so a subsequent unlabelled echo can be attributed to that ray. The opposite-heading tail ray cannot probe a portal beside the head. A body/head echo marks the portal busy through the following round; a miss never proves safety. When not probing, two alternating rays can carry a nearby portal's warning and two carry status. A blind crossing warns in all four directions. Coordination turns use multiple rays and therefore never also claim a single-ray probe. When a collector has no specific agreement, two alternating directions advertise its station and the other two retain ordinary traffic/status behavior. Accepted agreements and targeted offers use all four rays. Combined birth messages take all four rays after any split, including a partial-body rescue. That turn never claims a single-ray portal probe.

### Teaching children and sharing useful information

The old tail becomes the child's head for every legal split size. Changing the cut does not relocate that head. Sonar can reach any child segment after the split; head alignment is unnecessary. We therefore do not alter a safe split just to obtain a teaching opportunity, and never perform an unsafe ordinary split for communication.

After every split, send the same combined birth packet in all four directions. It carries the child length, map mode, rescue count, protect/release flags, current round, catalogue revision, parent identifier fragment and team, plus an error check. Only a newborn in that exact round with matching length, revision and team accepts it. Map codes 1–10 propose one catalogue entry; zero means unknown; fifteen means permanently use the general strategy. The child independently checks its own observed terrain before accepting a positive map mode.

A bent body can make every ray miss the child. Engine probes confirm this physical limit. We preserve essential rescue cuts rather than forcing a worse cut for communication. A child without a birth message uses the general strategy and never guesses from its incidental head position. It can later validate a positive map report from a contact. A child taught permanent fallback cannot be switched back by a positive report. Rescue role and map identity share one packet so they cannot overwrite each other on the same ray.

On the parent's following turn, try one optional information packet along a ray whose first recipient is a currently visible ally. It uses whichever ally the ray actually reaches. Ordinary information turns occur when round plus dragon ID is odd, with at most one ray replacing status. When round plus ID leaves remainder one after division by eight, send the known map mode or permanent fallback with the catalogue revision instead. Feeding messages and a single-direction portal probe retain priority. These map reports must be from this or the preceding round and cannot bypass the recipient's own map checks. This is contact-based propagation, not guaranteed whole-team delivery.

Information is chosen in this order, with some alternation to prevent food reports crowding out terrain:

- **Food:** consider currently seen, unoccupied pearls or observed spawns due in one to six rounds, excluding squares eaten or occupied by our just-chosen action. Score ready pearls as twelve, future ones as six, minus wrapped distance from the intended recipient's visible location. Food gets first choice on teaching turns and when round plus ID leaves remainder one after division by four; otherwise it is a fallback after portal/terrain information.
- **Portal pair:** send both confirmed edge positions and the actual portal ID, supporting IDs zero through 65,535. Favor a pair near the recipient: twenty minus distance to its nearer end. Partial pairs are never advertised as complete.
- **Terrain patch:** send six edges from a three-by-two patch: four horizontal connections and the first two vertical connections. Each is open, wall, or unknown; portal connections are omitted because a portal type alone does not identify its exit. Only directly learned edges are sent. Require at least three known edges. Prefer more known edges, edges outside a wrapped distance of three from the recipient, nearby patches and recent observations. The exact score is known-edge count plus twice the distant-edge count, minus a quarter point per travel step and 0.02 per observation-age round capped at fifty. This is a bandwidth preference, not a sight guarantee.

Do not resend the same food square, patch anchor, or portal ID within eight turns, except that teaching may repeat a food target. At most sixteen incoming information packets are processed per turn. Terrain and food envelopes carry coordinates, sender ID, team, round and a fourteen-bit check value; their type is distinguished from status/feeding. Portal packets use two thirteen-bit edge positions, a sixteen-bit ID, team, a seven-bit round counter and the same check size. Its round counter wraps every 128 rounds; the content is static terrain, never a promise about current occupancy. Accept only current/previous-round information by the relevant counter. Bad coordinates, endpoints with different orientations, same-endpoint pairs, wrong teams, malformed types and bad check values are ignored.

A checksum detects accidental corruption; it is not secret authentication. Therefore reports never set a tile's actual food, body, observed status or verified terrain. A terrain hint fills only an unknown hint. Known direct edges win, and a shared portal cannot replace a directly confirmed pair or contradict an observed wall/portal ID. Navigation searches can use these hints; actual moves, territory threat checks, body reconstruction and rescue proofs use only direct terrain. We do not forward received reports as if we saw them ourselves.

Store at most sixteen food reports, each for at most eight rounds. Any observation of its square at or after the report, including seeing it empty, overrides it. Add at most the first three usable distinct reports to food targets, each with value 0.55 divided by one plus 0.2 times age. Use its reported due time for waiting. This reward guides travel only: it cannot count as eaten food, finance a sprint, or certify growth.

These messages help contacts exchange useful knowledge. They do not create a complete global map on a schedule; rays can miss, hit self, kelp or opponents, and useful allies may never meet.

## 10. Local collection areas and coordinated feeding

### Choosing an area and approaching it

From round 300, a dragon with length at least six, a complete body and at least four living allies can advertise a collection area. Its head must be more than three known enemy steps away. It yields local leadership to a fresh larger ally, or an equal-length lower-ID ally, within six wrapped steps.

The area is a place to circulate, not a square where a dragon can stand still. Search within four known moves for a square observed in the last eight rounds, with at least three ordinary open edges, no adjacent portal edge and enemy distance greater than four. It cannot lie on our body except at the head. Require room for at least twelve squares or our length plus four, whichever is greater. Score space up to forty squares minus twice travel distance, plus twice the value of nearby food targets within four moves, capped at six extra points. Renew after twenty rounds, after moving more than six wrapped steps away, or when eligibility, the current head threat or the station threat invalidates it. Only a fresh offer from a currently visible matching allied head within four wrapped steps activates the collector’s return preference, for eight rounds. While active, it adds 1.5 points per step back toward this area, capped at six steps, with no pull within two route steps. Without a nearby offering worker, an empty meeting point does not pull a collector away from farming. This allows circulation rather than occupying a particular square; ordinary food rewards and danger checks still win over a small positioning preference. This is a local body-aware check, not proof that enemies will never arrive.

From round 350, a complete worker can approach a reported collector if at least four allies are alive. Before round 425 it must be a non-grower of length at most four; thereafter length at most six is allowed, including small growers. No new worker approach starts on round 499. The collector's reported size must be at least two larger. Prefer larger collectors, counting size only up to twelve before round 425 and without that cap afterwards; subtract travel distance. Routes are limited to ten moves before 425, sixteen afterwards, and need five extra rounds before the game ends. A currently seen threatened destination is rejected.

Initially approach the advertised area. Once the collector's head is currently visible within four wrapped steps, approach that head's neighborhood using known paths; occupied squares are still forbidden. A collector visibly more than six wrapped steps from its advertised area is no longer a valid destination. Retain a chosen collector for up to eight rounds by giving it four extra preference points; invalid reports/size/safety still cancel it, and a sufficiently better collector can replace it earlier. This reduces switching without locking a worker to a dead or departed leader. Progress adds a soft preference of three points per step, capped at six steps in either direction. Food, danger and body checks still matter. A collector may move away or die and a station can become stale; no worker dies merely because it reached a station.

### Offer, agreement, death and collection

The worker sends an offer when a post-action sonar ray is known to reach its chosen collector. It can also offer to another freshly seen/reported ally of at least four segments and at least two longer than itself when a ray reaches that ally. It need not wait for a station message to arrive. This merely offers cooperation: the receiver still needs its own full collector eligibility and a safe route before accepting. The offer contains the worker's actual resulting head and size. The collector considers at most three fresh offers, each with a currently visible matching allied head within four wrapped steps. The worker's complete visible body must contain two to six segments; the collector must be at least two longer. The donor's square must be more than four known enemy steps away.

For each offer, consider up to eight one-step current moves that avoid direct enemy attacks, retain an exit, and can send sonar to that actual worker after moving. All legal one-step candidates are kept separately for this purpose, even if the generic shortlist dropped one. A route that fails while the donor stays alive can become safe after its agreed death, so it is judged using the full post-donation check instead. Temporarily remove the worker and place its real death pearls at the head and every second body segment. Require a route to the head pearl in one or two future **ordinary turns**, using currently seen ordinary edges. Each move must avoid a direct head attack, and up to ten further continuation moves must be verified after eating, using the same four-continuation safety search used when picking up the meal. Shorten the entire promise near game end. Never replace those turns with a paid sprint just to make a length-two donation work.

The potential benefit adds eight preference points for each donor pearl actually eaten along the checked pickup route, minus three for a two-turn pickup. Pearls elsewhere on the corpse receive no credit: they may never reach the collector. Only an approach whose resulting score beats our normal best route is accepted. The collector executes that approach and sends the acknowledgement from its real new position. It remembers the food square through at most the next two turns.

Immediately before death, the worker rechecks its own complete body, population of at least four, the same length/role restrictions, and enemy distance greater than four. The message must name its ID and **current head square**, and the promised collector square must now contain another allied head. The collector's visible chain or fresh exact report must establish a size at least two greater. Its head must also be more than four known enemy steps away.

An acknowledgement from last round is valid only from a higher-ID collector that has not acted again. A lower-ID collector must have sent it in the current round. The worker checks a one- or two-step ordinary approach through currently visible empty squares, with no close threat on the intermediate square. It also requires an exit after eating that is not the future neck, not the collector's old head, and not another occupied body. At least the required number of collection turns must remain, accounting for whether the collector has already acted this round. On the last round, an already-acted collector cannot benefit.

Only then does the worker move into its own verified neck. That kills this worker without attacking another head. At least three teammates remain. Roughly half the donor's length becomes pearls; this is costly concentration, not a free merge.

On the collector's next turn, the exact pearl must actually be visible and unoccupied. If the worker did not die, the message was lost, someone stole the food or the situation changed, there is no blind commitment. Before generic route pruning, prefer a safe one-step meal that truly increases length, with up to ten verified further moves or all remaining game turns. An intervening obstacle or threat can still cancel the meal. If two ordinary turns are needed, first choose a safe approach with a checked next-step meal and further continuation, and renew the reservation. Recheck again on the actual eating turn. This second turn is necessary for some relative movement patterns on a grid: a one-turn-only protocol cannot arrange every pair without wasting length on a sprint.

Both lower- and higher-ID collectors are supported. Intervening enemy moves, unseen traffic and missed sonar can still prevent a transfer. The code can confirm a local plan and current evidence; it cannot guarantee future cooperation or safety across the whole map. It never uses a fixed ID-based schedule of suicides.

## 11. Portals and last resorts

Mapped portals already belong to the movement graph and cost one step. The destination depends on which side of the entry edge is crossed. Ordinary portal moves and sprints require a visible destination and all normal body checks.

An optional blind portal is allowed only for a non-grower of length at most six, after eight foodless rounds and at least eight rounds since its last blind crossing, and when no retained candidate eats immediately. A grower may risk one only when no ordinary route remains or the checked complete-body search proves its retained routes fail, after rescue attempts.

Reject blind neck reversal, a known occupied exit, a currently visible exit handled by the normal planner, a current traffic warning, or an exit within one wrapped step of an ally reported in the last round. V4.1's last-resort exception remains: if **no ordinary move is legal**, an ally beside the exit does not by itself force certain death here. An ally reported directly on the exit still vetoes it. All other hard checks remain. Merely disliking an available move does not activate this exception.

Blind choices prefer mapped, less recently visited destinations; an unmapped endpoint gets one point, a mapped one two plus 0.1 per unvisited round capped at thirty. A repeatable variation smaller than 0.11 breaks ties. Hidden arrivals remain uncertain. A traffic warning cannot veto an ordinary currently visible legal crossing.

If no move remains, a head attack may be chosen when another ally will survive, or when our last dragon might draw against the only visible enemy head. The final fallback tries the current facing followed by clockwise alternatives, avoiding known allied heads if possible. If all four lead to allied heads, forward remains the unavoidable fallback.

## 12. Preference scales, limits and endgame details

These details complete the plain-English strategy above; the numbers are preferences unless explicitly described as safety checks.

| Item | Exact comparison used |
|---|---|
| Sprint loss | 24 points per extra step for growers; 22 for other dragons from round 450; otherwise 14. Actual legal cost is still one segment per extra step. |
| Current enemy danger | 130 / 36 / 6 for a one / two / three-step head attack; 12 replaces 130 only for the qualified worker standoff. |
| Current room | Twice the natural logarithm of one plus reachable room; count up to the smaller of 400 and the larger of 80 or `2 × length + 20`. The logarithm gives diminishing value to extra open squares. |
| Known small enclosure | Lose four points per square short of length plus five, only when the search actually proved enclosure. No exits costs 180; one exit costs nine. |
| Contested territory | Add 2.5 times the logarithm of one plus first-arrival space, and 0.75 times the logarithm of one plus tied space. |
| Recent repetition | Lose 0.7 per round short of eight since our last visit. A repeatable ID/round/square tie-break adds less than half a point. |
| Future food | Each newly imagined gain is multiplied by 0.94 once for each turn into the future. Future zero exits cost 150, one exit five. Future danger is 13% of the ordinary danger penalty, divided by one plus depth. |
| Future repetition and final room | Lose 0.18 per round short of six since a visit. At the search endpoint add 1.2 times the logarithm of room; a proven enclosure loses three points per square short of that future body length plus three. Room count caps at the smaller of 180 and the larger of 60 or length plus twenty. |
| Food-pocket rescue | Requires at least three gained segments, a complete future body and a free slot; subtract two normal food rewards for the retained front and require up to four checked rear moves. |
| Rescue search | Keep four continuations per depth, up to six turns. Favor room, newly collected food at one fifth its ordinary preference, and exits. Count room up to the smaller of 80 or rescued-half length plus twelve. |
| Exploration target | Frontier 12; otherwise portal 10 plus up to six from the observed portal fraction; otherwise old square three. Add 0.04 per unvisited round capped at 100, at most eight for unseen view squares, the measured openness/region preference, and subtract 0.7 per travel step and two per nearby ally. Branching preference is 1.5 times open edges above two, scaled by confidence. Regional openness difference is scaled by six. |
| Exploration progress | One point per closer step for growers, two for workers, capped at six steps in either direction. |
| Short enemy-reply check | Worst reachable direct head kill costs 80 except for a qualified standoff; closing every exit costs 45, leaving one costs six. |

Ordinary lookahead stops at 0.036 seconds on the game's virtual clock. Short enemy replies stop at 0.040. Feeding consideration must start before 0.040, stops taking new offers at 0.043 and new candidates at 0.044. Ordinary rescue searches can run to 0.052. A staged rescue must start before 0.044 and stops considering intermediate bodies at 0.049; its bounded continuation proof shares the 0.052 outer rescue deadline. Checks happen between bounded pieces of work; parsing, bookkeeping and output also consume the 100-million-point limit. Native clock behavior differs from the sandbox. Measured final costs and errors are in [VALIDATION](evaluations/v8-review/VALIDATION.md); they are observations, not a bound on every possible position.

From round 325, ordinary worker creation has stopped. From 350, agreed concentration is possible; from 425 it allows larger small donors and prioritizes larger collectors more strongly; from 450 losing length to sprints is more expensive for workers. Our exact largest enemy length remains unknown outside vision. We do not assume a lead is safe, sacrifice a scorer merely because another looks promising, or impose an upper winning length of nine or ten.

## 13. Human opponents, exposure and future suggestions

People write the opponents. Some deliberately exploit a bot that always yields to nearby heads; others recycle workers aggressively or plan late feeding. We therefore price the actual attacker, protect valuable allies and permit qualified pressure instead of reducing danger everywhere. Observed corpse food alone does not establish that a death was intentional.

We do not print strategy scores, targets or explanations into public replay logs. Sonar needed for cooperation remains visible and may reveal patterns. Close alternatives receive repeatable variation; it is not secret randomness. We will not weaken play just to disguise it.

For suggestions, compare the relevant section here first. Explain clear degradations briefly. For plausible improvements, verify only the relevant rule in MASTER and, if necessary, the focused documentation. Use wins as preservation evidence and losses as diagnostic evidence. Small local match results do not identify the isolated effect of each combined change or establish a ranked win rate.

## 14. Version and submission discipline

Working source, frozen release, packaged ZIP and evaluated source must have the same recorded fingerprint. Historical releases and MainBaselineBuild remain unchanged. Read the [current timing report](evaluations/v8-review/TIMING.md) and [submission ledger](submissions/history.json) before advising about another upload. Abyss-v7 is already submitted; Jormungandr-v1 is not uploaded by this task.

The 12-hour fresh-bot window begins at the server's first eligible ranked-play marker, not at upload. Reuploading identical code does not reset it. Different code inside an occupied window inherits the relevant ranked count; high sensitivity can amplify losses as well as gains. Missing submission identities are never guessed from which bot is currently active. Commands and upload records use the submission wrapper so timing and exact bytes remain traceable.
