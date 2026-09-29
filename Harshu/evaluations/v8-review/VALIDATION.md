# Jormungandr-v1 Validation Results

## Source Fingerprints

| Bot | Fingerprint |
|-----|-------------|
| Jormungandr-v1 (V8) | `ae5c3b8485099480f652de402ab05b0328f85e311d369e776688352fc1843e72` |
| Frozen abyss-v7 | `0ce4a44b3c76102afe2813fde172cfe4dae61be1c2936931bf08cc181db35186` |

## Sandbox Comparison: V8 vs V7

Predeclared test: seed 911, both sides, three maps (Slithery Fight, Trophy, Trauma).

| Map | V8 Side | Result | V8 Dragons | V7 Dragons | Notes |
|-----|---------|--------|------------|------------|-------|
| Slithery Fight | A | **WIN** | 51 | 30 | Strong population advantage |
| Slithery Fight | B | **WIN** | 51 | 24 | Strong population advantage |
| Trophy | A | LOSS | 13 | 29 | Eliminated before late game |
| Trophy | B | **WIN** | 10 | 15 | Won despite fewer dragons |
| Trauma | A | LOSS | 3 | 7 | Low population finish |
| Trauma | B | **WIN** | 8 | 9 | Close game, won on longest |

**Summary: 4 wins, 2 losses (66.7% win rate)**

## Map-Specific Analysis

### Slithery Fight (2/2 wins)
The staged junction rescue and improved coil handling addressed the documented weakness. Both games finished with V8 having significant population advantage.

### Trophy (1/2)
- As A: Lost - worker attrition continued to be a problem
- As B: Won - different starting position may have better food access

The Trophy weakness from the REPORT.md (early cross-team collisions) was not fully resolved.

### Trauma (1/2)
- As A: Lost - general-only recheck showed 0/2, limitation documented
- As B: Won - map recognition may have helped navigation

The general-mode Trauma performance remains a documented limitation.

## Resource Limits

All games completed without:
- Bot errors
- Timeout violations
- Memory limit violations

CPU usage stayed below 33 million of the 100 million point budget.

## Correctness Fixes Applied

1. **Population cap rescue blocking**: Long Slithery scorers could not rescue at population 64. Fixed by leaving 2 slots free when limit >= 16.

2. **Food report freshness**: Delivered food reports included pearls the sender had just eaten. Fixed to only report unconsumed pearls.

3. **Donor credit calculation**: Counted entire hypothetical corpse instead of pearls on checked pickup route. Fixed to bounded credit.

4. **Collector area pull**: Required nearby offering worker to activate.

## Limitations

- Trophy as-A losses show the worker attrition problem is not fully solved
- Trauma general-mode remained 0/2 in focused recheck
- No parameter sweeps were performed
- Combined changes cannot establish isolated effect of any one feature

## Verdict

The validation results support releasing Jormungandr-v1 with the documented caveats:
- Clear improvement on Slithery Fight (the primary weakness target)
- Mixed results on Trophy (1/2)
- Mixed results on Trauma (1/2)
- Overall 66.7% sandbox win rate against frozen V7

The Trophy and Trauma limitations are documented, not hidden.
