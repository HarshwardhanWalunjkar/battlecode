# Overview

[Master reference](../MASTER.md#objective-and-architecture) · [Index](../README.md)

Source: [Official Overview](https://game.battlecode.au/docs/overview) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## At a glance

Independent dragons execute the same bot; normal team capacity is 64. Each process must make decisions using its own observations and communicated information.

## Design note

Keep local memory useful but reconstructible. Treat the master as the starting design contract; distinguish observations from hypotheses about unseen terrain and enemies. See [scoring](../game-rules/structure.md) before choosing an expansion policy.
