# Submitting via Website

[Master reference](../MASTER.md#build-and-submit) · [Index](../README.md)

Source: [Official Submitting via Website](https://game.battlecode.au/docs/submitting) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Manifest

```toml
[project]
language = "py"
include = ["*.py"]
```

Manifest language values are `py`, `c`, `c++`. Include every required file, including helpers. The website language must agree with the manifest.

## Release check

Unzip into a clean directory and run there. A successful upload/build becomes active automatically; retain a known-good Ready build for rollback. If a build fails, inspect layout and included files before escalating with its version number.

See [CLI](../advanced/cli.md) for upload commands and [API](../advanced/api.md) for multipart automation.
