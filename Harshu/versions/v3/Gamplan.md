# Our game plan — Abyss

This file explains the **v3 candidate** in `bot/`, including every policy threshold. The active online submission remains v2 (9062) until an upload is made. The [current audit](../../evaluations/recent-20260928/REPORT.md) explains the recurring weaknesses behind these changes. Version history and validation are in [versions/README.md](../../versions/README.md). A small local check can catch regressions; it does not establish ladder strength.

The project uses toolkit 1.2.2. The C++ helper is identical to 1.1.0; compatibility was checked separately in [the toolkit update report](../../evaluations/TOOLKIT-1.2.2.md).

## 1. What we are trying to achieve

We want the best rank and rating we can earn. Inside a game, keeping at least one dragon alive comes first. If neither team is eliminated, the longest living dragon decides the winner; total team length matters only if those longest lengths tie.

We collect food while keeping enough room to continue moving. Breeders can use all 64 slots, subject to the map's actual limit, child survival checks and local crowding. Normal splitting ends at round 350, leaving 150 rounds to grow. A large population helps only if we can also keep long dragons alive.

Once there are at least four allies, a dragon becomes a protected grower if its length is at least six, or if its ID leaves remainder zero when divided by six on team A, or remainder one on team B. This simple repeatable rule reserves some small dragons for growth and protects every already-long dragon. It does not guarantee an exact team-wide number of growers: visibility and messages are limited. If population falls below four, normal breeding takes priority again. Growers avoid normal splits and voluntary head trades, pay a higher sprint penalty, and take fewer blind portal risks. Emergencies can still force a split.

Every dragon runs the same rules independently. There is no all-seeing team captain. A dragon knows its own complete history when that history can be checked, its current view, its remembered map, and the recent messages it accepts.

## 2. What we remember and what we do not assume

We remember walls and discovered portal connections for the whole game. We update food, bodies and heads whenever a tile comes into view. Another dragon's last known body remains an obstacle for the current round and one additional round. Older body observations are no longer treated as certain obstacles.

We remember our own expected body, including a partly known chain behind the head. On the next turn we check its head, visible segments and that it is no longer than our reported length. If inconsistent, we rebuild from visible segments pointing towards one another. A fresh child does the same. Moving extends this known chain until the whole body is recovered. We retain observed own-body obstacles conservatively and never assume an unseen tail square has become free. Sprints remain possible with a partial body, but every destination must be visible and the reported length must pay for each step. Special uncertain splits are described in section 8; the end of a visible chain is never treated as a verified tail.

Old food sightings remain potential destinations for at most 15 rounds. Their appeal falls with age. A previously empty unseen tile is never assumed safe for an action we are about to take. We normally commit only to destinations currently visible, including every step of a sprint. Longer imagined routes may go through remembered terrain; we reconsider them after the next observation.

We currently do not infer unexplored terrain from map symmetry. This avoids committing to an unconfirmed map shape. Unknown terrain can still encourage exploration through known safe steps.

## 3. How we choose a move

We list legal routes of one, two, and three steps for the current turn. We check every step separately: its wall or portal, the destination, our entire known body, other bodies, food, and the length cost of extra steps. Going into the current tail is forbidden, even when it would otherwise move away.

We give each route an initial score for food, room, exits, enemy threats, repeated visits, exploration progress and lost length. These preference points are only for choosing moves. If a route has an exit, six reachable tiles and no one-step enemy attack, we discard routes exposing a one-step attack. Two- and three-step threats remain penalties, so they cannot automatically veto every useful food or exploration route. We keep the best 14 routes for closer examination. For each, we imagine up to 10 further turns, normally one step per turn, retaining seven promising continuations at each turn. We stop imagining turns beyond the end of the game.

Other dragons do not move inside these imagined continuations. Their current bodies are obstacles, and their nearby heads add danger. This is a useful caution, not an exact prediction of their next ten decisions. Future food appearances are also uncertain: we use their timing to choose attractive destinations, but do not pretend they have already fed us.

We check room again at the ends of the imagined routes. A route that collects food and then has no continuation receives a large penalty. The final comparison adds the initial route score to its best end-of-search continuation score, including the future food attraction and end-position room adjustment. We choose the highest-scoring current action, then throw away the rest of its imagined sequence and plan again next turn. A route not searched before the time limit retains only its initial score.

The search stops after 0.052 seconds on the game's own clock, measured after input parsing. In the judge, this corresponds to about 52 million work points. Some short setup and finishing work lies outside that check. The actual total, including input and output, is measured during sandbox tests against the 100 million limit.

## 4. Food and growth

A pearl collected on the current action adds 12 preference points. Food found in a possible later continuation receives less credit the farther away it is: each additional imagined turn multiplies that credit by 0.94.

We consider up to 12 food destinations. We first measure how many steps lead to each through the known terrain, including portals and wrapping. We discard destinations not reachable through that terrain, and favor nearby reliable food. This distance calculation does not account for every future body movement; the detailed route checks do that later.

A currently or recently seen pearl has confidence `1 / (1 + 0.09 × rounds since seen)`. A tile due to attempt a spawn within six rounds has confidence `0.45 / (1 + 0.15 × rounds until attempt)`. A spawn attempt can fail if occupied, so we value it below real food.

A destination's attraction is `9 × confidence / (1 + 0.38 × steps away + sharing adjustment)`. We use the best remaining destination, excluding food that the imagined route already ate. The sharing adjustment is 2 when a recent message reports a longer ally closer to that food. It encourages smaller allies to find their own food without making them obey a rigid reservation.

We do not yet estimate a tile's hidden long-run spawn rate, deliberately block enemy spawns, or deliberately kill ourselves to feed another dragon. Those remain ideas to test, not hidden features of this release.

## 5. Room, loops and trapped positions

We count reachable empty tiles around a proposed head position, with the proposed body already in place. During the first comparison we stop counting between 80 and 400 tiles: the limit is twice our length plus 20, kept within those bounds. More room adds `2 × natural logarithm of (1 + counted tiles)` preference points. A natural logarithm just means that the first few additional tiles matter more than the hundredth.

If the region has no unexplored opening and has fewer than our length plus five tiles, we subtract four points per missing tile. No immediate exit subtracts 180 points; one exit subtracts nine. A recently visited destination loses 0.7 points for each round short of an eight-round gap. This discourages pointless circling without forbidding a safe loop.

In imagined future turns, no exit costs 150 points and one exit costs five. Revisiting a tile seen within six rounds loses 0.18 points per missing round. Current nearby-enemy danger is reduced to 13% and then divided by one plus the imagined turn distance, because distant forecasts are less certain.

At the end of the search, we count up to our length plus 20 tiles, with a minimum of 60 and maximum of 180. Room contributes `1.2 × natural logarithm of (1 + tiles)`. A completely enclosed region smaller than the imagined body plus three loses three points per missing tile. If all continuations disappear, the search contributes minus 200 plus 12 times the turn at which that happened.

These are preferences, not guarantees. A reachable region can still have a narrow entrance, an enemy can change it, and a short search can miss a later trap.

### Persistent exploration

After eight rounds without eating, a scout seeks a lasting travel destination. A grower waits twenty rounds. The foodless timer begins at the dragon's first observation and resets when its selected visible route eats a pearl. Until twenty foodless rounds have passed, any selected food goal within four known steps with confidence at least 0.25 delays exploration. Productive loops can therefore remain useful farms.

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

The exact action priority is: a qualifying trade against a larger enemy; a checked normal or emergency split; an allowed uncertain partial-body split; an exploratory portal; a last-resort head trade; the best searched move; a fatal fallback chosen to avoid an ally's head where possible; otherwise forward.

## 8. Splitting

Normal expansion starts at length four for non-growers. We split two rear segments, allow up to 64 living dragons (or the map's lower limit), stop normal splits at round 350, and require one round since the parent's last split. Growers retain their length. There is no bonus merely for filling a slot.

Before a normal split we must know the whole body and thus the actual child location. The child needs an exit, eight reachable tiles, and a route surviving two ordinary moves while the parent's body stays blocked. Neither parent nor child may have a one-step enemy attack. Once our population is at least eight, we reject a normal split if three or more currently visible allied heads lie within four wrapped steps of the child. This local crowding check includes our own head if nearby. Room and two-move checks use remembered terrain and body observations; intervening enemy actions can still change them before the child's turn.

For a trapped dragon with a complete body, an emergency split needs four reachable tiles, an exit, the same two-move continuation and no one-step attack on the child. It can override grower protection, crowding, cooldown, and the normal stopping round. It may leave the parent endangered. Legal lengths and the actual team cap always apply.

With a partial body, no child safety check is claimed. If no ordinary move exists, a legal uncertain split may leave a known two-segment parent and give all remaining segments to a child. This also happens in rounds 0–2 for a length-eight-or-longer starting dragon when no one-step head threat exists. Both cases require at least two known head segments, total length at least four and a free team slot. The unseen child can be trapped; this is explicitly a rescue gamble, not a checked safe split. Ordinary partial-body travel later in the game no longer automatically sheds a long tail.

Each child starts without its parent's memory and determines its role from its own ID, length and team population. There is no merge, so splitting permanently redistributes concentrated length.

## 9. Sonar and sharing food

When we have another ally, or just created one, and know our post-action body exactly, we send our resulting head location, length, dragon number, team, and round. We send in two opposite directions, rotating the pair according to the round and dragon number. A failed/dead action does not send useful information.

The message contains a short check value to reject damaged or unrelated messages. It is not a secret or strong protection against deliberate imitation. We accept only matching-team messages with sensible coordinates and lengths, a matching check value, and a time no more than two rounds old. We ignore our own messages. If we can currently see the reported square and it disagrees with the report, we reject it.

Accepted reports last two rounds and only reduce competition for food when a longer ally is closer. They never override visible obstacles or authorize dangerous movement. We do not yet use sonar echoes to map distant obstacles, issue team-wide orders, or reveal an enemy location through a message. Portal routing, tail-origin emission and interception are handled by the game; we do not assume a message will reach its intended ally.

## 10. Portals and last resorts

A known portal connects to the other edge with the same number and orientation. We calculate which tile the head emerges on from the side we enter. Normal moves through it require a currently visible destination.

An unknown or unseen portal exit is considered when no ordinary move exists, or when no candidate immediately eats and one of these conditions holds: five foodless rounds plus eight rounds since our last blind portal; or the best candidate scores below 3 and its combined food/exploration attraction is below 1, at round 3 or later. The poor-prospects alternative does not require the eight-round cooldown. Growers additionally require twenty foodless rounds and round below 420, unless there is no ordinary move. Known occupied exits are rejected. Among available blind portals, an unknown endpoint scores 1; a known endpoint scores 2 plus 0.1 per round since visiting, capped at 30 rounds. A small ID/round/direction variation below 0.11 resolves close options. After entry, we clear the predicted body and reconstruct it at the next observation.

If no survival action remains, try the current facing then the following three directions in clockwise order, skipping known allied heads. The selected move can still be fatal. If all four destinations are allied heads, forward is the final fallback.

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

Uploads and verified ranked series are tracked in [submissions/history.json](../../submissions/history.json). v1 is submission 9003; active v2 is submission 9062; v3 is a local candidate. The server's first fresh-window marker is 2026-09-27 18:34:02.102 UTC, giving a 12-hour deadline of **28 September 12:04:02.102 India time**. This marker followed completion of a ranked series; the earliest individual game started earlier. Use the refreshed [timing report](../../evaluations/recent-20260928/TIMING-LATEST.md) before making a submission decision. A stronger release can still be worth uploading before that deadline with inherited rating sensitivity.

The tracker reports both India time and UTC. A genuinely stronger bot can be worth uploading inside the 12-hour window. Timing is an aid to improving rank, not a reason to leave a clearly weaker bot active.

## 15. Research used as inspiration

[AlphaSnake](https://arxiv.org/abs/2211.09622) studies searching ahead in Snake. It supports investigating lookahead, but its single-player setting does not establish the best policy for this game. We are not claiming to implement its trained method.

[The Battlesnake Challenge](https://arxiv.org/abs/2007.10504) combines learned play and human-supplied guidance. We take the lesson of comparing against meaningful opponent behaviors; we do not copy Battlesnake collision rules into Battlecode.

[Red Blob Games' pathfinding explanation](https://www.redblobgames.com/pathfinding/a-star/introduction.html) explains representing movement as connections between positions. We use that perspective for walls, wrapping and portals. A route that looks short on a flat board need not be short or safe here.

## 16. What the first online games revealed and how we responded

Version 1 lost all 20 unranked games against blauerdrache and bongcloud. The [replay audit](../../evaluations/online/REPORT.md) records the evidence. Five problems drove the losses:

1. We initiated 99 of 114 head collisions. With only four dragons against opponents fielding 21 to 64, each 1-for-1 trade devastated us while barely affecting them.
2. Opponents built massive populations while our cap was four. On Autarky at round 100, we had two dragons and four total segments against 31 dragons and 71 segments.
3. Our portal policy refused unseen exits whenever any safe local move existed. On Portals maps, our dragons never entered a distant portal and finished with less total length than they started with.
4. Some maps start dragons with bodies extending beyond vision. Disabling all splits and sprints with a partially visible body caused immediate kelp deaths on Autarky, Dilemma, and Slithery Fight.
5. Our local baselines (greedy and hunter) were too simple to expose these weaknesses.

v2 improved survival and population, but the later replay audit showed recurring round-limit losses: many small dragons did not compensate for a short longest dragon. The v3 changes above protect growers, allow the full population cap with room checks, add persistent exploration, repair the unknown-tail check, and permit selected attacks on larger enemies. These are implemented changes; their competitive value remains subject to the limited checks and future online results linked at the top.
