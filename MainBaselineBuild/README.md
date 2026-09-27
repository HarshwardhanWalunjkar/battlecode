# Abyss — UNSW Battlecode bot (team baseline)

This is the team's shared baseline. Copy it into your own folder to experiment. Read [Gamplan.md](Gamplan.md) for the complete strategy in plain English, and [documentation/MASTER.md](documentation/MASTER.md) for the game rules reference.

The active submission is **abyss-v2** (submission 9062, uploaded 2026-09-27). It won 55/64 local games against baselines and 21/32 head-to-head against v1.

## Setup

The project uses C++20 and the official `unswbc` toolkit (version 1.2.2), pinned in `requirements.txt`.

```sh
uv venv .venv
uv pip install --python .venv/bin/python -r requirements.txt
```

If the toolkit's updater complains about pip, update with uv directly:

```sh
uv pip install --python .venv/bin/python 'unswbc==1.2.2'
```

## Run a game

```sh
UNSWBC_NO_UPDATE=1 .venv/bin/unswbc run maps/arena.map bot bot --seed 101 -v
```

Run against a baseline opponent:

```sh
UNSWBC_NO_UPDATE=1 .venv/bin/unswbc run maps/arena.map bot baselines/hunter --seed 101 -v
```

Add `--sandbox` for judge-equivalent budget enforcement (slower but required before submitting).

## Run tests

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic tests/strategy_test.cpp -o /tmp/battlecode-strategy-test
/tmp/battlecode-strategy-test
.venv/bin/python tests/engine_contract.py
```

## Run an evaluation

```sh
UNSWBC_NO_UPDATE=1 .venv/bin/python tools/evaluate.py \
  --opponents baselines/greedy baselines/hunter \
  --maps arena default_small autarky devil dilemma stronghold portals slithery_fight \
  --seeds 101 42 \
  --output evaluations/my-results.json
```

This runs both sides on every map/seed/opponent combination and prints a win/loss summary. Each run freezes the source by content fingerprint so editing mid-evaluation can't affect running games.

## Package and submit

```sh
.venv/bin/python tools/submissions.py package
```

This creates `dist/abyss-v1.zip` containing only the four source files plus `bot.toml` (no evaluations, strategy notes, or keys). To upload:

```sh
.venv/bin/unswbc auth set YOUR_KEY
.venv/bin/python tools/submissions.py submit --name abyss-v2 --description "your description"
```

Get your API key from the [team page](https://game.battlecode.au/team). Do not put it in source files.

## Project structure

```
bot/                 The submission. This is what gets packaged and uploaded.
  main.cpp           Game loop: init, observe, choose, commit
  strategy.hpp       The brain: terrain caching, search, splitting, portals, sonar (~400 lines)
  config.hpp         All tunable policy constants (thresholds, penalties, limits)
  helper.hpp         Official SDK helper (do not modify)
  bot.toml           Project manifest

maps/                All 15 bundled maps (13 original + portals, slithery_fight from 1.2.2)
baselines/           Opponents for local testing (greedy, hunter, and frozen earlier variants)
tools/               Evaluation harness, replay analysis, submission packaging
tests/               Unit tests (C++ strategy scenarios, engine contract, submission timing)
documentation/       Game rules reference (MASTER.md is the single-file summary of all 23 doc pages)
evaluations/         Saved evaluation results and reports
dist/                Packaged ZIP and content manifest
Gamplan.md           Complete plain-English strategy — start here
requirements.txt     Toolkit version pin
```

## Current bot overview

The bot uses a beam search (7 beams, 10 turns ahead, 14 candidate routes) to pick each move. Key design choices:

- **Swarm expansion**: splits at length 4, up to 48 dragons, through round 420
- **Selective head trades**: only trades when allies survive or as a draw-chance last resort
- **Portal exploration**: takes blind portals when local area is stale or unpromising
- **Long-body handling**: sheds unseen tail portions early instead of disabling all actions
- **Threat scoring**: 130/36/6 points for 1/2/3 step enemy distance (low far-threat penalties so food/room scoring still works against swarms)

See [Gamplan.md](Gamplan.md) for every threshold, exception, and tradeoff.

## How to experiment

1. Copy this entire folder into your own workspace (e.g. `cp -r MainBaselineBuild/ YourName/`)
2. Edit `bot/config.hpp` to change constants, or `bot/strategy.hpp` to change logic
3. Run games against the baseline: `UNSWBC_NO_UPDATE=1 .venv/bin/unswbc run maps/arena.map YourName/bot MainBaselineBuild/bot --seed 101 -v`
4. Run the evaluation harness to compare systematically
5. If your version wins consistently, it becomes the new team submission candidate
