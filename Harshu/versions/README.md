# Abyss version history

Keep uploaded releases unchanged. Develop in `Harshu/bot/`; `current.json`
identifies the local candidate, not the active server version. A release ZIP
cannot be replaced with different source under the same name by our packaging
command. Advance the version before packaging another source change.

| Version | Status | Submission | Main strategy | Validation |
|---|---|---:|---|---|
| [v1](../../PastSubmissions/20260927_abyss-v1_sub9003/README.md) | Archived | 9003 | Four-dragon normal cap, frequent small head trades, cautious portals | Original online debut 0/20; 28/38 weak-baseline sandbox games |
| [v2](../../CurrentActiveSubmission/20260927_abyss-v2_sub9062/README.md) | Active online | 9062 | 48-dragon normal cap, splitting until round 420, easier portals and partial-body rescue | Current audit: 75/194 downloaded games won; 34/100 ranked games at latest listing cutoff |
| [v3](v3/manifest.json) | Local candidate, not submitted | — | Growers plus full 64-slot allowance, less crowding, persistent exploration, upward trades, real-tail child checks | 22 strategy checks; 7/8 native and 2/2 sandbox wins against exact v2 |

The online samples use different opponents and dates; these counts are not a
controlled comparison between versions. See the [replay audit and validation](../evaluations/recent-20260928/REPORT.md).

## Exact code identities

- v1: `396bb8d5e5b95ee2c95f15fa3eafa139e24dfa4952d4ccc47354e3ee8ebc44e6`
- v2: `28cbd6c113a585daf64b7f2b99c3cf37ce1be06fb98f2dfc92965aa1ca11fce8`
- v3: `52ffbaf11ec9244f9dc7333b81a4e18c1ed85b29f0028c21ca837599b070535d`

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
It records exact bytes, fingerprint, server version and time. It has not been
run for v3. Check [latest ranked timing](../evaluations/recent-20260928/TIMING-LATEST.md)
before deciding when to use it.
