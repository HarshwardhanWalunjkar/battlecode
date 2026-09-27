# Pearls

[Master reference](../MASTER.md#pearls) · [Index](../README.md)

Source: [Official Pearls](https://game.battlecode.au/docs/pearls) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Timing pitfall

A countdown is time until an **attempt**, not a guaranteed new resource. Occupied tiles still reset their timer. A visible pearl does not imply another can accumulate there.

## Inference

Record `(position, observation_round, countdown, pearl_present)` separately. A spawn forecast becomes conditional when occupancy is unknown. Compare two routes by expected collected pearls, safety, and arrival timing—not merely nearest current food.

See [death](death.md) for corpse-generated pearls, which do not use normal spawn attempts.
