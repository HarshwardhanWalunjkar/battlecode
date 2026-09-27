# Map Files

[Master reference](../MASTER.md#build-and-submit) · [Index](../README.md)

Source: [Official Map Files](https://game.battlecode.au/docs/map-files) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Directive lookup

| Directive | Meaning |
| --- | --- |
| `MAP width height` | Required first |
| `MAP_NAME name` | Optional display label |
| `SYMMETRY VALUE` | Optional paired spawn timing; `x`, `y`, or `xy` |
| `TILE_COUNT n` | Number of tile records |
| `TILE x y minGap maxGap` | Inclusive spawn interval; max=0 disables, otherwise min≥1 |
| `EDGE_COUNT n` | Number of edge records |
| `EDGE index kind portalId` | kind 0=open, 1=kelp, 2=portal; kelp ID −1 |
| `DRAGON_COUNT n` | Number of dragon records |
| `DRAGON team length x y ...` | Team 0/1, head-first coordinates |

Bodies must be adjacent, in bounds, nonoverlapping, length≥2. Edge storage alternates horizontal/vertical rows of width+1; documented allocation is `(2*height+1)*(width+1)`. The wording does not establish the last valid index clearly: prefer editor exports. Portal pairs must share orientation.
