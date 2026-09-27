#!/usr/bin/env python3
"""Build the release report from the recorded games; reject mixed code versions."""
from pathlib import Path
import collections,json
ROOT=Path(__file__).resolve().parents[1]
manifest=json.loads((ROOT/'dist/manifest.json').read_text())
suites=['large-sandbox','release-sandbox','remaining-sandbox'];rows=[]
for name in suites:
    data=json.loads((ROOT/'evaluations'/f'{name}.json').read_text())
    assert data['sandbox'] and data['immutable_builds']
    assert data['fingerprints']['bot']==manifest['fingerprint'],(name,'different code')
    rows.extend(data['records'])
wins=sum(x['score']==1 for x in rows);draws=sum(x['score']==.5 for x in rows)
points=max(x[x['bot_side']+'_budget']['max_points'] for x in rows)
memory=max(x[x['bot_side']+'_budget']['max_memory'] for x in rows)
turns=sum(x[x['bot_side']+'_budget']['turns'] for x in rows)
errors=sum(len(x['errors']) for x in rows)
text=f'''# Abyss v1 evaluation report

Evaluated 2026-09-27 with `unswbc==1.1.0` and `wasmtime==49.0.0`.

## Release result

**{wins} wins, {draws} draws, {len(rows)-wins-draws} losses in {len(rows)} official-sandbox games** across all 13 bundled maps. Both team sides were tested. These are locally written opponents, not human-ladder matches, so the result does not predict a particular rating or prove optimal play.

The ZIP's exact source fingerprint matches every game included below:

```text
{manifest['fingerprint']}
```

Across **{turns:,} bot turns**, peak cost was **{points:,} / 100,000,000 points** and peak reported memory was **{memory:,} / 48 MB**. There were **{errors} runtime errors** across either side. Our bot had **zero no-valid-action deaths**. Ordinary collision deaths still occurred; survival is not guaranteed.

## Opponents

The food-focused opponent takes legal short routes, values food/room, and discounts danger. The head-hunter also pursues enemies, creates up to four dragons through normal splits, and takes head trades when it has another survivor. Both share observation and move-checking utilities with our bot, so they are useful strategy opponents but are not independent validations of those utilities. The separate official-engine probes provide that check.

| Opponent | Games | Wins | Draws | Losses |
| --- | ---: | ---: | ---: | ---: |
'''
for opponent in sorted({x['opponent'] for x in rows}):
    r=[x for x in rows if x['opponent']==opponent];w=sum(x['score']==1 for x in r);d=sum(x['score']==.5 for x in r)
    text+=f'| {opponent} | {len(r)} | {w} | {d} | {len(r)-w-d} |\n'
text+='''
## Map coverage

These are small samples per map, not reliable estimates of each map's true win probability. In particular, both `default` games were lost and deserve attention after real opponent replays become available.

| Map | Games | Wins | Draws | Losses |
| --- | ---: | ---: | ---: | ---: |
'''
for m in sorted({x['map'] for x in rows}):
    r=[x for x in rows if x['map']==m];w=sum(x['score']==1 for x in r);d=sum(x['score']==.5 for x in r)
    text+=f'| {m} | {len(r)} | {w} | {d} | {len(r)-w-d} |\n'
text+='''
## Why this version was selected

The initial candidate won 53/72 native development games, but only 7/24 against the head-hunter. That exposed the need for earlier insurance, small-dragon attacks, possible split-tail threats, and a stronger preference for sheltered positions.

The selected version won 12/24 native games in its own development suite, then received the separate sandbox checks above. The suite compositions and seeds differ; do not treat those raw percentages as a controlled before/after measurement.

A deeper search (16 turns, ten retained continuations) won only 3/8 native head-to-head games against the selected shorter search, while costing more work. Splitting as soon as length four was reached won only 3/8 against the selected policy and 2/8 against the head-hunter. We retained ten-turn search, seven continuations, and ordinary splitting from length six. These samples support a practical release choice, not a universal ranking of all strategies.

`variants-native.json` is **excluded from selection evidence**: an early evaluation process reused a mutable executable while another run rebuilt it. The harness was corrected to freeze each build by its source fingerprint. All release suites use immutable builds. Other native development suites remain exploratory and are not mixed into the release win total.

## Tests and packaging

- 13 focused C++ transition/message scenarios passed.
- Six direct official-engine scenarios confirmed zero-based rounds, paired portal traversal, sprint payment, self-tail collision, same-round children with inherited protocol 3, and malformed-line preservation.
- Eight submission-timing checks passed, including timezones, duplicate-code history, hourly allowance and first-ranked window timing.
- The release archive is checked against its exact intended file list and source bytes, then run from a fresh directory with `--sandbox`. See `packaged-smoke.log` for the outcome; it is a packaging check, not an additional independent evaluation sample.
- `tests/audit_predictions.py` checks wall/self deaths against the bot's own certainty claims on `stronghold`; its saved result is in `prediction-audit.log`.

## Known limits and next evidence

Future enemy movement is approximated, not fully searched. Threat lookahead stops at three steps and does not cover every possible chain of splitting. Portal exits outside current sight remain unknown. Echo-based sonar scouting and symmetry inference are not implemented. Messages have a check value, not protection against deliberate forgery. A longer search can still miss traps. Deliberate head trades can be bad if the unseen enemy team is much stronger than expected.

The bot avoids publishing its route scores in replays, but observable play and status messages can be studied. It does not sacrifice performance to hide behavior. Native wall-clock truncation can differ from the judge's work clock; the release claims rely on sandbox runs.

The next useful evidence is actual ladder replays: classify losses into resource starvation, avoidable head trades, trapping, and poor expansion before changing the corresponding game-plan section. Do not retune everything after one five-game result.

## Reproducible records

- [Large-map sandbox suite](large-sandbox.json), seed 73; [replays](large-sandbox-replays/).
- [Main release suite](release-sandbox.json), seed 211; [replays](release-sandbox-replays/).
- [Remaining-map suite](remaining-sandbox.json), seed 307; [replays](remaining-sandbox-replays/).
- [Complete plain-English strategy](../Gamplan.md).
- [Submission manifest](../dist/manifest.json).

No live submission or ranked battle was performed. The user confirmed no prior account submissions or ranked play. The submission ledger is ready; the first ranked timestamp has not happened yet.
'''
(ROOT/'evaluations/REPORT.md').write_text(text)
print('Wrote evaluations/REPORT.md; release fingerprints agree')
