# Quickstart

[Master reference](../MASTER.md#build-and-submit) · [Index](../README.md)

Source: [Official Quickstart](https://game.battlecode.au/docs/quickstart) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Project essentials

Python projects use `main.py`, `helper.py`, and `bot.toml`; C uses `helper.h`/`helper.c`, C++ `helper.hpp`. `pip install unswbc` is an alternative to uv. Upgrade with `uv tool upgrade unswbc`.

The loop initializes once, updates each turn, selects an action, then ends/flushed output. The shipped starters try shuffled directions using repeatable seeds; they are baselines, not advanced planners.

## First experiment

Run self-play, inspect the replay, then compare against a frozen baseline across multiple maps. Use sandbox runs for performance conclusions. Keep bot source and generated replay artifacts separate.
