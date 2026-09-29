# Submission timing — Jormungandr-v1 preparation

Verified live at **2026-09-29T22:06:09.386247+00:00**. Active submission is **11413 / abyss-v7**,
uploaded 29 September 2026 at **09:12:25 UTC / 14:42:25 IST**. The server reports
50 wins and 70 losses at this snapshot; those are mixed ranked/unranked games,
not a controlled benchmark.

The authoritative `kFreshAt` is **29 September 2026 10:19:30.416 UTC**.
Its twelve-hour window expires **29 September 22:19:30 UTC / 30 September
03:49:30 IST**. A different bot entering ranked play within that window inherits
the applicable count; uploading itself does not start a fresh window. At this snapshot the window had thirteen minutes remaining. This is about fresh-code Elo sensitivity,
not upload permission; check the actual clock before applying that advice. Exact effective counts are not guessed from aggregate wins.

Jormungandr-v1 has not been uploaded by this task. No accepted uploads were
recorded in the last hour at the check. Refresh before relying on this timing:

```sh
.venv/bin/python Harshu/tools/submissions.py sync
.venv/bin/python Harshu/tools/submissions.py status
```

The [ledger](../../submissions/history.json) and
[server snapshot](../../submissions/server-snapshot.json) retain the authoritative
markers and v7 receipt. Frozen historical manifests describe their original
release snapshots and have not been silently rewritten.
