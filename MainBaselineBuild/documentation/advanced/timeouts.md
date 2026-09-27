# Timeouts

[Master reference](../MASTER.md#compute-and-runtime) · [Index](../README.md)

Source: [Official Timeouts](https://game.battlecode.au/docs/timeouts) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Additional limits and costs

Judge backstop: 1 CPU-second/10 wall-seconds. Team replay output: 500,000 log/indicator/drawing lines or 16 MB; indicators truncate at 512 characters.

Meter examples: ordinary arithmetic/comparisons 1; loads/stores/SIMD 2; division/remainder 3; direct calls 4; indirect calls 6; memory growth 50. See source for the exhaustive opcode schedule. Reads cost 6/byte; randomness 40,000+3/byte; file/path access 40,000.

C/C++ helpers configure full buffering. Avoid `unitbuf`, `sync_with_stdio(false)`, and unflushed `cin.tie(nullptr)`; prefer `clog` over `cerr`. Python output is already buffered. Fixed-seed C generators can synchronize dragons unintentionally.
