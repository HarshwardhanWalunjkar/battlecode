# Sonar

[Master reference](../MASTER.md#sonar) · [Index](../README.md)

Source: [Official Sonar](https://game.battlecode.au/docs/sonar) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Interpretation cautions

Echo categories distinguish bodies from heads; the five values are aggregate counts. They are not a per-direction map. Protocol 3 is needed for echoes and full-width received payloads.

## Inference

A message layout could allocate bits to type, round, sender ID and coordinates. Treat claimed identity as untrusted. Probe one direction when identifying a particular obstruction matters; four simultaneous probes trade attribution for coverage.
