# Our game plan — Abyss

This file explains the bot in `bot/`, including its limits and its numerical choices. The current version (abyss-v2, submission 9062) won 55 of 64 local native games across 8 maps against the hunter and greedy baselines, and 21 of 32 head-to-head games against the previous version. The full v1 baseline results are in [evaluations/REPORT.md](evaluations/REPORT.md) and the online loss audit that motivated v2 is in [evaluations/online/REPORT.md](evaluations/online/REPORT.md). A strong local result is evidence, not proof that we have the best bot on the human ladder.

The project uses toolkit 1.2.2. The C++ helper is identical to 1.1.0; compatibility was checked separately in [the toolkit update report](evaluations/TOOLKIT-1.2.2.md).

## 1. What we are trying to achieve

We want the best rank and rating we can earn. Inside a game, keeping at least one dragon alive comes first. If neither team is eliminated, the longest living dragon decides the winner; total team length matters only if those longest lengths tie.

We collect food while keeping enough room to continue moving. We aggressively create additional dragons to build a population that can absorb losses, contest territory, and replace eliminated members. We allow up to 48 dragons through normal splitting and continue splitting through round 420. We do not try to fill all 64 team slots, but we aim for a substantial population rather than relying on a few large dragons.

Every dragon runs the same rules independently. There is no all-seeing team captain. A dragon knows its own complete history when that history can be checked, its current view, its remembered map, and the recent messages it accepts.

## 2. What we remember and what we do not assume

We remember walls and discovered portal connections for the whole game. We update food, bodies and heads whenever a tile comes into view. Another dragon's last known body remains an obstacle for the current round and one additional round. Older body observations are no longer treated as certain obstacles.

We remember our own expected body after each action. On the next turn, we check that its length, head and visible segments agree with what the game reports. If they disagree, we rebuild the body from visible segments that point towards one another. A fresh child does the same reconstruction. If we cannot recover the complete body, we avoid guessing which unseen tail tiles have become free and avoid sprinting. However, we proactively split off the unseen tail when our length is at least five, keeping a known two-segment parent and giving the rear its own process. In an emergency, we split even earlier. This prevents long-body starts from becoming death traps.

Old food sightings remain potential destinations for at most 15 rounds. Their appeal falls with age. A previously empty unseen tile is never assumed safe for an action we are about to take. We normally commit only to destinations currently visible, including every step of a sprint. Longer imagined routes may go through remembered terrain; we reconsider them after the next observation.

We currently do not infer unexplored terrain from map symmetry. This avoids committing to an unconfirmed map shape. Unknown terrain can still encourage exploration through known safe steps.

## 3. How we choose a move

We list legal routes of one, two, and three steps for the current turn. We check every step separately: its wall or portal, the destination, our entire known body, other bodies, food, and the length cost of extra steps. Going into the current tail is forbidden, even when it would otherwise move away.

We give each route an initial score for food, room to move, exits, nearby enemy threats, repeated visits, and lost length. These preference points only help us choose; they are not game length or the judge’s work allowance. If at least one route has an exit, at least six reachable tiles, and no enemy attack within three steps, we discard routes with an enemy attack within three steps. This prevents tempting food from overriding an available sheltered move. We keep the best 14 routes for closer examination. For each, we imagine up to 10 further turns, normally one step per turn. At each imagined turn we keep the seven most promising continuations rather than following every possible route. This makes the search affordable. We stop imagining turns beyond the end of the actual game.

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

## 6. Sprinting

We consider at most three steps in a single action. Every additional step normally loses 14 preference points because it costs length. From round number 450 onwards that becomes 22, making late-game length preservation more important.

We allow a sprint when food, escape, or a better position makes up for that cost. We check affordability before each extra step. A pearl at the destination cannot pay for a step that is already unaffordable. We update the body after every successful step, so a tile freed by the first step may be usable later in the same action.

We do not take a blind multi-step sprint. If our body cannot be reconstructed fully, we consider only single-step moves.

## 7. Enemy threats and deliberate attacks

For each proposed position, we look for enemy routes to our new head within three steps. Walls and bodies block those routes. We also consider visible possible enemy tails when at least four enemy segments are visible: that tail may become a new child's head through splitting.

An enemy able to reach us in one step costs 130 preference points; two steps costs 36; three steps costs 6. The two-step and three-step penalties are deliberately lower than v1’s values (48 and 12) because against swarm opponents, distant threats are everywhere. Heavy penalties for ubiquitous danger made all moves equally bad and prevented the bot from choosing based on food and room. We do not assume an enemy will choose the polite or length-preserving move. These short threat checks are conservative about whether an enemy can afford a sprint, but do not cover every longer sprint or every chain of new splits.

Head trades are handled selectively. A head collision kills both dragons regardless of length, so the question is whether we can afford to lose one. We only initiate a head trade under two conditions. First, when we have no safe moves at all: we take the trade if at least one ally survives us (count greater than one), or if we are the last dragon and only one enemy head is visible, making a draw possible. Second, when we are trapped but some moves exist: we take the trade only if we have at least three living dragons and our length is four or less, meaning we are expendable. We never initiate a head trade when we have good safe moves available.

Our trapped check triggers when all routes are exhausted, or when the best route has no exit and at most two reachable tiles, or when it leaves a one-step attack on our head.

The exact action priority is: split if the normal or emergency conditions pass; handle incomplete-body splits; try an exploratory portal; attempt a qualifying head trade; otherwise take the best searched move; otherwise avoid an ally’s head; otherwise move forward.

## 8. Splitting

Normal expansion starts at length four. We split exactly two rear segments into a new dragon. We allow up to 48 living dragons through normal splitting, continue normal expansion through round 420, and wait at least one round between a parent's normal splits. This aggressive expansion means a dragon that collects two pearls (reaching length four) immediately splits, producing two length-two dragons that each independently seek food and split again. Against swarm opponents, matching their population is essential for contesting territory and absorbing head-trade losses.

Before a normal split, the child must have at least one exit and four reachable tiles. Neither the parent's stationary head nor the child's new head may have an enemy within the one-step attack check. The child's exit check leaves the parent's body blocked. A new child acts immediately in the same round; we account for that opportunity and danger.

Emergency splitting is different. If we are trapped, a legal split can preserve a child even if the parent is likely to die. It needs only two reachable tiles and one child exit, and still rejects a one-step attack on the child. It can exceed the normal 48-dragon target, ignore the usual cooldown and expansion-round rules, and works from length four. Emergency splits also work when the body is only partially known, as long as at least two segments have been reconstructed. The game's actual team cap and minimum lengths always apply.

Each child starts without its parent's memory. It does not receive a secret role assignment. Small dragons naturally qualify for more head trades; longer ones naturally collect and preserve more length. There is no merging rule, so splitting is a genuine loss of length concentration.

## 9. Sonar and sharing food

When we have another ally, or just created one, and know our post-action body exactly, we send our resulting head location, length, dragon number, team, and round. We send in two opposite directions, rotating the pair according to the round and dragon number. A failed/dead action does not send useful information.

The message contains a short check value to reject damaged or unrelated messages. It is not a secret or strong protection against deliberate imitation. We accept only matching-team messages with sensible coordinates and lengths, a matching check value, and a time no more than two rounds old. We ignore our own messages. If we can currently see the reported square and it disagrees with the report, we reject it.

Accepted reports last two rounds and only reduce competition for food when a longer ally is closer. They never override visible obstacles or authorize dangerous movement. We do not yet use sonar echoes to map distant obstacles, issue team-wide orders, or reveal an enemy location through a message. Portal routing, tail-origin emission and interception are handled by the game; we do not assume a message will reach its intended ally.

## 10. Portals and last resorts

A known portal connects to the other edge with the same number and orientation. We calculate which tile the head emerges on from the side we enter. Normal moves through it require a currently visible destination.

We take a portal with an unknown or currently unseen exit in three situations. First, when no safe local route exists. Second, when we have not collected food for five rounds and have not taken a portal for eight rounds, and no immediate food is available on any candidate route. Third, when all candidate routes score below 3.0 preference points with no food attraction, and the game has passed round three. This third trigger catches situations where the local area is thoroughly explored and staying put offers no growth, even if we are not technically starving. In all cases we then forget the expected body and reconstruct from the next observation.

If no portal is available either, we move along our current facing as the final required action. This can be fatal; it is not described as safe behavior.

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

We record every upload made with our submission command, the exact files, the returned version, and the time in a local ledger. Version 1 (abyss-v1, submission 9003) was uploaded on 2026-09-27 at 17:07 UTC. It played two unranked challenge sets totaling 20 games and lost all of them. Version 2 (abyss-v2, submission 9062) was uploaded on 2026-09-27 at 18:06 UTC and replaced v1 as the active submission. First-ranked events and effective battle counts must still come from verified server information once ranked play begins; later missing history stays marked unknown. We will not advise waiting for a fictional deadline.

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

All five are now addressed in the current strategy described in sections 1 through 12. Head trades require ally survivors or a draw opportunity (section 7). Expansion targets up to 48 dragons through round 420 (section 8). Portal exploration triggers after five foodless rounds or when local prospects are poor (section 10). Incomplete bodies shed their unseen tail at length five (section 2). Far-threat penalties were reduced so the bot can differentiate moves in swarm-heavy games (section 7).
