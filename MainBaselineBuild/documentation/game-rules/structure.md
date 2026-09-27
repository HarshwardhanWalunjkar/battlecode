# Structure

[Master reference](../MASTER.md#objective-and-architecture) · [Index](../README.md)

Source: [Official Structure](https://game.battlecode.au/docs/structure) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Decision hierarchy

Survive elimination; at the horizon maximize the largest living length, then total living length. A round contains sequential turns, ordered by dragon ID.

## Worked inference

At the length tiebreak, lengths `[12,2]` beat `[11,11]`. Lengths `[12,3]` beat `[12,2]` only on the secondary comparison. A policy that optimizes total biomass alone can choose the wrong winner.

See the master's termination conflict before implementing a terminal-state evaluator.
