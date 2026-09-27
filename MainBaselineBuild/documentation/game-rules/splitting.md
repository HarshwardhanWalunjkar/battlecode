# Splitting

[Master reference](../MASTER.md#splitting) · [Index](../README.md)

Source: [Official Splitting](https://game.battlecode.au/docs/splitting) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Boundary cases

Length 4 can split only a length-2 child. A length-6 parent can request 2, 3, or 4. Check the actual reported team limit/count.

## Inference

Evaluate two future heads, not merely two lengths. The parent's new tail and child's reversed body change escape space and sonar routing. A split can delay the parent in place, but cannot be repeated indefinitely without replenishment.
