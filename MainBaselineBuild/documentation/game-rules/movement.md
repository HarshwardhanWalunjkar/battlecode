# Movement

[Master reference](../MASTER.md#movement-and-sprinting) · [Index](../README.md)

Source: [Official Movement](https://game.battlecode.au/docs/movement) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Planner contract

Evaluate a path step by step. Do not reserve only its destination or remove the tail before checking collision.

## Inference example

With length 3, a two-step route ending on a pearl can survive and retain length 3. Starting at length 2, the same route without a first-step pearl fails its second-step affordability check, even if that destination contains food.

Use [execution order](../advanced/execution-order.md) to resolve intermediate body changes.
