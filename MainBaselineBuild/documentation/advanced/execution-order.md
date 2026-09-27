# Execution Order

[Master reference](../MASTER.md#execution-sequence) · [Index](../README.md)

Source: [Official Execution Order](https://game.battlecode.au/docs/execution-order) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Implementation caution

The master contains the transition sequence. End-turn delivery can occur through `ENDTURN` or blocking for the next input. Exit or budget exhaustion delivers nothing; do not confuse process termination with finishing a turn.

## Test design

Create minimal scenarios with one changed variable. Compare the replay after each sprint substep and after each dragon ID, rather than only round-end positions. Include a head collision and a subsequent dragon collecting the resulting food.

## Verified engine note

Toolkit 1.1.0 starts at `ROUND 0`, so the final round is 499. The targeted contract tests also confirm that an unparseable line after a valid move preserves that move.
