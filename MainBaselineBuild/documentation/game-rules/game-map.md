# Game Map

[Master reference](../MASTER.md#board-and-information) · [Index](../README.md)

Source: [Official Game Map](https://game.battlecode.au/docs/map-info) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Coordinate formulas

Derived from toroidal geometry:

```text
step: ((x + dx) % width, (y + dy) % height)
dx_wrap = min(abs(x1-x2), width-abs(x1-x2))
dy_wrap = min(abs(y1-y2), height-abs(y1-y2))
```

`dx_wrap + dy_wrap` is empty-board distance without portals. Kelp and portals require graph search; this expression is not a guaranteed admissible heuristic with shortcuts.

## Inference

Maintain candidate mirror/rotation models and eliminate inconsistent ones. Unseen occupancy remains uncertain even when the static map has been reconstructed.
