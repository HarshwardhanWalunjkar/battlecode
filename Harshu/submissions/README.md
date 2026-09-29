# Submission timing and exact uploads

The [latest v7 timing report](../evaluations/v7-review/TIMING.md) identifies the active server bot and current verified window. The final review snapshot is 29 September 2026, 08:53 UTC: **v4.1 / 10120 active**, Claude's **v6 / 10600 idle**, local **v7 unsubmitted**. The last confirmed fresh window expired at **29 September, 04:22:22.078 IST**. Refresh before advice much later than that snapshot.

From the repository root:

```sh
.venv/bin/python Harshu/tools/submissions.py sync
.venv/bin/python Harshu/tools/submissions.py status
```

`sync` reads submissions and up to 200 recent series, stores raw evidence, and updates the authoritative fresh-window and active-submission markers. It does not guess version attribution or an effective battle count when the API omits submission identity. `status` checks the latest recorded marker as well as historical attributed first-ranked entries, and reports both India time and UTC.

Upload only when requested, through:

```sh
.venv/bin/python Harshu/tools/submissions.py submit
```

This command can activate the new bot after building. It archives the exact ZIP, fingerprint, local upload time and server receipt in `history.json`. Ambiguous network outcomes are retained instead of retried blindly. The server's version number is separate from our release name.

Upload time does not start the 12-hour fresh-bot window. First eligible ranked play does. Identical code does not restart it. Stronger rating changes magnify losses too; the timing advantage does not establish that a bot is stronger.

For a ranked series whose source and effective count have actually been verified:

```sh
.venv/bin/python Harshu/tools/submissions.py record-ranked --version VERSION --battle-id ID --time TIMESTAMP_WITH_TIMEZONE --count-before COUNT
```

Add `--fresh-window` only for a confirmed first-ranked event that starts a new window at count zero. Count series, not individual map games. Recent completed series with missing submission identity stay in `unattributed_ranked_battles`; they are not assigned from upload chronology or current active status. Historical receipts and attributed counts remain intact.

`tools/refresh_timing.py` is the older reconciliation script for the specifically archived `recent-20260928` detail set. It is not a generic refresh command for newer unarchived game details; use `sync` for current server markers.
