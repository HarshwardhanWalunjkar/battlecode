# Ranked timing and version audit

Verified against the saved API snapshot fetched **2026-09-27 22:12:25.815 UTC / 2026-09-28 03:42:25.815 IST**. This is a historical cutoff, not a live guarantee. No upload, activation, challenge, or account setting was changed.

## Submission record

| Version | Submission | Server upload time (UTC) | First individual ranked game started (UTC) | Ranked series completed | Effective count after them | Server fresh-window marker |
|---|---:|---|---|---:|---:|---|
| abyss-v1 | 9003 | 2026-09-27 17:07:28.690 UTC | 2026-09-27 18:01:15.921 UTC | 2 | 2 | 2026-09-27 18:34:02.102 UTC |
| abyss-v2 | 9062 | 2026-09-27 18:06:20.040 UTC | 2026-09-27 18:45:33.340 UTC | 11 | 13 | None; inherited count |

**v1:** server starting count `0`; two completed ranked series leave count `2`. **v2:** server starting count `2`; eleven completed ranked series leave count `13`. The count is per five-game series. v2 is active. Its two queued series have not been counted. The next completed v2 series uses the minimum rating multiplier, **K = 24**.

Both version folders were checked against their ZIP contents: every archived source file matches its ZIP. v1 is 13,446 bytes and fingerprint `396bb8d5e5b95ee2c95f15fa3eafa139e24dfa4952d4ccc47354e3ee8ebc44e6`; v2 is 13,956 bytes and fingerprint `28cbd6c113a585daf64b7f2b99c3cf37ce1be06fb98f2dfc92965aa1ca11fce8`. These are our local file fingerprints, not the server’s separately calculated `sourceHash`.

## The next fresh-code window

The authoritative server marker is **2026-09-27 18:34:02.102 UTC / 2026-09-28 00:04:02.102 IST** on v1. Adding 12 hours gives **2026-09-28 06:34:02.102 UTC / 2026-09-28 12:04:02.102 IST**.

The earliest ranked game began at 18:01:15.921 UTC, and v2 was uploaded at 18:06:20.040 UTC. Neither timestamp starts this recorded window. The first ranked series to finish was 413797 at 18:34:02.074 UTC; the server set the fresh marker 28 milliseconds later. Using the game start would incorrectly bring the deadline forward by almost 33 minutes.

Before that deadline, genuinely different code inherits the last ranked bot’s count, capped at ten; with the verified v2 history that means a new bot starts at count ten and K = 24. After the deadline, genuinely different code may qualify for a fresh starting count when it first enters ranked play, provided no newer fresh window has begun. Identical code retains its historical progress regardless of timing. Refresh the account before making a timing decision, because queued games and later uploads can change the relevant state.

An improved bot can still be uploaded during this window. Waiting is not automatically better: a high rating multiplier increases losses as well as gains. Upload eligibility is separate from the rating window. Only two accepted uploads are in the complete recorded history; both precede the snapshot by more than one hour, so the recorded last-hour upload count is zero of twelve.

## Every completed ranked series

The table uses series completion order because rating changes and completed-battle counts apply to series. Submission IDs come from the saved match details, so the two queued-at-18:00 autoscrims are correctly attributed to v1 even though they finished after v2 was uploaded.

| Representative battle ID | Opponent | Version | Series completed (UTC, 27 Sep) | Games W–D–L | Elo change | Effective count before → after |
|---:|---|---|---|---|---:|---|
| [413797](413797.json) | #define int 龍龍 | v1 | 18:34:02.074 | 2–0–3 | -10 | 0 → 1 |
| [412497](412497.json) | Kraken the Code | v1 | 18:35:08.042 | 1–0–4 | -24 | 1 → 2 |
| [415847](415847.json) | Low Cortisol | v2 | 18:46:22.765 | 1–0–4 | -24 | 2 → 3 |
| [417017](417017.json) | tridev6509 | v2 | 19:12:20.618 | 2–0–3 | -8 | 3 → 4 |
| [417040](417040.json) | tridev6509 | v2 | 19:13:22.598 | 3–0–2 | +7 | 4 → 5 |
| [417047](417047.json) | tridev6509 | v2 | 19:14:24.515 | 1–0–4 | -19 | 5 → 6 |
| [417186](417186.json) | tridev6509 | v2 | 19:15:54.631 | 2–0–3 | -4 | 6 → 7 |
| [421835](421835.json) | tridev6509 | v2 | 20:27:57.927 | 1–0–4 | -13 | 7 → 8 |
| [421845](421845.json) | tridev6509 | v2 | 20:28:27.760 | 3–0–2 | +6 | 8 → 9 |
| [420558](420558.json) | Wapow | v2 | 20:29:26.334 | 1–0–4 | -9 | 9 → 10 |
| [421908](421908.json) | tridev6509 | v2 | 20:30:16.824 | 2–0–3 | -1 | 10 → 11 |
| [419283](419283.json) | just bored | v2 | 20:34:22.126 | 2–0–3 | -2 | 11 → 12 |
| [421981](421981.json) | tridev6509 | v2 | 20:34:43.532 | 0–0–5 | -11 | 12 → 13 |

v1 ranked total: **3 wins / 7 losses**, two lost series, **−34 Elo**. v2 ranked total: **18 wins / 37 losses**, two won and nine lost series, **−78 Elo**. Overall ranked total: **21 wins / 44 losses**, **−112 Elo**, consistent with the initial 1500 rating becoming 1388. Unranked games are excluded.

## Queued at the snapshot

| Battle ID | Opponent | Submission | Requested | Games |
|---:|---|---|---|---:|
| [426986](426986.json) | AI Warriors | v2 / 9062 | 2026-09-27 22:00:00.000 UTC | 5 |
| [425701](425701.json) | vc? | v2 / 9062 | 2026-09-27 22:00:00.000 UTC | 5 |

These ten games have no results or replays in this snapshot. They can change the count and rating once completed; they do not create a new fresh-code window merely by running the same active submission.

## Evidence and limits

- [Snapshot](snapshot.json): submissions including `kStart` and `kFreshAt`, and all twenty returned series since the account began.
- Individual battle JSON files: submission IDs, request/start/completion times, and series members. The first two v1 series and first v2 series have all five game details, so the initial game-start times above are directly verified.
- [Updated timing history](../../submissions/history.json): thirteen completed ranked entries, latest server states, and authoritative fresh-window deadline. Original upload receipts were retained.
- [Rules reference](../../documentation/MASTER.md#competition) and [focused Elo note](../../documentation/competing/elo-system.md). No complete-documentation reread was needed.

The API snapshot gives no separate live “effective count” field. Counts above are a derivation from the authoritative starting counts plus the fully recorded completed series. Do not treat the two pending series as completed or infer rating eligibility from upload time.
