# Abyss version history

Develop in `Harshu/bot/`. [current.json](current.json) names the local release, not the active server bot. Freeze each release's source, strategy and manifest; never silently replace an archived ZIP with different code. Server version numbers and our release labels are separate.

Latest verified live state: **v4.1 / submission 10120 active**, **Claude v6 / 10600 idle**, at 29 September 2026, 08:53 UTC. **V7 is packaged locally and unsubmitted.** See [timing](../evaluations/v7-review/TIMING.md).

| Release | Status / submission | Main change | Relevant validation |
|---|---|---|---|
| [v1](../../PastSubmissions/20260927_abyss-v1_sub9003/README.md) | Archived; 9003 | Four-dragon cap, frequent small trades, cautious portals | First online debut 0/20; weak local opponents did not predict the failures |
| [v2](../../CurrentActiveSubmission/20260927_abyss-v2_sub9062/README.md) | Archived; 9062 | 48-dragon cap, later splitting, partial-body escapes | Archived online sample 78/200; see timestamped audit for attribution |
| [v3](v3/manifest.json) | Archived; 9700 | All 64 slots, growers, persistent exploration, upward trades and real-tail checks | 22 logic checks; 7/8 native and 2/2 sandbox versus v2 |
| [v4](v4/manifest.json) | Unsubmitted; superseded by v4.1 | Portal continuity/traffic, largest checked rescues, guarded local feeding | 44 logic checks; 4/4 native and 2/2 sandbox versus v3 |
| [v4.1](v4.1/manifest.json) | Verified active; 10120, also identical 10118 | V4 plus last-resort portal escape beside, but never onto, an ally report | 47 logic checks; 2/2 native and 2/2 sandbox versus v3 |
| [v5](v5/manifest.json) | Held experiment, unsubmitted | Food-score/rescue/feed experiments | 60 logic checks; final native Queen 1/2, sandbox Trauma 0/2 versus v4.1 |
| [v6](v6/manifest.json) | Claude's intervening build; 10600 idle | Terrain/corridor changes; reviewed as evidence for v7 | Native 5/8, sandbox 3/8 versus v4.1; status/control encoding defect identified independently |
| [v7](v7/manifest.json) | Next local candidate; unsubmitted | From v4.1: affordable threats, territory/food/frontier scoring, bounded critical replies, repeated-rescue handling, productive-front worker replacement, explicit one/two-turn collector agreements | 72 logic checks; both engine handshake ID orders; 6/8 final sandbox versus v4.1; twelve timing checks |

Online samples contain different opponents and dates and are not controlled comparisons. Local results are not ranked win rates. V7 preserved Queen/Trauma in this small set but did not solve Slithery opening collection; [review and limitations](../evaluations/v7-review/REPORT.md), [exact validation](../evaluations/v7-review/VALIDATION.md).

## Exact source identities

| Release | Source fingerprint |
|---|---|
| v1 | `396bb8d5e5b95ee2c95f15fa3eafa139e24dfa4952d4ccc47354e3ee8ebc44e6` |
| v2 | `28cbd6c113a585daf64b7f2b99c3cf37ce1be06fb98f2dfc92965aa1ca11fce8` |
| v3 | `52ffbaf11ec9244f9dc7333b81a4e18c1ed85b29f0028c21ca837599b070535d` |
| v4 | `9ab45c60f07dc9f6e14bdbeee225a870b765ce9e6328a80abfae627f5db02497` |
| v4.1 | `217f6872e1b068b83481e5cc89777406f6615f3a1428ed2ac975c56d289d6aa8` |
| v5 | `53fbd1fc2c0f6b5c2d8f76971df564bc12031096bc9421017638de1a6b68335c` |
| v6 | `05ec14fdb136b847c2db3271ce0d7a666bbf1691d52f3c3c1106622a91b99853` |
| v7 | `0ce4a44b3c76102afe2813fde172cfe4dae61be1c2936931bf08cc181db35186` |

V6's originally supplied `versions/v6/bot/` contains only two headers; its complete evaluated source is retained under [the immutable build fingerprint](../evaluations/builds/05ec14fdb136b847c2db3271ce0d7a666bbf1691d52f3c3c1106622a91b99853/). Its existing archive was not rewritten. V7 has a complete five-file frozen source, strategy snapshot, focused tests and manifest.

V7's development included an interrupted two-win Trophy check before the feeding phase fix. Those results have their own fingerprint and are excluded from the final eight games. No intermediate v7 release or submission exists. Keep audit builds distinct from released versions.

## Commands from the repository root

```sh
.venv/bin/python Harshu/tools/submissions.py status
.venv/bin/python Harshu/tools/submissions.py package
.venv/bin/python Harshu/tools/submissions.py submit
```

Only run `submit` when an upload is requested. It can activate the bot and records exact bytes, server receipt and timing. Packaging an already frozen name succeeds only for identical bytes. MainBaselineBuild and historical source/ZIPs remain unchanged.
