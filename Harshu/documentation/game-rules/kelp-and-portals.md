# Kelp and Portals

[Master reference](../MASTER.md#board-and-information) · [Index](../README.md)

Source: [Official Kelp and Portals](https://game.battlecode.au/docs/kelp-and-portals) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Representation

Store terrain on directed tile boundaries while keeping opposite views of the same physical edge consistent. A portal needs a paired edge and entry side; a neighboring-cell occupancy check alone is insufficient.

## Inference

Penalize unknown exits. Once a portal pair is mapped, evaluate destination occupancy using the most recent evidence. Test wrapping and portal traversal together, rather than assuming each separately correct mechanism composes automatically.
