# Submission timing

Use `.venv/bin/python tools/submissions.py status` before discussing timing.

Upload through `.venv/bin/python tools/submissions.py submit` so the exact code, local time, server version and outcome enter `history.json`. An upload can activate automatically once built. The wrapper retains an archive and records ambiguous network outcomes rather than retrying blindly.

`sync` reads server submissions and up to 200 recent battles into a local snapshot. Until the account's actual fields/history are verified, it does not guess the start of an Elo window. The snapshot is excluded from version control.

After verifying a ranked battle, record it once:

```sh
.venv/bin/python tools/submissions.py record-ranked --version VERSION --battle-id ID --time TIMESTAMP_WITH_TIMEZONE --count-before COUNT
```

Add `--fresh-window` only to a confirmed first-ranked event whose starting count was zero and which starts the team's fresh-bot window. Count battles, not individual games. This is not needed for every game of a five-game series.

The latest verified active upload is v2 / 9062. v3 is packaged locally. Ranked
history is reconciled from the API in `history.json`; see the
[latest timing report](../evaluations/recent-20260928/TIMING-LATEST.md) for counts,
queued games and the server-derived fresh-window deadline. The historical first
window ends on 28 September at 12:04:02.102 IST. Refresh before later advice.

After downloading a new snapshot and all relevant game details, run
`../.venv/bin/python tools/refresh_timing.py` from `Harshu/` to reconcile the
ledger. The first individual game start and the server fresh-window marker are
stored separately. Live series never count as completed ranked series.
