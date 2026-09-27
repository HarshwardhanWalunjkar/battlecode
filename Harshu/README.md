# Abyss — UNSW Battlecode bot

Read [Gamplan.md](Gamplan.md) for the complete plain-English strategy, [documentation/MASTER.md](documentation/MASTER.md) for rules, and [evaluations/REPORT.md](evaluations/REPORT.md) for measured results and limitations.

## Run locally

The project uses C++20 and the official `unswbc` toolkit, pinned in `requirements.txt`.

```sh
uv venv .venv
uv pip install --python .venv/bin/python -r requirements.txt
UNSWBC_NO_UPDATE=1 .venv/bin/unswbc run maps/arena.map bot baselines/hunter --sandbox --seed 101 -v
```

The workspace already has the toolkit installed. The judge-equivalent `--sandbox` check is required before a release; an ordinary local run does not establish the computation budget.

The current toolkit is 1.2.2. This virtual environment is managed by `uv` and does
not need `pip` installed inside it. If the toolkit's automatic updater reports
`No module named pip`, update with `uv` instead:

```sh
uv pip install --python .venv/bin/python 'unswbc==1.2.2'
```

The original 38-game report remains labeled with its tested version, 1.1.0.
See [the update check](evaluations/TOOLKIT-1.2.2.md) for separate 1.2.2 results.

## Checks

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic tests/strategy_test.cpp -o /tmp/battlecode-strategy-test
/tmp/battlecode-strategy-test
.venv/bin/python tests/engine_contract.py
.venv/bin/python tests/submissions_test.py
```

Run a repeatable two-sided evaluation:

```sh
UNSWBC_NO_UPDATE=1 .venv/bin/python tools/evaluate.py --opponents baselines/greedy baselines/hunter --maps arena default_small autarky devil dilemma stronghold --seeds 101 --sandbox --output evaluations/new-results.json
```

Each evaluation freezes source and builds under its content fingerprint. Editing a bot during evaluation cannot change that run's executable. Results record sides, maps, seeds, fingerprints, deaths, runtime errors and sandbox costs. Native runs are development comparisons, not server-performance guarantees. `tools/make_variants.py` generates experimental opponents; do not rerun it over historical opponents whose results you want to preserve.

## Package and submit

```sh
.venv/bin/python tools/submissions.py package
.venv/bin/python tools/submissions.py status
```

The ZIP is `dist/abyss-v1.zip`. It contains the manifest and four C++ source/header files, with no evaluation history, strategy notes, keys, or replays.

Create an API key on your [team page](https://game.battlecode.au/team) and configure it locally. The key authorizes uploads; do not put it in source files or chat.

```sh
.venv/bin/unswbc auth set YOUR_KEY
.venv/bin/python tools/submissions.py submit --name abyss-v1
```

Use the wrapper for uploads so [submission history](submissions/README.md) stays accurate. Successful builds activate automatically. No live submission was made during development.

## Files

- `bot/`: the submission project. `config.hpp` contains policy choices; `strategy.hpp` implements them; `main.cpp` runs the loop.
- `bot/helper.hpp`: unmodified helper, identical in toolkits 1.1.0 and 1.2.2.
- `maps/`: the original 13 maps and two maps added by toolkit 1.2.2.
- `baselines/`: starter, custom opponents and frozen earlier policies.
- `tools/`: evaluation and submission tracking.
- `tests/`: transition rules, official-engine behavior and submission timing.
- `dist/`: packaged release and exact content manifest.
