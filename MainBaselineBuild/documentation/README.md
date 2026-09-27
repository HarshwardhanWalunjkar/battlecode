# Battlecode documentation

**Start with [MASTER.md](MASTER.md)** for bot design and implementation. It is the single-file working reference: core mechanics, runtime/protocol requirements, workflow, strategic deductions, disputed wording, and scenario checks.

Reviewed **2026-09-27**. Coverage: **23/23 pages** linked in the official documentation navigation. These are concise original summaries and technical lookup notes, not a verbatim website archive. Tutorials' repeated examples, animations, and exhaustive library/opcode inventories remain available through source links. No bot, toolkit, or engine was installed or tested as part of this documentation task.

## How to use

- Read the master for normal bot work; follow a focused page only when that subsystem needs more detail.
- Treat **Rule** as source-backed, **Inference** as a strategy hypothesis, and **Verify** as unresolved.
- Keep rules in the master synchronized with focused pages when updating.
- Confirm toolkit/rule freshness before competition; this review date is not a claim about future behavior.

## Page inventory

### Intro

| Local reference | Official page |
| --- | --- |
| [Overview](intro/overview.md) | [overview](https://game.battlecode.au/docs/overview) |
| [Quickstart](intro/quickstart.md) | [quickstart](https://game.battlecode.au/docs/quickstart) |
| [Submitting via Website](intro/submitting-via-website.md) | [submitting](https://game.battlecode.au/docs/submitting) |

### Game Rules

| Local reference | Official page |
| --- | --- |
| [Structure](game-rules/structure.md) | [structure](https://game.battlecode.au/docs/structure) |
| [Game Map](game-rules/game-map.md) | [map-info](https://game.battlecode.au/docs/map-info) |
| [Pearls](game-rules/pearls.md) | [pearls](https://game.battlecode.au/docs/pearls) |
| [Kelp and Portals](game-rules/kelp-and-portals.md) | [kelp-and-portals](https://game.battlecode.au/docs/kelp-and-portals) |
| [Vision](game-rules/vision.md) | [vision](https://game.battlecode.au/docs/vision) |
| [Movement](game-rules/movement.md) | [movement](https://game.battlecode.au/docs/movement) |
| [Splitting](game-rules/splitting.md) | [splitting](https://game.battlecode.au/docs/splitting) |
| [Sonar](game-rules/sonar.md) | [sonar](https://game.battlecode.au/docs/sonar) |
| [Death](game-rules/death.md) | [death](https://game.battlecode.au/docs/death) |

### Competing

| Local reference | Official page |
| --- | --- |
| [Game Format](competing/game-format.md) | [game-format](https://game.battlecode.au/docs/game-format) |
| [ELO System](competing/elo-system.md) | [elo](https://game.battlecode.au/docs/elo) |

### Advanced

| Local reference | Official page |
| --- | --- |
| [CLI](advanced/cli.md) | [cli](https://game.battlecode.au/docs/cli) |
| [Execution Order](advanced/execution-order.md) | [execution-order](https://game.battlecode.au/docs/execution-order) |
| [Timeouts](advanced/timeouts.md) | [timeouts](https://game.battlecode.au/docs/timeouts) |
| [Standard Library](advanced/standard-library.md) | [libraries](https://game.battlecode.au/docs/libraries) |
| [Helper Reference](advanced/helper-reference.md) | [helper](https://game.battlecode.au/docs/helper) |
| [IO Protocol](advanced/io-protocol.md) | [protocol](https://game.battlecode.au/docs/protocol) |
| [Protocol Upgrade](advanced/protocol-upgrade.md) | [protocol-upgrade](https://game.battlecode.au/docs/protocol-upgrade) |
| [Map Files](advanced/map-files.md) | [map-files](https://game.battlecode.au/docs/map-files) |
| [API](advanced/api.md) | [api](https://game.battlecode.au/docs/api) |

## Maintenance

Compare the official sidebar with this inventory, review changed pages, update the relevant master sections and supplemental notes, and check relative links/anchors. Preserve explicit uncertainty until a reproducible engine experiment resolves it. Record the tested toolkit version and seed with any future behavioral correction.
