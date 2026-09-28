# Our game plan — Abyss

This file explains the **v4.1 candidate** in `bot/`, including every policy threshold. The latest verified online upload is v3 (9700); neither v4 nor v4.1 has been submitted. The [v4 review](evaluations/v4-review/REPORT.md) evaluates both user notes and the named games. The [local validation](evaluations/v4-review/VALIDATION.md) records the subsequent matches and the small portal fix. The [work tracker](current%20planned%20work.md) records completed work and remaining validation. Version history and validation are in [versions/README.md](versions/README.md). A small local check can catch regressions; it does not establish ladder strength.

The project uses toolkit 1.2.2. The C++ helper is identical to 1.1.0; compatibility was checked separately in [the toolkit update report](evaluations/TOOLKIT-1.2.2.md).

## 1. What we are trying to achieve

We want the best rank and rating we can earn. Inside a game, keeping at least one dragon alive comes first. If neither team is eliminated, the longest living dragon decides the winner; total team length matters only if those longest lengths tie.

We collect food while keeping enough room to continue moving. We can use all 64 slots, subject to the actual map limit, child safety and local crowding. Ordinary splitting ends at round 225. A large population helps only when it also supports a long surviving dragon.

Before round 200, ordinary breeders have length four, five or six. Length seven and above is protected from the start, even when population is small. From round 200, length six is protected too. From that round, with at least four allies, IDs whose remainder after division by six equals their team number (A=0, B=1) also become growers. From round 225 no one breeds normally. A child explicitly saved in a rescue remains a grower whenever its length is at least four. These roles do not guarantee a fixed team-wide grower count: each dragon has limited information.

Growers avoid ordinary splits, voluntary head trades and optional blind portals. Their sprint penalty is higher. A rescue may still divide them, but it checks which end can survive and keeps the greatest verified length it finds. We do not promise that every trap has an escape.

Every dragon runs the same rules independently. There is no all-seeing team captain. A dragon knows its own complete history when that history can be checked, its current view, its remembered map, and the recent messages it accepts.

## 2. What we remember and what we do not assume

We remember walls and discovered portal connections for the whole game. We update food, bodies and heads whenever a tile comes into view. Another dragon's last known body remains an obstacle for the current round and one additional round. Older body observations are no longer treated as certain obstacles.

We remember our expected body, including a partly known chain behind the head. On the next turn we check its head, visible segments and reported length. If inconsistent, we rebuild from visible segments pointing towards the head. Fresh children do the same; moving gradually recovers unseen segments. We never treat the end of an incomplete chain as the actual tail.

A blind portal crossing is still exactly one move. We keep the previous known body, prepend the new head reported on the following turn, and remove excess tail segments using the reported length. If the crossing ate a pearl, we retain that extra segment and reset the food timer. This prevents forgetting the neck left on the other side. Independently, blind reversal opposite the current heading is forbidden, including for newborns whose neck is outside view. After a split, visible segments given to the child remain recent allied obstacles until a newer observation replaces them.

Sprints with a partial body still require visible destinations and enough reported length for every step. Uncertain rescue splits are a separate last resort, described in section 8.

Old food sightings remain potential destinations for at most 15 rounds. Their appeal falls with age. A previously empty unseen tile is never assumed safe for an action we are about to take. We normally commit only to destinations currently visible, including every step of a sprint. Longer imagined routes may go through remembered terrain; we reconsider them after the next observation.

We currently do not infer unexplored terrain from map symmetry. This avoids committing to an unconfirmed map shape. Unknown terrain can still encourage exploration through known safe steps.

## 3. How we choose a move

We list legal routes of one, two, and three steps for the current turn. We check every step separately: its wall or portal, the destination, our entire known body, other bodies, food, and the length cost of extra steps. Going into the current tail is forbidden, even when it would otherwise move away.

We give each route an initial score for food, room, exits, enemy threats, repeated visits, exploration progress and lost length. These preference points are only for choosing moves. If a route has an exit, six reachable tiles and no one-step enemy attack, we discard routes exposing a one-step attack. Two- and three-step threats remain penalties, so they cannot automatically veto every useful food or exploration route. We keep the best 14 routes for closer examination. For each, we imagine up to 10 further turns, normally one step per turn, retaining seven promising continuations at each turn. We stop imagining turns beyond the end of the game.

Other dragons do not move inside these imagined continuations. Their current bodies are obstacles, and their nearby heads add danger. This is a useful caution, not an exact prediction of their next ten decisions. Future food appearances are also uncertain: we use their timing to choose attractive destinations, but do not pretend they have already fed us.

We check room again at the ends of the imagined routes. A route that collects food and then has no continuation receives a large penalty. The final comparison adds the initial route score to its best end-of-search continuation score, including the future food attraction and end-position room adjustment. We choose the highest-scoring current action, then throw away the rest of its imagined sequence and plan again next turn. A route not searched before the time limit retains only its initial score.

Ordinary lookahead stops at 0.036 seconds after input parsing on the game's clock. Rescue checks can use the remaining time up to 0.052 seconds. This reserves work for safety decisions instead of spending everything on ordinary routes. Donation checks do not start after 0.044 seconds. Loops check time between search levels, so a little work can finish beyond those cutoffs; parsing and output also cost points. The 100-million-point judge limit still applies. V4.1's 54,589 measured sandbox turns peaked at 25,426,672 points and 786,432 bytes. This is useful observed headroom, not a guarantee for every map; native timing is not a substitute.

If a searched branch runs out of continuations, we distinguish that from running out of information or search time. An incomplete body or unknown outgoing edge is not proof of death. When a searched surviving alternative exists, routes whose explored branches exhausted their known continuations are dropped. This is a bounded prediction with other dragons held still, not a mathematical proof of safety.

When a food route would become trapped, the search may also consider reversing through a split. It does this only before 0.030 seconds, with a complete imagined body, a free slot, and at least three net segments gained since our present length. It checks a four-move rear escape, shortened near the game's end. Leaving a two-segment parent must therefore still leave a child longer than we are now. It subtracts 24 preference points for the two segments left behind. Only already observed pearls contribute to that length calculation; future spawn attempts cannot pay for the escape. The plan is reconsidered every actual turn.

## 4. Food and growth

A pearl collected on the current action normally adds 12 preference points. A temporarily shared pearl described below adds 2.4 instead; survival can still justify taking it. Food found in a possible later continuation receives less credit the farther away it is: each additional imagined turn multiplies that credit by 0.94.

We consider up to 12 food destinations. We first measure how many steps lead to each through the known terrain, including portals and wrapping. We discard destinations not reachable through that terrain, and favor nearby reliable food. This distance calculation does not account for every future body movement; the detailed route checks do that later.

A currently or recently seen pearl has confidence `1 / (1 + 0.09 × rounds since seen)`. A tile due to attempt a spawn within six rounds has confidence `0.45 / (1 + 0.15 × rounds until attempt)`. A spawn attempt can fail if occupied, so we value it below real food.

A destination's attraction is `9 × confidence / (1 + 0.38 × steps away + 0.6 × expected wait + sharing adjustment)`. The expected wait counts rounds until a known spawn attempt after our estimated arrival, including imagined future turns. If our imagined body occupies the destination at its next spawn boundary, its attraction is multiplied by 0.1 because our body would block that attempt. We use the best remaining destination, excluding pearls already eaten by that imagined route. The sharing adjustment is two when a recently reported longer ally has a shorter route through known terrain. Straight-line distance through walls is no longer used for this comparison.

We reserve real food softly for a larger ally if it has length at least six, was seen or heard from within one round, is at most four known steps from the pearl, and is no more than one step farther away than we are. It must have a route through squares seen this round or last round with no currently known body in the way, and enough rounds left to arrive. There must be no known enemy route within two steps of that pearl. For at most eight rounds since we first saw the still-unclaimed pearl, its food reward and destination confidence are multiplied by 0.2. A confirmed empty sighting resets this age; unseen replacement pearls cannot be distinguished reliably. After eight rounds the discount ends. This is encouragement, not a ban: we never kill ourselves to obey a reservation.

We do not infer a hidden long-run spawn distribution from a single countdown. Deliberate late-game feeding is implemented separately in section 9; it is never used as a substitute for ordinary safe collection.

## 5. Room, loops and trapped positions

We count reachable empty tiles around a proposed head position, with the proposed body already in place. During the first comparison we stop counting between 80 and 400 tiles: the limit is twice our length plus 20, kept within those bounds. More room adds `2 × natural logarithm of (1 + counted tiles)` preference points. A natural logarithm just means that the first few additional tiles matter more than the hundredth.

If the region has no unexplored opening and has fewer than our length plus five tiles, we subtract four points per missing tile. No immediate exit subtracts 180 points; one exit subtracts nine. A recently visited destination loses 0.7 points for each round short of an eight-round gap. This discourages pointless circling without forbidding a safe loop.

In imagined future turns, no exit costs 150 points and one exit costs five. Revisiting a tile seen within six rounds loses 0.18 points per missing round. Current nearby-enemy danger is reduced to 13% and then divided by one plus the imagined turn distance, because distant forecasts are less certain.

At the end of the search, we count up to our length plus 20 tiles, with a minimum of 60 and maximum of 180. Room contributes `1.2 × natural logarithm of (1 + tiles)`. A completely enclosed region smaller than the imagined body plus three loses three points per missing tile. If all continuations disappear, the search contributes minus 200 plus 12 times the turn at which that happened.

These are preferences, not guarantees. A reachable region can still have a narrow entrance, an enemy can change it, and a short search can miss a later trap.

### Persistent exploration

Normally a short dragon starts persistent exploration after eight foodless rounds; a grower waits twenty. Before round 225, about one third of non-growers—IDs whose remainder after division by three equals their team number—start after four foodless rounds. The foodless timer begins at the dragon's first observation and resets when its selected visible route eats a pearl. A selected food goal within four known steps with confidence at least 0.25 delays exploration until twenty foodless rounds, or eight for the early exploration group. Productive loops can therefore remain useful farms.

We search known terrain for a reachable destination. Prefer a square next to unexplored terrain, then a portal with an unknown destination or a destination not visited for twenty rounds, then a square unvisited for thirty rounds. Their starting scores are 12, 10 and 3 respectively. Add 0.04 per round since our visit, capped at 100 rounds, and subtract 0.7 per travel step. Recent other bodies block this travel search; it does not predict their later movement. We keep the chosen destination for up to 24 rounds, replacing it upon arrival, expiry, or loss of a known route. Eating pauses the exploration incentive; a still-valid destination can be resumed later.

Being a step closer to this destination adds two preference points for scouts and one for growers. The difference from our current distance is capped at six steps in either direction. This adjustment joins the food attraction wherever the search uses it. It encourages a continuing route toward an exit or new region instead of relying only on short revisit penalties. We do not force departure from every small region: the Queen of Spades boxes are productive food areas when managed without excessive crowding.

## 6. Sprinting

We consider at most three steps in a single action. Every additional step normally loses 14 preference points because it costs length. From round 450 that becomes 22. Growers always pay 24 preference points per extra step. Escape or other benefits can still justify paying this cost.

We allow a sprint when food, escape, or a better position makes up for that cost. We check affordability before each extra step. A pearl at the destination cannot pay for a step that is already unaffordable. We update the body after every successful step, so a tile freed by the first step may be usable later in the same action.

We do not take a blind multi-step sprint. Partial-body movement uses the conservative occupancy and exact reported-length accounting described in section 2.

## 7. Enemy threats and deliberate attacks

For each proposed position, we look for enemy routes to our new head within three steps. Walls and bodies block those routes. We also consider visible possible enemy tails when at least four enemy segments are visible: that tail may become a new child's head through splitting.

An enemy able to reach us in one step costs 130 preference points; two steps costs 36; three steps costs 6. The two-step and three-step penalties are deliberately lower than v1’s values (48 and 12) because against swarm opponents, distant threats are everywhere. Heavy penalties for ubiquitous danger made all moves equally bad and prevented the bot from choosing based on food and room. We do not assume an enemy will choose the polite or length-preserving move. These short threat checks are conservative about whether an enemy can afford a sprint, but do not cover every longer sprint or every chain of new splits.

A head collision kills both dragons regardless of length. We now allow a voluntary trade when we have at least three living allies, our length is at most four, we are not a protected grower, and the enemy has more visible segments than our complete reported length. This is a verified lower bound on its size. We choose the largest qualifying visible enemy; ties retain route enumeration order (north, east, south, west). These trades can take priority even when safe moves exist. A larger victim is useful, but sacrificing our last survivor or a designated grower is not automatically favourable.

After rescue splits and portal options, a head trade is also a last resort when no ordinary move remains: either another ally exists, or we are the last survivor with at most one visible enemy head. The latter offers only a chance of a draw; unseen enemies may survive. We no longer trade against an equal or smaller enemy merely because a legal move has a nearby threat.

Our trapped check triggers when all routes are exhausted, or when the best route has no exit and at most two reachable tiles, or when it leaves a one-step attack on our head.

The exact action priority is: a qualifying trade against a larger enemy; a checked rescue split when trapped, or checked ordinary split otherwise; an allowed uncertain partial-body rescue; a qualified late-game donation; an allowed blind portal; a last-resort enemy head trade; the best searched move; a fatal fallback that avoids known allied heads where possible; otherwise forward. Trapping also includes a fully known searched route that runs out of continuations.

## 8. Splitting and escaping

### Ordinary expansion

Before round 225, a non-grower of length four through six can give two rear segments to a child. It needs a free slot below both 64 and the map's actual cap, at least one round since its own last split, and a complete body. Its head must not have a known one-step attack.

The child needs an immediate exit, eight reachable squares, no known one-step attack, and a two-move continuation with the parent's body held still. The parent also needs a checked two-move continuation with the child held still. At a population of eight or more, we refuse another birth if at least three visible allied heads are within four wrapped steps of the child, or if at least two are nearby and fewer than two food destinations within six known steps have confidence at least 0.25. This discourages overcrowding scarce food without setting another artificial global population cap.

### Checked rescues

When trapped and the full body is known, try child sizes `length−2`, 2, `length−3`, 3, `length−4`, 4 and half the length rounded down. Ignore duplicate or illegal sizes. This covers a large rear escape, a large front survivor, smaller adjustments and a balanced division without spending the whole turn testing every possible cut.

For each half, hold the other half's body still and simulate its own body moving. Keep up to four promising continuations per step. Check six moves for a child, or only the rounds still available; the parent gets one fewer remaining round because splitting uses its current action. The first destination must have no known one-step enemy attack. A proposed survivor's current head must also avoid a one-step attack. A blind portal is not accepted as the checked escape. At the horizon, require another exit unless the game would already be over. Ordinary remembered terrain and fresh body observations can still become outdated before the real move.

The continuation preference is `2 × natural logarithm of (1 + reachable squares) + 0.2 × simulated food reward + immediate exits`; count at most the smaller of 80 and that half's length plus twelve squares. A premature position with no exit loses 100 points. These scores select which four routes to retain; successful survival checks decide whether a half can count as saved.

Choose the cut with the greatest length in a checked surviving half. Break equal-length choices by preferring both halves surviving (20 extra comparison points), then their continuation scores. A checked `length−2` rear escape is accepted immediately because no legal split can keep more in one dragon. The other two-segment half may be lost; saving a long dragon can justify that loss. A rescue overrides ordinary breeding time, role, crowding and cooldown, but never the actual cap or minimum lengths.

On the final round, a parent needs no further move after its valid split. This can save a trapped parent even if its child has no escape. The child's same-round turn still matters.

### Incomplete bodies and preserving the saved child

An incomplete body is not split merely because it is the opening. If no visible ordinary move exists, a dragon with at least four segments, two known head segments and a free slot may keep two at the front and give the rest to the child. This is a legal rescue gamble with an unknown tail, not a checked escape. A child that already has a usable route will not repeat the old automatic opening cut.

For rescue splits of an original length seven or greater, a remaining parent of at least four keeps a protected role. A child of at least four receives a rescue-role message containing its expected length. Only a newborn receiving it in its birth round with exactly that length accepts it. We send it in all four directions; the game's tail-origin ray normally reaches the adjoining child body directly. Delivery is still handled by the engine. The role persists for its life, applies whenever it has length at least four, and prevents immediate ordinary re-splitting of the saved child. Ordinary births do not receive this protection. No merge exists.

## 9. Sonar, sharing and late-game feeding

### Status and traffic messages

When another ally exists (or a split creates one) and the post-action body is exactly known, normally send our resulting head, length, ID, team and round in all four directions. The 14-bit check value rejects corrupted/unrelated packets; it is not strong authentication against a determined imitator. Reject future messages, messages older than two rounds, invalid coordinates or lengths, our own status, and visible contradictions.

Visible heads and body counts also supply size lower bounds. A sonar size remains exact only while its position matches and it is from this round, or from the previous round when that higher-ID ally has not acted yet. A fresh visible lower bound does not indefinitely refresh an old exact size.

Two reserved message types carry a portal-busy warning or a newborn rescue role. A busy warning identifies the portal and expires after its send round plus one. It only blocks blind entry; it never authorizes a move. A rescue role is accepted only under the birth/length conditions in section 8.

When our post-action head is next to a portal with an unseen exit, send one probe toward it instead of the normal messages. Never use the direction opposite our post-action facing for this head probe: that direction emits from the tail. With only one ray sent, an ally or enemy hit in the following turn's echo can be attributed to that probe. Any such body/head hit marks that portal busy through the next round. A miss does not prove the exit clear, and a hit does not reveal how far away the obstruction was. If multiple adjacent portals qualify, the last in north/east/south/west order is probed.

When adjacent to a portal but not probing, two alternating directions carry its busy warning and the other two carry status. After a blind crossing, send a busy warning in all four directions, even though the new head position is not yet known. Warnings do not supply a portal map, guarantee receipt, or create a team-wide reservation. No map-sharing or group attack orders are implemented.

### Deliberate feeding

This is a narrow endgame tactic, not routine worker disposal. From round 400, a complete length-two-to-four non-grower that has not eaten for eight rounds may donate only with at least eight living teammates before the action. It must have no known enemy route within three steps of its head. The recipient must be a currently visible allied head of length at least eight and at least four longer than the donor, with no larger ally in our current reports. It must have a fresh exact size and an entirely reconstructable visible body. Its ID must be higher so it acts later this same round.

The recipient must be one ordinary open-edge step from the donor's current head, have no known enemy route within three steps, and have no other currently visible pearl reachable within two clear steps. We simulate removing the donor, creating pearls at its head and every second body segment, the recipient taking the head pearl, and up to three further safe moves with its actual known body. Shorten those future moves at round 499. If this check fails or time runs short, keep the worker.

The donor then makes exactly one move into its own verified neck. It dies in place without striking anyone else's head; the head pearl is immediately available to that later-moving recipient. We do not intentionally cause an illegal-action death. The new pearl also prevents another nearby worker from donating to an already supplied recipient in that round. The population floor prevents feeding from consuming the last few allies.

This checks the opportunity, not the recipient's future choice. An intervening opponent can still change the position, sonar can miss us, and an unseen longer teammate may exist. We do not claim guaranteed collection. We prefer a strict small opportunity over indiscriminate losses of workers. The local matches show both prompt feeding of long teammates and missed or diverted food. They do not establish that donations improve overall results.

## 10. Portals and last resorts

A known portal connects two edges with the same number and orientation. Compute the exit from the entry side. A normal move or sprint through it requires a currently visible destination; body and collision checks apply exactly as elsewhere.

A blind crossing is optional only for a non-grower of length at most six after eight foodless rounds and at least eight rounds since its last blind crossing. No retained candidate may immediately eat a pearl. The old low-score bypass is removed. Growers may risk a blind crossing only when no ordinary move remains or a complete known search has exhausted all continuations, after a checked rescue was unavailable.

Reject blind reversal into the neck, a known occupied exit, an exit already visible (the ordinary planner handles it), a current portal-busy warning, or a known exit within one wrapped step of an ally reported within one round. V4.1 adds one narrow exception: after rescue attempts, if **no ordinary move is legal**, an ally reported beside the exit no longer rules out that escape. A reported ally head directly on the exit still rules it out. All the other blocks remain. A merely unattractive or predicted-to-fail ordinary move does not activate this exception. This risks an uncertain arrival instead of choosing certain death solely because of a nearby report. A warning cannot rule out a currently visible legal crossing. An unmapped endpoint scores one; a known endpoint scores two plus 0.1 per round since our visit, capped at thirty rounds. A small repeatable ID/round/direction variation below 0.11 breaks close ties. After crossing, preserve the body as explained in section 2.

Hidden exits can still be occupied despite these precautions; turn order is sequential, not simultaneous, but our information is limited. These checks reduce avoidable collisions rather than promising collision-free portals.

If no survival action remains, try the current facing and then the next three clockwise directions, skipping known allied heads. The chosen move may still be fatal. If all four destinations are allied heads, forward is the final fallback.

## 11. End of the game

The installed engine numbers its 500 rounds from 0 to 499. We confirmed this directly. Our normal splitting stops well before the end, late sprints cost more in our comparison, and our imagined future stops at round 499.

We do not know the exact largest enemy length outside vision, so we do not pretend to know that a lead is secure. We also do not yet run a separate exact endgame solver. Survival, food and length preservation remain the priorities, with emergency actions still available.

## 12. Humans, predictable habits and concealment

We test against food chasers, head-hunters that make many small attackers, cautious versions, and versions that split at different times. These are models of plausible human-written bots, not a claim that people are irrational or all behave alike.

Close move choices receive a small variation below half a preference point, determined by the dragon, round, and destination. This changes repetitive ties without overriding a materially better score. It is repeatable, not secret randomness; someone studying enough replays could predict it.

The submitted bot does not print its route scores, intended long-term targets, or decision explanations into public replays. It still sends the status messages it uses. We do not deliberately lose games, withhold useful attacks, or weaken a move merely to hide strategy. Ranking comes first.

## 13. How we decide whether a suggestion helps

We first compare a suggestion with the relevant section here. If it clearly makes us worse, we explain the reason briefly. If it looks promising, we check the rule in the master reference and only look further where that specific question remains unanswered. Then we compare both the benefit and the cost, and test a changed version when the difference is uncertain.

A rejected experiment remains in the evaluation records where useful. It does not silently become part of the implemented plan.

## 14. Submissions and rating timing

The upload time and the first ranked battle time are different. A new-code bot may receive stronger rating changes for its early ranked battles, but the 12-hour window starts when an eligible bot first plays ranked. Identical code does not restart that advantage. Stronger rating changes can also magnify losses.

Uploads and verified ranked series are tracked in [submissions/history.json](submissions/history.json). Uploaded IDs are v1: 9003, v2: 9062, and v3: 9700. V4.1 is the local candidate; v4 remains a frozen unsubmitted build. Use the timestamped [timing report](evaluations/recent-20260928/TIMING-LATEST.md) and current server markers before giving upload advice. Upload time, first individual game and the server's fresh-window marker are recorded separately; missing version attribution is never guessed.

The tracker reports both India time and UTC. A genuinely stronger bot can be worth uploading inside the 12-hour window. Timing is an aid to improving rank, not a reason to leave a clearly weaker bot active.

## 15. Research used as inspiration

[AlphaSnake](https://arxiv.org/abs/2211.09622) studies searching ahead in Snake. It supports investigating lookahead, but its single-player setting does not establish the best policy for this game. We are not claiming to implement its trained method.

[The Battlesnake Challenge](https://arxiv.org/abs/2007.10504) combines learned play and human-supplied guidance. We take the lesson of comparing against meaningful opponent behaviors; we do not copy Battlesnake collision rules into Battlecode.

[Red Blob Games' pathfinding explanation](https://www.redblobgames.com/pathfinding/a-star/introduction.html) explains representing movement as connections between positions. We use that perspective for walls, wrapping and portals. A route that looks short on a flat board need not be short or safe here.

## 16. What the first online games revealed and how we responded

Version 1 lost all 20 unranked games against blauerdrache and bongcloud. The [replay audit](evaluations/online/REPORT.md) records the evidence. Five problems drove the losses:

1. We initiated 99 of 114 head collisions. With only four dragons against opponents fielding 21 to 64, each 1-for-1 trade devastated us while barely affecting them.
2. Opponents built massive populations while our cap was four. On Autarky at round 100, we had two dragons and four total segments against 31 dragons and 71 segments.
3. Our portal policy refused unseen exits whenever any safe local move existed. On Portals maps, our dragons never entered a distant portal and finished with less total length than they started with.
4. Some maps start dragons with bodies extending beyond vision. Disabling all splits and sprints with a partially visible body caused immediate kelp deaths on Autarky, Dilemma, and Slithery Fight.
5. Our local baselines (greedy and hunter) were too simple to expose these weaknesses.

v2 improved survival and population, but the later replay audit showed recurring round-limit losses: many small dragons did not compensate for a short longest dragon. V3 introduced protected growers, the full population cap with room checks, persistent exploration, actual-tail checks, and selected attacks on larger enemies. V4 refines those policies using the additional evidence in section 17. Competitive value remains subject to match validation.

## 17. V4 evidence and validation

The [v4 review](evaluations/v4-review/REPORT.md) separates measured replay behavior, interpretation, and implemented changes. It covers the named Hampter examples, the latest named Borgor series, and the two high-rated reference games. Portal self-collisions and repeated splitting of trapped long parents recur across games. Deliberate feeding is visible in explicit actions from the Trauma reference; it is not inferred solely from corpse collection.

V4 passed 44 focused strategy checks, four native games and two sandbox games against frozen v3. It won all six without runtime errors. V4.1 adds three checks for the last-resort portal exception, bringing the total to 47 with a warning-clean C++ compilation. It then won two native Portals games and two sandbox Slithery Fight games without runtime errors, engine notices or initiated portal collisions. Its exact source identity, resource usage and remaining limitations are in the [validation report](evaluations/v4-review/VALIDATION.md). These are small fixed-seed checks against one earlier version, not a measured ranked win rate.
