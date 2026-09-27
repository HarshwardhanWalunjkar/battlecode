# Submission timing

Use `.venv/bin/python tools/submissions.py status` before discussing timing.

Upload through `.venv/bin/python tools/submissions.py submit` so the exact code, local time, server version and outcome enter `history.json`. An upload can activate automatically once built. The wrapper retains an archive and records ambiguous network outcomes rather than retrying blindly.

`sync` reads server submissions and up to 200 recent battles into a local snapshot. Until the account's actual fields/history are verified, it does not guess the start of an Elo window. The snapshot is excluded from version control.

After verifying a ranked battle, record it once:

```sh
.venv/bin/python tools/submissions.py record-ranked --version VERSION --battle-id ID --time TIMESTAMP_WITH_TIMEZONE --count-before COUNT
```

Add `--fresh-window` only to a confirmed first-ranked event whose starting count was zero and which starts the team's fresh-bot window. Count battles, not individual games. This is not needed for every game of a five-game series.

The user initially confirmed no earlier submissions or ranked games. Version 1
was subsequently uploaded through the wrapper at 2026-09-27 17:07 UTC (22:37 IST)
and is now active. The loss-audit server snapshot contains two unranked series
(409862 and 409982), 20 games in total, and no ranked battles. The submission's
`kStart` and `kFreshAt` are null. No 12-hour fresh-bot deadline can be calculated
from these unranked games. Refresh before giving later timing advice, because
autoscrims and user activity may change the account state.
