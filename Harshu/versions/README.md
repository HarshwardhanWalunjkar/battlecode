# Abyss version history

Keep uploaded releases unchanged. Develop in `Harshu/bot/`; `current.json`
identifies the local candidate, not the active server version. A release ZIP
cannot be replaced with different source under the same name by our packaging
command. Advance the version before packaging another source change.

| Version | Status | Submission | Main strategy | Validation |
|---|---|---:|---|---|
| [v1](../../PastSubmissions/20260927_abyss-v1_sub9003/README.md) | Archived | 9003 | Four-dragon normal cap, frequent small head trades, cautious portals | Original online debut 0/20; 28/38 weak-baseline sandbox games |
| [v2](../../CurrentActiveSubmission/20260927_abyss-v2_sub9062/README.md) | Archived; v3 replaced it | 9062 | 48-dragon normal cap, splitting until round 420, easier portals and partial-body rescue | Expanded archive: 78/200 downloaded games won; see timestamped timing report for ranked counts |
| [v3](v3/manifest.json) | Active online | 9700 | Growers plus full 64-slot allowance, less crowding, persistent exploration, upward trades, real-tail child checks | 22 strategy checks; 7/8 native and 2/2 sandbox wins against exact v2; first attributed online sample 5/10 against Lozer |
| [v4](v4/manifest.json) | Superseded locally; never submitted | — | Portal body continuity and traffic warnings; largest checked rescue; early small breeders; food sharing and guarded late feeding | 44 logic checks; 4/4 native and 2/2 sandbox wins against v3 |
| [v4.1](v4.1/manifest.json) | Current packaged candidate; not submitted | — | V4 plus a last-resort portal escape beside an ally report, retaining all direct-occupancy vetoes | 47 logic checks; 2/2 native and 2/2 sandbox wins against v3; maximum 25.43M CPU points, 0.75 MiB |

The online samples use different opponents and dates; these counts are not a
controlled comparison between versions. See the [replay audit and validation](../evaluations/recent-20260928/REPORT.md).

## Exact code identities

- v1: `396bb8d5e5b95ee2c95f15fa3eafa139e24dfa4952d4ccc47354e3ee8ebc44e6`
- v2: `28cbd6c113a585daf64b7f2b99c3cf37ce1be06fb98f2dfc92965aa1ca11fce8`
- v3: `52ffbaf11ec9244f9dc7333b81a4e18c1ed85b29f0028c21ca837599b070535d`
- v4: `9ab45c60f07dc9f6e14bdbeee225a870b765ce9e6328a80abfae627f5db02497`
- v4.1: `217f6872e1b068b83481e5cc89777406f6615f3a1428ed2ac975c56d289d6aa8`

v3 has a [frozen source copy](v3/bot/strategy.hpp),
[plain-English strategy snapshot](v3/Gamplan.md), and
[versioned ZIP](../dist/abyss-v3.zip), 14,778 bytes.
Both native and sandbox results identify the same source fingerprint.

## v3 changes and deliberate limits

- Normal breeders may use all 64 slots but must pass room, crowding and child
  continuation checks. Normal splits stop at round 350.
- Once population reaches four, selected IDs and every length-six-or-longer
  dragon preserve length. Emergencies can override that protection.
- Foodless scouts keep an exploration destination instead of only avoiding
  very recent visits. Growers wait longer before exploring or risking blind portals.
- Small non-growers may deliberately kill a larger visible enemy when at least
  two allies remain afterwards. Equal-size voluntary exchanges are rejected.
- Checked child safety requires the actual known tail. Unseen-tail rescues are
  a separate uncertain fallback, limited to no-move emergencies or long starts.
- Sonar still shares position/length, not map fragments or group attack orders.
  Coordinated swarming is not claimed as implemented.

## Commands from the repository root

```sh
cd Harshu
../.venv/bin/python tools/submissions.py status
../.venv/bin/python tools/submissions.py package
../.venv/bin/python tools/submissions.py submit
```

The submit command uploads the local candidate and may activate it automatically.
It records exact bytes, fingerprint, server version and time. It was run for v3 on 28 September at 05:09:14 UTC. It has not been
run for v4 or v4.1. Check [latest ranked timing](../evaluations/recent-20260928/TIMING-LATEST.md)
before deciding when to use it.

## V4 and v4.1 validation

V4 has a [frozen source](v4/bot/strategy.hpp), [strategy snapshot](v4/Gamplan.md),
[manifest](v4/manifest.json), and [ZIP](../dist/abyss-v4.zip), 18745 bytes.
The [review](../evaluations/v4-review/REPORT.md) evaluates both user note files
and the referenced games. The subsequent [local validation](../evaluations/v4-review/VALIDATION.md)
records all matches, source identities and limitations. Original v4 won six
games against v3 without runtime errors, but a targeted replay exposed an
overly cautious portal rule. V4.1 permits the only remaining blind escape beside
a reported ally when no ordinary move exists; a reported head on the exit and
other hard vetoes still block entry.

V4.1 has its own [frozen source](v4.1/bot/strategy.hpp),
[strategy snapshot](v4.1/Gamplan.md), [manifest](v4.1/manifest.json), and
[ZIP](../dist/abyss-v4.1.zip), 18,869 bytes. Its 54,589 sandbox turns peaked at
25,426,672 CPU points and 786,432 bytes. All four revised-build games passed
without runtime errors or engine notices. The same seed, two maps and one
opponent give useful regression evidence, not a ranked strength estimate.
Prior frozen source and ZIPs are unchanged.
