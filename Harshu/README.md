# Abyss — UNSW Battlecode bot

**Local v7 is the next candidate, built directly from frozen v4.1.** It won six of eight final sandbox comparisons against v4.1, with zero runtime errors. Queen and Trauma won on both sides; Trophy and Slithery each split 1–1. These are small regression checks, not a ranked win-rate estimate. Slithery's opening worker economy remains a weakness. V7 has not been submitted.

At the final read-only server check, 29 September 2026, 08:53 UTC, **v4.1 / 10120 remained active** and Claude's **v6 / 10600 was idle**. No activation or submission was performed by this task.

## Fast references

- [Gamplan.md](Gamplan.md): every implemented policy, threshold, exception and tradeoff in plain English.
- [documentation/MASTER.md](documentation/MASTER.md): rules to read first; use focused pages only for unresolved questions. Do not reread everything unless requested.
- [current planned work.md](current%20planned%20work.md): completed work and a handoff without restarting the investigation.
- [V7 review](evaluations/v7-review/REPORT.md): seventeen independently reconstructed games, Lozer analysis, accepted/rejected suggestions and preserved strengths.
- [V7 validation](evaluations/v7-review/VALIDATION.md): exact source, eight sandbox games, actual feeding transfers and limitations.
- [Version history](versions/README.md) and [timing](evaluations/v7-review/TIMING.md): local releases, active server identity and eligibility.

## Package and submit

Run from the repository root, containing `.venv/` and `Harshu/`. The toolkit is 1.2.2 and the bot is C++20. V7's package is [abyss-v7.zip](dist/abyss-v7.zip); its [manifest](dist/abyss-v7.manifest.json) identifies the five included files and fingerprint. The frozen source is under [versions/v7](versions/v7/manifest.json).

```sh
.venv/bin/python Harshu/tools/submissions.py sync
.venv/bin/python Harshu/tools/submissions.py status
```

The last confirmed fresh window expired at 04:22:22.078 IST on 29 September. A newer server marker or activity can change later advice; upload time itself does not start the window. `sync` refreshes authoritative markers without inventing battle attribution.

When an upload is requested:

```sh
.venv/bin/python Harshu/tools/submissions.py submit
```

This can activate the bot once built, and records the exact bytes, receipt and timing. Authentication is already configured locally; keep credentials out of files and chat. The environment is managed by `uv` and does not require pip inside `.venv`.

## Verification and future changes

V7 passed 72 warning-clean C++ scenarios, two small official-engine coordination fixtures, twelve timing checks and the predeclared eight sandbox games. Across 81,311 sandbox turns, maximum cost was 25,836,992 CPU points and 917,504 bytes. The full matches contained 63 matched agreed donations and 59 intended head-pearl collections within the promised time. The four misses are traced in the validation report.

Focused checks, when changes justify them:

```sh
g++ -std=c++20 -O2 -Wall -Wextra -Wpedantic Harshu/tests/strategy_test.cpp -o /tmp/abyss-strategy-test
/tmp/abyss-strategy-test
.venv/bin/python Harshu/tests/coordination_engine.py
.venv/bin/python Harshu/tests/submissions_test.py
```

No further automatic match sweep is planned. Use a new result filename and explicit frozen opponent for a future necessary comparison. Native games do not check judge points; sandbox games do. Evaluated builds are stored under their fingerprints, and release ZIPs cannot be replaced with different bytes under the same name.

MainBaselineBuild, older releases and research/ were unchanged. Bot source stays in `bot/`; rule references in `documentation/`; source/strategy snapshots in `versions/`; replay evidence in `evaluations/`; upload receipts and rating history in `submissions/`.
