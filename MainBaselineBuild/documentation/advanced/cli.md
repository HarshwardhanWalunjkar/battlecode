# CLI

[Master reference](../MASTER.md#build-and-submit) · [Index](../README.md)

Source: [Official CLI](https://game.battlecode.au/docs/cli) · Reviewed 2026-09-27.

The linked master section contains the core rules; these are focused supplementary notes.

## Command lookup

| Command after `unswbc` | Purpose |
| --- | --- |
| `init LANGUAGE DIR` | Scaffold (`python`, `c`, `cpp`) |
| `update DIR` | Refresh helpers/maps |
| `maps [DIR]` | Add missing maps |
| `run MAP BOT_A BOT_B` | Build/play/replay |
| `vscode` | Replay extension |
| `auth set KEY` | Store team-member key |
| `auth status` / `auth clear` | Inspect/remove authentication |
| `submit DIR -n NAME -d DESCRIPTION` | Upload version |
| `log` | Diagnostics with masked keys |
| `help COMMAND` | Options |

Run supports `--sandbox`, `--seed`, and `-v`. Sandbox also accepts `main.wasm` directly or its directory. `UNSWBC_NO_UPDATE=1` disables update prompts. Pin toolkit versions for recorded experiments; update deliberately for competition compatibility.
