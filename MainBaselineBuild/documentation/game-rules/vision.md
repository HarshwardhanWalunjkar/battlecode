# Vision

[Master reference](../MASTER.md#board-and-information) · [Index](../README.md)

Source: [Official Vision](https://game.battlecode.au/docs/vision) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## What a snapshot means

The visible region is local, wrapped, and refreshed for your turn. It does not expose every dragon's complete body or an enemy's total length. Infer only what visible parts support.

## Inference

Keep terrain memory separate from occupancy memory. An old empty observation is not proof of a safe sprint. Inspect peripheral edges as well as tile contents; the body segment direction can help reconstruct a visible chain but does not reveal missing segments.
