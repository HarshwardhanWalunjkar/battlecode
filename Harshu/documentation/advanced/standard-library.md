# Standard Library

[Master reference](../MASTER.md#compute-and-runtime) · [Index](../README.md)

Source: [Official Standard Library](https://game.battlecode.au/docs/libraries) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Compatibility traps

Missing Python groups include network modules; `asyncio`, `multiprocessing`, `concurrent.futures`; `zlib`, `bz2`, `lzma`, `gzip`; `xml`, `sqlite3`, `dbm`, `plistlib`, `mimetypes`, `mailbox`, `importlib.metadata`; terminal/display and packaging tools. The source has the complete import inventory.

Importing is not capability: `threading` cannot start threads; `subprocess` cannot spawn; `shelve` lacks its backend; `pydoc` depends on missing networking. `zipfile`/`tarfile` support uncompressed content only. C++ headers exist except `<generator>`, but unsupported OS calls still fail.

Local tool overrides: `UNSWBC_PYTHON`, `CC`, `CXX`. Verify dependencies under the sandbox, not just the host interpreter.
