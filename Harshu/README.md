# Abyss — UNSW Battlecode bot

**V4.1 is packaged and not submitted. V3 / submission 9700 is the latest verified
active bot.** V4.1 passes 47 focused logic checks and won both native Portals
games plus both sandbox Slithery Fight games against frozen v3, without runtime
errors. Sandbox CPU peaked at 25.43 million of 100 million points; memory peaked
at 0.75 MiB. These small fixed-seed checks do not establish a ranked win rate.

Start with [current planned work.md](current%20planned%20work.md) to resume work.
Read [Gamplan.md](Gamplan.md) for every implemented strategy detail,
[documentation/MASTER.md](documentation/MASTER.md) for rules, and the
[v4 review](evaluations/v4-review/REPORT.md) for evidence and limitations.
[Version history](versions/README.md) identifies frozen builds and prior results.
Do not reread the complete documentation unless explicitly requested.

## Package and submission

All commands below run from the repository root, which contains `.venv/` and
`Harshu/`. The installed toolkit is 1.2.2; the submission uses C++20.

```sh
.venv/bin/python Harshu/tools/submissions.py status
.venv/bin/python Harshu/tools/submissions.py package
```

The candidate is [dist/abyss-v4.1.zip](dist/abyss-v4.1.zip), 18,869 bytes, containing
only `bot.toml` and four C++ source/header files. Exact uploaded bytes, server
receipts and timing are kept in [submission history](submissions/README.md).
Submitting can activate the build automatically. Only when an upload is requested:

```sh
.venv/bin/python Harshu/tools/submissions.py submit
```

Authentication is already configured locally. Keep keys out of source files and
chat. This workspace's environment is managed by `uv`; it does not require pip
inside the virtual environment.

## Focused checks

The following compiled logic checks passed for this candidate. They do not
launch bots in matches or run the official engine.

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic Harshu/tests/strategy_test.cpp -o /tmp/battlecode-strategy-test
/tmp/battlecode-strategy-test
```

## Match validation

The [validation report](evaluations/v4-review/VALIDATION.md) records exact builds,
both-side results, CPU and memory, portal failures, rescue lengths and feeding
outcomes. Original v4 won four native and two sandbox games; review found and
fixed one overly cautious portal fallback. V4.1 then won four targeted games.
No additional automatic runs are planned.

For a future targeted comparison, use a fresh output name and the intended
frozen opponent. This example runs two games; do not rerun it automatically:

```sh
.venv/bin/python Harshu/tools/evaluate.py --bot Harshu/bot --opponents Harshu/versions/v3/bot --maps portals --seeds 509 --output Harshu/evaluations/next-check.json --replays
```

A native game does not verify judge CPU points or memory; `--sandbox` measures
those. Native and sandbox decisions can differ because of their search clocks.

Evaluations freeze source under its fingerprint and record maps, sides, seeds,
deaths, errors and resource usage where available. Avoid tuning sweeps and do
not overwrite earlier evaluated opponents or frozen release files.

## Files

- `bot/`: current candidate source.
- `versions/`: frozen source, strategy, manifests and version index.
- `documentation/`: master rules and focused references.
- `evaluations/v4-review/`: named game review and archived replay evidence.
- `submissions/`: receipts, exact uploaded ZIPs and rating timing.
- `tools/`: replay audits, evaluation and submission tracking.
- `tests/`: focused strategy assertions and separate engine/timing checks.
- `maps/`, `baselines/`: local maps and prior opponents; unchanged for v4.
