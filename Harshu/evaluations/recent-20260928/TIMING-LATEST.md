# Latest ranked timing

Snapshot: **2026-09-28T10:12:57.741088+00:00**. Active submission: **abyss-v3 / 9700**.

| Version | Attributed completed ranked series | Games W–D–L | Verified count (minimum if attribution is incomplete) | K from that count |
|---|---:|---|---:|---:|
| v1 | 2 | 3–0–7 | 2 | 81.6 |
| v2 | 22 | 39–0–71 | 24 | 24 |
| v3 | 0 | 0–0–0 | 10 | 24 |

Fresh-window deadline: **2026-09-28 12:04:02.102 IST**
or **2026-09-28 06:34:02.102 UTC**. This uses the server's
`kFreshAt` (2026-09-27T18:34:02.102Z), not upload or individual-game start time. Fresh code before
that deadline inherits the last ranked bot's count capped at ten; identical code
retains its own progress. A stronger candidate can still be worth submitting earlier.

2 live/queued ranked series are excluded from completed counts:
478549 (GZHU-AISquad-Machine), 477219 (Wisher).
6 additional completed ranked series have no submission identity in
the current API response and are retained separately in the ledger. Their results
are not assigned to a version by guesswork. Counts above may therefore be lower
bounds. A version already at count ten or greater still has K=24.
Some games can finish while replays download; this timing ledger consistently uses
the listing's snapshot cutoff. Refresh before later submission advice.

Original upload receipts remain in [history.json](../../submissions/history.json).
The earlier [timing audit](TIMING.md) explains the first-ranked timestamp discrepancy.
Local working release: **abyss-v4**. This is separate from the active
server version shown above; inspect upload receipts before claiming it was submitted.
