# Jormungandr-v1 — review and architecture decisions

Jormungandr-v1 continues frozen abyss-v7. The name begins a new series; it does
not overwrite v7 or use the held v5 experiment. MainBaselineBuild, historical
releases, research/ and user notes remain untouched. [Gamplan](../../Gamplan.md)
is the complete plain-English policy; [validation](VALIDATION.md) records the
exact finished source and the small controlled comparison.

## Evidence and attribution

Downloaded **twenty games**, both recent Lozer series 578853–578862 and
580042–580051, including seven wins and thirteen losses. Each individual API
record names Borgor as A. Starting dragon ID parity changes when maps swap
sides, so IDs alone are not team labels. The map's explicit team flags,
valid v7-format sonar packets, API scores and replay outcomes agree. See
[identity checks](replay-identities.json), [phase audit](competition.json) and
[evidence index](evidence-index.json).

Eight games received the independent full-body/vision/death/feeding audit:
both Slithery games, both Trophy losses, a Devil win/loss and both Trauma wins.
It matched **3,234 round snapshots**. The remaining twelve have phase/action
and collision analysis, not an invented full decision reconstruction. Online
maps redact spawn ranges; replay observations do not expose the old process's
memory. A skipped pearl is a diagnostic, not proof of a safe missed meal.
[Deep audit](deep-audit.json), [first-150-round hunts](hunting.json).

The game API does not expose per-game submission identity. The chronology,
our distinctive protocol and exact opening behavior support the v7 attribution;
we do not claim the API supplies a cryptographic source fingerprint for a replay.
The local comparisons use immutable source fingerprints.

| Game | Map | Borgor result | First 100 pearls, ours / opponent | First 100 splits, ours / opponent |
|---|---|---|---:|---:|
| [578853](https://game.battlecode.au/visualiser?match=578853) | Autarky | Loss | 32 / 40 | 18 / 22 |
| [578854](https://game.battlecode.au/visualiser?match=578854) | Default | Win | 28 / 30 | 13 / 16 |
| [578855](https://game.battlecode.au/visualiser?match=578855) | Devil | Win | 13 / 143 | 7 / 56 |
| [578856](https://game.battlecode.au/visualiser?match=578856) | Portals | Loss | 19 / 5 | 9 / 2 |
| [578857](https://game.battlecode.au/visualiser?match=578857) | Prisoners Dilemma | Win | 5 / 80 | 7 / 29 |
| [578858](https://game.battlecode.au/visualiser?match=578858) | Queen Of Spades | Loss | 43 / 14 | 11 / 6 |
| [578859](https://game.battlecode.au/visualiser?match=578859) | Schooltime | Loss | 50 / 133 | 17 / 41 |
| [578860](https://game.battlecode.au/visualiser?match=578860) | Slithery Fight | Win | 68 / 405 | 17 / 147 |
| [578861](https://game.battlecode.au/visualiser?match=578861) | Trauma | Win | 6 / 5 | 4 / 3 |
| [578862](https://game.battlecode.au/visualiser?match=578862) | Trophy | Loss | 30 / 104 | 12 / 40 |
| [580042](https://game.battlecode.au/visualiser?match=580042) | Autarky | Loss | 21 / 35 | 16 / 19 |
| [580043](https://game.battlecode.au/visualiser?match=580043) | Default | Loss | 15 / 10 | 9 / 8 |
| [580044](https://game.battlecode.au/visualiser?match=580044) | Devil | Loss | 5 / 116 | 5 / 37 |
| [580045](https://game.battlecode.au/visualiser?match=580045) | Portals | Win | 1 / 12 | 1 / 4 |
| [580046](https://game.battlecode.au/visualiser?match=580046) | Prisoners Dilemma | Loss | 10 / 114 | 6 / 41 |
| [580047](https://game.battlecode.au/visualiser?match=580047) | Queen Of Spades | Loss | 3 / 18 | 1 / 8 |
| [580048](https://game.battlecode.au/visualiser?match=580048) | Schooltime | Loss | 19 / 29 | 10 / 16 |
| [580049](https://game.battlecode.au/visualiser?match=580049) | Slithery Fight | Loss | 115 / 414 | 32 / 149 |
| [580050](https://game.battlecode.au/visualiser?match=580050) | Trauma | Win | 1 / 3 | 2 / 2 |
| [580051](https://game.battlecode.au/visualiser?match=580051) | Trophy | Loss | 24 / 102 | 10 / 35 |

## Weaknesses worth changing

**1. Trapped short starters lose viable descendants.** In both Slithery replays,
our length-four and length-five corridor starters collect food twice, then die
at round two: IDs 2/8 on one orientation, 3/9 on the other. The central body
segments cover open junctions. A single six-move walking proof cannot see that
splitting, gathering visible food and splitting again exposes a new head there.
A brute-force maximum rear split alone merely reverses into another dead end.
The new bounded staged rescue searches that second cut with all abandoned
pieces still blocked. It does not assume future corpse food or friendly movement.
The ordinary 25→2+23 coil save stays first, because it already preserves a scorer.

**2. Worker attrition follows a resource/access disadvantage.** Both Trophy
losses are eliminated before round 225, so late feeding cannot repair them.
In game 578862 the enemy initiates eight early cross-team head collisions before
round 150, killing eight of our length-two workers for 21 of its own segments.
In 580051 it initiates four such collisions, killing four length-two workers for
eight segments. All were ordinary one-step attacks, not unaffordable imaginary
sprints. The attacker was visible at each victim's preceding turn in these Trophy
cases. That does not by itself prove a safe immediate escape existed; chokepoint
choices and the continuing supply of replacements matter.

Keep hard collision checks. Extend the bounded enemy-reply check to opening
encounters with one fully seen opponent. At contested fronts, a length-four
worker with at least four teammates keeps a spare segment until five unless the
rear can immediately feed or the productive replacement exception applies.
Increase hungry short workers' food preference within the existing 35% ceiling.
Let all workers in a barren opening seek new ground promptly, while productive
farms and large growers keep priority. No broad equal-size suicide policy was
added. The replay does not establish that copying Lozer's deaths would help us.

**3. Coordination works mechanically but rendezvous is weak.** Across the eight
new deep audits, fifteen delivered acknowledgements led to eight matching donor
deaths, and all eight intended head pearls were collected. Thus these games do
not support calling every late death a broken handshake. Many games have no
agreement at all, or end before the feeding phase. Source inspection found that
advertised areas did not affect collector movement; workers switched assignments
and chased moving heads at long range. Older v7 sandbox evidence also contained
four missed intended pickups out of 63 donations.

Collectors now have a modest pull back to their area, with two steps of free
circulation and a preference for nearby food. Workers retain an assignment for
eight rounds unless it becomes invalid or a clearly better option appears.
They approach the area at long range and the actual head only within four steps.
A worker may offer to a visible plausible collector before hearing a station;
only the collector's full acknowledgement permits death. The engine fixture
caught a real failure to rendezvous without this exception, and both ID orders
now complete a real transfer. Agreement and pickup both check ten continuation
moves; promised and potential feeding moves are retained outside generic pruning.

## Strengths that constrain the changes

The Slithery win finished with our longest dragon **45 versus 28**, despite only
three survivors versus eighteen and total length 63 versus 232. The Devil win
finished 16 versus 13 despite a vast collection deficit. Both Trauma games won,
with maximum lengths 17/7 and 14/6. These directly contradict optimizing only
population or total pearls. Grower protection, the coil rescue, body/portal
pathfinding, last-round timing and selective profitable trades remain.

## Jormungandr.md: considered decisions

| Suggestion | Decision and reliable implementation |
|---|---|
| Birth-turn teaching | Final design: all four rays carry one combined map-mode/rescue-role packet after every split, including partial-body rescues. Four engine fixtures verify straight, coil, portal delivery and an unreachable bent child. An uneducated child stays general; no unsafe cut is made solely for teaching. Optional food/terrain sharing resumes after the split turn. |
| Choose a split size to align the child's head | Rejected as stated: the old tail is the child's head for every cut. A body segment can receive sonar. Changing length solely for head alignment adds cost without changing that position. Never use an unsafe default split to obtain education. |
| Four-by-four map chunks | Replaced with independently framed six-edge patches. The proposed layout consumes every bit before adding a separate type and cannot encode portal IDs or age. Our ordinary packet retains sender, time, team and a fourteen-bit error check. Patches influence a separate navigation graph, not legality or escape proofs. |
| Portal sharing | Implemented whole paired edges plus ID and validation of orientation/range/direct contradictions. Directly known pairs cannot be overwritten. No portal is invented from a type marker alone. Both crossing sides are represented. |
| Communication schedule | Keep ordinary status by default; at most one optional intel ray on alternating ordinary turns. Teaching can bypass that schedule. Rescue, feeding and single-ray portal probes retain priority. Only rays known to reach an ally carry optional information. No fixed north/south assumption about where allies live. |
| Food reports and forecasts | Implement fresh, bounded targets from current food or the real one-to-six-round countdown. At most sixteen reports, eight-round lifetime, three extra targets. Direct emptiness cancels a report. Reported pearls never finance sprinting or prove growth. Existing countdown-aware planning remains; no guessed hidden spawn distribution is used. |
| Message types and hostile input | Implement separate tags, range/time/team checks and bounded processing. Check values are not authentication. Remote terrain cannot authorize a current move, certify an escape, or become relayed first-hand knowledge. |
| Isolation after sixty quiet rounds | Implement a quiet-front exploration mode after round 100 and sixty rounds without seeing any enemy segment. Immediately cancel on sighting. Do not infer complete partitioning or enemy elimination; do not erase danger, spend length freely, or enter dead ends. |
| Pair hunting with shared enemy positions | Deferred. Stale head coordinates and ray delivery do not establish a joint trap. Retained current-vision tactical replies and valuable trades are safer than a new unvalidated pursuit system. |
| Collective death danger map | Deferred. Deliberate donations, unavoidable remnants and combat deaths mean a death square is not automatically dangerous. Permanent penalties could repel workers from profitable farms. |
| Full-map convergence and survival percentages | Not claimed. The document's 70→90% survival and 100–150-round convergence figures are hypotheses. Ray contacts, losses, repeated data and unknown recipient memory prevent inferring them from nominal bandwidth. |

The full thresholds, exceptions, packet priorities and remaining limits are in
[Gamplan](../../Gamplan.md). Shared knowledge is useful guidance, not an extra
sensor. This build changes no game rules and does not require a full documentation
reread: MASTER, focused splitting/sonar references and the installed 1.2.2 engine
were sufficient for the rule questions.

## Validation discipline

One integrated release, no intermediate uploads. Predeclared six sandbox games:
seed 911, both sides against frozen v7, Slithery and Trophy as weaknesses and
Trauma as a preservation check. Small correctness fixtures are separate from
strength evidence. See [VALIDATION](VALIDATION.md) for outcomes and resource limits;
combined changes cannot establish the isolated effect of any one feature or a
future ranked win rate.


## Thor.md: map-specific strategy and caveats

Accepted complete per-map terrain/resource knowledge, but only after a strict
original-start signature and sufficient observed landmarks; child proposals need
the same local consistency checks. Every newly observed edge, portal ID and
spawn capability continues to validate the hypothesis. Contradiction permanently
returns the process to general mode and is passed in future birth packets and
contact reports. Changed dimensions/start layouts cannot enter a catalogue path.
A previously unseen map that matches every visible feature cannot be conclusively
rejected until a different feature is seen. Catalogue predictions therefore guide
travel only, never movement legality, rescue safety, pearl presence or live enemies.

All ten images were inspected; exact resource ranges and layouts come from their
local official .map files. Ten generated strategy headers contain all map data.
The separate atlas module handles recognition. The general planner retains
collision, sprint, body, portal, grower, trade and rescue rules. Resource routes
use graph distance and expected regional spawn rates, current crowding and
persistent approach targets; each map has its own travel pull/holding period.
Full thresholds and per-map tactics are in Gamplan section5.

Rejected Thor's dimensions-only classification, its ambiguous Devil/Dilemma head
rule, guessed ongoing enemy positions, small population caps and ordinary split
thresholds above the grower threshold. Raising sprint cost would discourage
sprints, despite the text claiming the opposite. The document's Slithery starter
list omits one B dragon; generated data preserves all fourteen starters. Its
10–20% win-rate projections have no supporting controlled measurements.

Child education has a physical limit: on the tested bent split, none of four
sonar rays reaches the child. Sending the birth packet on all four rays improves
coverage but cannot remove this limit. Missing education means general mode,
not a position-based guess. Positive contact reports are independently validated;
a permanent negative cannot be overwritten. Sonar error checks are not security
against an opponent forging the protocol; uncertain remote knowledge cannot
turn an unsafe move into a legal one.

## Fresh ranked evidence: Link, game630369

Pulled the recent completed ranked series and studied its Slithery game, with
501 independent body/snapshot checks. Borgor is teamB, despite its initial IDs
being the even set. First100 pearl collection was **202 versus54**, and final
population/total length **64/263 versus39/159**. The game was lost on longest
living dragon **15 versus19**. This example does not support universal opening
fearfulness; adding indiscriminate food hunger would optimize the wrong result.

The original coil's retained lineage went25→23 at round0,24→22 at137,
22→20 at226,19→17 at316,17→15 at319, then that length15 descendant died by
self-collision at326. These are actual split/death events, not inferred intent.
The new ordinary-growth reserve addresses one observed way crowded populations
prevent scorer rescues; it cannot guarantee that an unseen rear is safe.
The earlier Trophy hunts and Slithery round-two junction losses remain genuine
separate weaknesses. Map knowledge provides better approaches, not immunity.

## Corrections found during controlled validation

The first integrated candidate won1/6 and was rejected. Full replay review found
both long Slithery scorers unable to rescue at population64, and delivered food
reports for pearls the sender had just eaten. Fixed both. Restored quiet growers'
ordinary patience and made collector-area pull require a nearby offering worker.
The corrected general-only check won2/4, both Slithery games, while losing both
Trauma games. A further donor-credit fix counts only pearls on the checked pickup
route rather than the whole hypothetical corpse; both real feeding ID orders
still work, but the limited general-only Trauma recheck remained0/2.

These correctness and specific-rescue gains do **not** establish broad general
strategy superiority. The final map-aware results are reported separately; they
must not be used to hide general-mode uncertainty. No parameter sweeps or
intermediate submissions were performed. The final validation report retains
all tested fingerprints, failed attempts and sample limitations.
