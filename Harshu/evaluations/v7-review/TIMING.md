# Submission timing for the v7 review

Read-only server snapshot: **2026-09-29T08:53:49.588877+00:00**. Active: **abyss-v4.1, ID 10120, server version 5**. Claude's **abyss-v6, ID 10600, server version 6**, is idle. Our local v7 has not been uploaded; a future server version number is assigned by the server and need not match its release label.

The v6 upload is preserved in the ledger: local receipt **2026-09-28 18:32:06.032154 UTC**, server upload **2026-09-28 18:32:07.216 UTC**. It inherited starting count ten and has no new `kFreshAt`, so it did not restart the fresh-bot window. Its server starting K is therefore already at the 24-point floor; subsequent battles cannot increase it.

The latest confirmed fresh window belongs to submission 10120 and starts at **2026-09-28 10:52:22.078 UTC**, from `kFreshAt`, not upload time. It expires at:

- **29 September 2026, 04:22:22.078 IST**
- **28 September 2026, 22:52:22.078 UTC**

At this final snapshot the recorded window has expired, and no later fresh marker is present. Different-code v7 may therefore qualify for a new window at its first ranked battle, provided no newer eligible activity occurs in between. A first ranked battle inside an occupied window instead inherits the applicable previous count, capped at ten. Upload alone does not start or reserve a window. High K magnifies losses as well as wins; waiting is not inherently beneficial if the build is weaker.

The snapshot contains 59 ranked series. The ledger preserves 24 previously attributed completed series and 35 completed series without verifiable submission identity; 0 are pending. We do not assign recent games to the currently active bot or to v6 merely because they followed its upload. V4.1's exact current effective count cannot be recovered from `kStart=0` alone. The latest snapshot is [timing-final-snapshot.json](timing-final-snapshot.json); receipts and historical counts are in [history.json](../../submissions/history.json).

There were no accepted uploads in the hour preceding this snapshot. The documented upload quota is twelve per hour. Refresh if acting substantially later:

```sh
.venv/bin/python Harshu/tools/submissions.py sync
.venv/bin/python Harshu/tools/submissions.py status
```

`status` now includes the authoritative fresh-window marker even when new battles cannot be attributed. Previously it could fall back to an older attributed first-ranked battle and incorrectly report that the current window had expired. Twelve timing checks cover the correction and marker refresh. `sync` saves raw server evidence and refreshes authoritative fresh-window/active-submission markers; it does not invent missing battle attribution or completed counts.

Use the wrapper to upload only when requested:

```sh
.venv/bin/python Harshu/tools/submissions.py submit
```

The command can activate the new bot after a successful build and records exact bytes, upload time and server receipt. This review did not run it. No local intermediate v7 packages or staged submissions were created.

Rule reference: [Elo ratings](https://game.battlecode.au/docs/elo), especially new bots and the first-ranked-play window. Existing master rules were used; no complete documentation review was needed.
