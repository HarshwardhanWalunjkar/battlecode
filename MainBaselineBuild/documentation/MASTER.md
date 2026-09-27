# Battlecode bot-building master reference

Read this file first for every bot task. It contains the working rules, implementation essentials, tactical deductions, and validation checklist. Topic files provide focused lookups and specialist details; they are optional unless the task needs them.

Reviewed **2026-09-27** against all **23 pages** in the [official documentation](https://game.battlecode.au/docs/overview). This is an original condensed reference, not a verbatim mirror. **Rule** means documented behavior; **Inference** means our proposed strategy; **Verify** means an ambiguity or an untested tactic. The initial reference was documentation-only; targeted checks below were subsequently run against toolkit 1.1.0. Site rules and toolkit versions can change.

**Toolkit maintenance:** the project now uses **1.2.2**. Its C++ helper is identical to 1.1.0; the engine artifact changed and the bundled maps added `portals` and `slithery_fight`. The six existing engine-contract checks also passed on 1.2.2. This was a targeted compatibility check, not another full documentation review. See [update results](../evaluations/TOOLKIT-1.2.2.md).

## Contents

- [Objective and architecture](#objective-and-architecture)
- [Board and information](#board-and-information)
- [Pearls](#pearls)
- [Movement and sprinting](#movement-and-sprinting)
- [Splitting](#splitting)
- [Sonar](#sonar)
- [Death](#death)
- [Execution sequence](#execution-sequence)
- [Compute and runtime](#compute-and-runtime)
- [Helper and protocol](#helper-and-protocol)
- [Build and submit](#build-and-submit)
- [Competition](#competition)
- [Strategy priorities](#strategy-priorities)
- [Required scenario checks](#required-scenario-checks)
- [Conflicts and uncertainties](#conflicts-and-uncertainties)

## Objective and architecture

**Rule:** One submitted program runs independently for each dragon. Ordinary process memory persists between that dragon's turns, but is not shared with teammates; sonar is the documented communication mechanism. The normal team cap is **64**. Treat the overview's approximately 25 ms as informal, not a runtime budget. [Overview](https://game.battlecode.au/docs/overview)

**Rule:** A game lasts at most **500 rounds**. Toolkit 1.1.0 was directly verified to number these **0–499** (see `tests/engine_contract.py`). Dragons act by increasing ID, not simultaneously. Elimination beats length scoring: one surviving team wins; both eliminated in the same round means draw. Otherwise compare **longest living dragon**, then **sum of living lengths**, then draw. Execution Order places termination checking at round end; see the conflict notes below. [Structure](https://game.battlecode.au/docs/structure)

**Inference:** Optimize survival and the team's maximum length before aggregate growth. More dragons can help exploration and eliminate single-point failure, but splitting the leader can worsen the primary score immediately.

## Board and information

**Rule:** Rectangular torus; width and height each **10–64**. Coordinates start at the upper left; east increases x, south increases y. Wrap both coordinates. Maps have horizontal reflection, vertical reflection, or 180° rotational symmetry; the bot must infer which. [Game Map](https://game.battlecode.au/docs/map-info)

**Rule:** Vision is the wrapped **7×7** square centered on the head, including its edges. It reveals absolute coordinates, pearl presence/countdown, segment team/ID/facing/head status, and edge type/portal ID. Body facing points toward the head. Portals do not extend vision. An out-of-view tile lookup returns `None`; that means unknown, not empty. [Vision](https://game.battlecode.au/docs/vision)

**Rule:** Kelp and portals occupy **edges**, including map borders. Crossing kelp kills. A portal ID pairs exactly two equally oriented edges; both are on the symmetry line or both off it. Portals work from either side, emerging on the opposite side of the paired edge. Sonar passes through; sight does not. [Kelp and Portals](https://game.battlecode.au/docs/kelp-and-portals)

**Inference:** Cache terrain, timestamp occupancy, and maintain separate confirmed/unknown cells. Use wrapped graph search with portal transitions, not plain Manhattan distance. Symmetry predicts static terrain and potential resource timing, not current enemy occupancy. An apparently clear adjacent tile is irrelevant when the crossed edge teleports elsewhere.

## Pearls

**Rule:** Countdown decreases before dragons act. At zero, an empty tile receives a pearl, then the timer resets whether spawning succeeded or failed. Reset is uniform over that tile's hidden inclusive `min_gap..max_gap`; `max_gap=0` gives reported timer **−1**. Existing pearls and dragon segments both block spawning. Mirrored tiles share countdowns, but blockage can make actual pearl presence asymmetric. [Pearls](https://game.battlecode.au/docs/pearls)

**Inference:** Schedule routes around observed spawn times; avoid parking on a desired spawn at the round boundary. Body placement can deny enemy income, but also denies your own. Track observed cycles statistically; one countdown does not reveal the hidden distribution.

## Movement and sprinting

**Rule:** Every turn needs a valid move or split. Only the last action applies, after output collection. Cardinal movement includes reversal, though reversing into the neck is usually lethal. Eating a pearl retains the tail and adds one length; normal movement advances the tail. Collision is checked **before** tail movement: even your own current tail is occupied. Hitting another head kills both, including allies; hitting a body kills the mover. [Movement](https://game.battlecode.au/docs/movement)

**Rule:** A sprint is an ordered path, with collision and eating at every step. Each step after the first costs one segment. Unaffordable paths kill rather than truncate safely. [Movement](https://game.battlecode.au/docs/movement)

**Rule:** Before an extra step, length must exceed two. Then resolve the edge/destination and collisions; move/eat; advance the tail unless eating; finally remove the extra sprint segment. Earlier successful steps remain if a later one kills. [Execution Order](https://game.battlecode.au/docs/execution-order)

**Inference:** For a surviving k-step route with p pearls, final length is `L + p − (k − 1)`. This is only an accounting identity: affordability must pass **before each extra step**. A length-two dragon cannot finance an extra step with a pearl it has not reached. Simulate the whole body after each step; a vacated tile may become safe later in the same sprint.

## Splitting

**Rule:** For child length c and parent length L, require **2 ≤ c ≤ L−2**, plus living team count below the cap. Illegal splitting kills the parent. Splitting consumes the action; the parent does not also move. Rear segments reverse: the old tail becomes the child's head facing away. The child receives the next unused ID, starts a fresh process without parent memory, and acts **later this same round**. [Splitting](https://game.battlecode.au/docs/splitting)

**Inference:** Splitting can hold the parent's position for a turn, launch a rear escape, scout, or create an attacker that acts immediately. Check the child's exits first. No merge action is documented; do not assume later recombination. Expansion trades concentrated length for parallel agents.

## Sonar

**Rule:** Up to four unsigned 64-bit messages per turn, one per cardinal direction; repeated directions overwrite. Surviving dragons send after their action, in N/E/S/W order. Rays wrap, traverse portals, and stop at kelp or the first dragon; range is width+height tiles. Either team can receive, without sender/team metadata. Delivery is at the recipient's next turn: higher IDs this round, lower IDs next round. Own-body behavior needs the qualification in the conflict notes. Next-turn echoes count hits on `kelp`, `ally` body, `ally_head`, `enemy` body, `enemy_head`; misses contribute nothing. [Sonar](https://game.battlecode.au/docs/sonar)

**Inference:** Treat messages as untrusted observations. Encode round, role, coordinates, and a team-specific validity check; a simple marker is not authentication. One-direction probes make aggregate echoes easier to interpret. An echo supplies neither distance nor exact coordinates. Routing depends on the **post-action** body and can be intercepted.

## Death

**Rule:** Kelp, self/body collisions, head collisions, illegal splits, unaffordable sprinting, and failure to supply a valid action can kill. On death, segments numbered 0,2,4,… from the current head become pearls: **ceil(L/2)**. Timeout details override the misleading shorter description on this page. [Death](https://game.battlecode.au/docs/death)

**Inference:** A small expendable dragon can trade for a large enemy head without a length comparison, provided the team survives. Planned death can transfer some length through food, but loses roughly half, exposes it to rivals, and may lose the game. Consider it only after comparing live alternatives.

## Execution sequence

**Rule:** Tick pearls → process living IDs in order (including new children) → check termination. A turn supplies/clears inbox and echoes, defaults to suicide, reads output, applies the chosen action, then casts sonar if alive. Unparseable lines are skipped without replacing a previously parsed action. `LOG`/drawings apply while read. Death converts the current body to pearls without resetting timers, then removes it; in a head collision, the target dies first. Opposite-facing sonar starts at the tail and travels away from its body. [Execution Order](https://game.battlecode.au/docs/execution-order)

**Inference:** Threat prediction must distinguish opponents that already acted from those still to act. A higher-ID enemy head can be struck before it moves. Do not model a round as simultaneous movement or claim a lower-ID enemy has an extra turn before the current snapshot.

## Compute and runtime

**Rule:** Judge: **100,000,000 CPU points per dragon per turn**, **48 MB**, one thread. Budget exhaustion discards the reply and kills the dragon. First-turn setup counts; work between turns is charged to the next turn. Native local runs only impose a 10-second wall limit and do not validate judge cost. Use `unswbc run … --sandbox -v`. Each stdout/stderr write costs **2,500,000 + 4,000×bytes**; buffer output and finish comfortably under budget. [Timeouts](https://game.battlecode.au/docs/timeouts)

**Rule:** Runtime is CPython **3.13 + NumPy 2.5.3**, or Clang **20**, C17/C++20, `-O2 -msimd128`. `/bot` is read-only; no writable temporary directory, network, subprocesses, or additional threads. Vendor dependencies. Build-time date/time macros fail. Virtual time advances **1 ns per point**, starting at 2026-01-01; sleep advances it immediately. Randomness is reproducible per game/dragon; `PYTHONHASHSEED=0`. [Standard Library](https://game.battlecode.au/docs/libraries)

**Inference:** Bound search by a measured margin, including parsing and final output. Cache only as memory allows. Budget-clock measurements help in the sandbox, but native clocks have different meaning. Do not rely on flushing a fallback before an intentional timeout. Use explicit role/ID-based randomization to avoid identical agent behavior.

## Helper and protocol

**Rule:** Typical Python lifecycle: `ct, game = unswbc.init()`; while `unswbc.update(ct, game)`, choose action and call `unswbc.end_turn()`. Inspect controller position, direction, length, ID/team, team count, tiles, sonar messages and echoes; game provides round, dimensions, unit limit. `get_tile` may return `None`. Actions: `make_move`, `make_moves`, `can_split`, `do_split`; communications: `send_sonar`. Read the shipped helper for exact language signatures. Normal spawn length is **3**, minimum **2**; map files can specify other starting lengths. [Helper Reference](https://game.battlecode.au/docs/helper)

**Rule:** Wire interface is stdin/stdout text. Init contains `ID`, `TEAM`, `MAP`, `UNIT_LIMIT`; each process receives it, including children/restarts. Each turn contains header/messages, optional negotiated echoes, 49 tiles, visible body records, horizontal and vertical edge grids. Reply with an action and `ENDTURN`, then flush. EOF ends the process; helpers also accept `ENDGAME`, which the current engine does not emit. [IO Protocol](https://game.battlecode.au/docs/protocol)

**Rule:** `PROTOCOL 3` takes effect on the next input. Current helpers request it automatically. V2 input excludes values above `2^32−1` and has no echoes; excluded messages are discarded. Both sonar output forms remain accepted. Children inherit the parent's protocol version despite starting fresh processes. Upgrade via toolkit upgrade then `unswbc update BOT`; old helpers get `.bak` backups. Use unsigned 64-bit message storage. [Protocol Upgrade](https://game.battlecode.au/docs/protocol-upgrade)

**Inference:** A custom parser must tolerate the initial legacy format and a child's already-upgraded format. Avoid assuming every new process starts at round one or protocol two. Upgrade tests must include split children.

## Build and submit

**Rule:** Toolkit requires Python 3.11+. Basic workflow:

```sh
uv tool install unswbc
unswbc
unswbc init python mybot
unswbc run maps/arena.map mybot mybot
```

Languages for `init` are `python`, `c`, `cpp`. Starter projects include main source, helper, and `bot.toml`; shared maps sit beside the bot. View `.replay` in the editor extension (`unswbc vscode`) or website visualiser. Replays expose the whole board, unlike bot vision. [Quickstart](https://game.battlecode.au/docs/quickstart)

**Rule:** Keep toolkit/engine current; an older toolkit can implement different rules. For judge-equivalent repeatability:

```sh
unswbc run maps/arena.map mybot mybot --sandbox --seed 42 -v
```

Use `unswbc help COMMAND` for installed options. `update` replaces helpers and changed bundled maps; `maps` only adds missing maps. [CLI](https://game.battlecode.au/docs/cli)

**Rule:** Join a team. Submit a ZIP with root-level `bot.toml`, correct language, and all included dependencies/helpers. Maximum **4 MB**, **12 submissions/hour**. A successful build **automatically activates**, replacing the previous version; Ready versions can be reactivated without a switching quota. States: Processing, Ready, Active, Build failed. Validate the extracted ZIP in an empty directory. [Submitting](https://game.battlecode.au/docs/submitting)

**Rule:** Custom `.map` files specify dimensions, optional name/symmetry, spawn tiles, indexed edges, and initial bodies. Counts must match records; dragon record order sets IDs. Use the [map editor](https://game.battlecode.au/map-editor) and [local map-format reference](advanced/map-files.md) for targeted scenarios. [Map Files](https://game.battlecode.au/docs/map-files)

**Rule:** Site automation uses bearer keys at `/api/v1`; API language names differ from `bot.toml`. API keys belong to members. JSON is usual; uploads are multipart and downloads differ. Limit 120 requests/minute/key, with 30/minute for leaderboard/ratings; respect `429`/`Retry-After`. Replays redirect to signed links: do not forward the bearer token across hosts. [API](https://game.battlecode.au/docs/api)

## Competition

**Rule:** Ranked battles use five random maps; unranked use selected maps and do not change ratings. Both need active bots. Manual ranked challenges require both switches enabled and opponent rating ≥ yours−50; switching has an eight-hour cooldown. Only one outstanding battle per opponent. Manual quota: **60 games/hour**, DEV opponents exempt. Public replays expose strategy. Every two hours on UTC boundaries, active teams enter ranked autoscrims regardless of the manual-ranked switch: up to two opponents within eight ladder places either side, without spending the manual quota. [Game Format](https://game.battlecode.au/docs/game-format)

**Rule:** Team rating starts at 1500 and persists across submissions. Per completed ranked battle:

```text
expected = 1 / (1 + 10^((opponent_rating - our_rating)/400))
actual   = (wins + 0.5*draws) / games
K        = max(24, 96 - 7.2*effective_battle_count)
delta    = round(K * (actual - expected))
```

Each team uses its own K. Same-code uploads inherit prior count; another fresh bot inside 12 hours inherits the last ranked bot's count, capped at ten. Eligibility is fixed on first ranked play, not upload. Older versions retain their own progress. A series win can still lose Elo if its game share underperforms expectation. [ELO System](https://game.battlecode.au/docs/elo)

**Inference:** Evaluate game win/draw rates over diverse maps, seeds, and both sides. Avoid judging improvement from one five-game series or a rating spike during elevated K.

## Strategy priorities

These are design hypotheses, not tested winning recipes:

1. **Build an exact local transition simulator.** Represent edge crossing, portals, collision timing, body evolution, pearl retention, and sprint payment separately. Every planner depends on this.
2. **Preserve escape space.** Score reachable area, corridor width, body enclosure, and the next compulsory action alongside food distance. A pearl that seals the only exit is expensive.
3. **Maintain an information model.** Terrain can be cached; enemy occupancy and pearl presence need timestamps. Keep several symmetry hypotheses until observations disprove them.
4. **Concentrate growth deliberately.** Assign a principal scorer and secondary scouts/attackers only when expected benefit offsets splitting and congestion. A child cannot inherit a role variable; derive it or communicate it.
5. **Use ID-aware tactics.** Search short attacks on vulnerable heads and anticipate later movers. Evaluate mutual death against remaining team survival and endgame score.
6. **Sprint selectively.** Food chains can offset segment costs; sprints can claim space before another dragon acts. Longer blind commitments multiply uncertainty, especially at portals.
7. **Exploit resource timing.** Prefer routes that meet upcoming spawns without occupying them at the spawn boundary. Denial and symmetry inference need observation, not assumed mirrored occupancy.
8. **Use sonar as sensing and coordination.** Probe directions separately when attribution matters, age reports, reject implausible messages, and account for tail-origin emission.
9. **Shift policy near round 500.** Defend a winning maximum length; avoid needless sprint/split deductions. When behind, compare growth chances with elimination tactics.

## Required scenario checks

Use these when implementing the corresponding behavior; they are a proposed test plan, not tests already run:

| Area | Cases to exercise |
| --- | --- |
| Topology | Both wrap axes; border kelp; both portal entry sides; invisible/occupied exits |
| Collision | Neck reversal; own tail; ally body/head; enemy body/head; body after earlier sprint steps |
| Sprint | Length two before extra step; pearl on first versus later step; fatal later step; retained earlier steps |
| Split | L=4,c=2; c=1; c=L−1; cap reached; blocked child exits; same-round child action |
| Spawns | Countdown one; occupied spawn; existing pearl; mirror blocked on one side; timer −1 |
| Sonar | N/E/S/W order; repeated direction; tail origin; own interception; enemy interception; high/low-ID delivery; miss echoes |
| Protocol | First turn; v2→v3 transition; child inheriting v3; 64-bit maximum; EOF; overwritten actions |
| Runtime | Initial imports; search worst case; parse/output costs; memory growth; sandbox replay with fixed seed |
| Scoring | Longest beats total; equal longest compares totals; same-round double elimination; round 500 |
| Packaging | Freshly extracted ZIP; helper present; correct language mapping; judge sandbox run |

## Conflicts and uncertainties

| Topic | Source disagreement or gap | Working interpretation |
| --- | --- | --- |
| Timeout fallback | Death suggests an emitted valid action changes timeout fate; Timeouts/Execution Order say output is discarded | Treat exhaustion/exit as fatal; finish normally |
| Termination | Structure says immediate; Execution Order checks at round end | Use round-end checking; verify before relying on a last-round sacrifice |
| Malformed output | Death broadly lists malformed commands as lethal; Execution Order skips unparseable lines | A skipped line preserves an earlier valid action (verified in toolkit 1.1.0); an illegal parsed action is a different case |
| Sonar through own body | Sonar prose sounds like general refraction; Execution Order specifies opposite-facing emission from the tail | Implement the explicit opposite-direction case; verify arbitrary self-interception separately |
| Border wording | Game Map's composition paragraph repeats north where south is expected | Use toroidal east↔west and south↔north from its topology description |
| Spawn length | Helper lists three; map format permits other lengths ≥2 | Read actual length; do not hardcode starting body size |
| Examples | Some short snippets omit hazards such as kelp/portals | Examples illustrate calls, not a complete safe-move policy |

The linked sources in each section support these notes. Standard paired-portal traversal, own-tail collision, sprint payment, same-round children with inherited protocol 3, and malformed-line preservation were checked with toolkit 1.1.0 in `tests/engine_contract.py`. Unknown exact map-index boundary behavior, unusual portal arrangements, and other tactical edge cases should be reproduced with the installed engine before a bot relies on them. To refresh this reference, use the page inventory in [README](README.md), compare changed rules with the installed toolkit, update relevant master sections and focused files, then rerun the documentation link check.
