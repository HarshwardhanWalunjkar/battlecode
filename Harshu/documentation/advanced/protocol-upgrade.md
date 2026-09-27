# Protocol Upgrade

[Master reference](../MASTER.md#helper-and-protocol) · [Index](../README.md)

Source: [Official Protocol Upgrade](https://game.battlecode.au/docs/protocol-upgrade) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Upgrade checklist

1. Upgrade the toolkit, then update the project's helper.
2. Review custom helper changes against the `.bak`.
3. Change explicitly typed 32-bit sonar containers to unsigned 64-bit.
4. Test first input, next-turn negotiation, and inherited child negotiation.

Legacy `send_sonar(message)` sends a 32-bit value along facing. Directed Python/C++ uses `send_sonar(direction,message)`; C uses `unswbc_send_sonar_to`. C echo access is `unswbc_sonar_echoes`.

The input switch is prospective; it does not recover previously discarded large messages. Maximum full-width payload: `18446744073709551615`.
