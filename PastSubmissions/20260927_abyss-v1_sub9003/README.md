# abyss-v1 — Past Submission (Replaced by v2)

| Field | Value |
|---|---|
| Name | abyss-v1 |
| Submission ID | 9003 |
| Team ID | 422 |
| Uploaded | 2026-09-27 17:07:26 UTC (22:37 IST) |
| Fingerprint | 396bb8d5e5b95ee2c95f15fa3eafa139e24dfa4952d4ccc47354e3ee8ebc44e6 |
| Language | C++20 |
| Toolkit | 1.2.2 |
| Status | Replaced by abyss-v2 |

## Performance

- **Local**: 28/38 wins vs greedy+hunter baselines (sandbox)
- **Online**: 0/20 vs blauerdrache (1522) and bongcloud (1586)

## Why replaced

Lost every online game. Root causes: reckless head trades (99/114 deaths self-initiated), max 4 dragons vs opponents with 21-64, portal avoidance, early kelp deaths from long bodies. See `evaluations/online/REPORT.md`.
