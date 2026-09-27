# API

[Master reference](../MASTER.md#build-and-submit) · [Index](../README.md)

Source: [Official API](https://game.battlecode.au/docs/api) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Endpoint inventory

Base: `https://game.battlecode.au/api/v1`. Header: `Authorization: Bearer KEY`.

| Method | Path |
| --- | --- |
| GET | `/me`, `/team` |
| GET/POST | `/submissions` |
| GET | `/submissions/:id`, `/submissions/:id/download` |
| POST | `/submissions/:id/activate` |
| GET/POST | `/battles` |
| GET | `/battles/:id`, `/battles/:id/replay` |
| GET | `/leaderboard`, `/ratings`, `/teams`, `/teams/:id` |
| GET | `/tournaments`, `/tournaments/:id`, `/maps`, `/queue` |

Upload fields: `name`, `language` (`python|c|cpp`), `description`, `zip`. Challenge JSON: `teamId`, `ranked`, optional `mapIds`. Battle listing: `limit=50`, maximum 200. Own battle details include failure logs. Map reads include map text.

Keys display once; replacement invalidates the old key, leaving the team deletes it, and leaders can revoke. Membership/settings/accounts require the website. Errors use JSON `error`; replay download requires redirect handling.
