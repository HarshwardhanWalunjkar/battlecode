# IO Protocol

[Master reference](../MASTER.md#helper-and-protocol) · [Index](../README.md)

Source: [Official IO Protocol](https://game.battlecode.au/docs/protocol) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Ordered input schema

```text
INIT: ID, TEAM, MAP(width,height), UNIT_LIMIT
TURN: ROUND, DIR, LENGTH, UNIT_COUNT, NUM_MSGS
      NUM_MSGS unsigned values
      ECHOES(kelp,ally,ally_head,enemy,enemy_head) if negotiated
      49 × (x,y,hasPearl,pearlIn)
      DRAGON_BODIES count
      count × (team,id,x,y,facing,isHead)
      horizontal edges: 8 rows × 7 tokens
      vertical edges:   7 rows × 8 tokens
```

Edge tokens: `.` open, `w` kelp, integer portal ID. Split whitespace, not characters. Tile origin is head−(3,3), wrapped. Horizontal rows are north edges plus final south; vertical rows west edges plus final east.

Reply commands: `MOVE directions`, `SPLIT size`, `SONAR direction value`, `PROTOCOL 3`, `LOG text`, `INDICATOR text`, `DOT x y r g b`, `LINE x1 y1 x2 y2 r g b`, `ENDTURN`.
