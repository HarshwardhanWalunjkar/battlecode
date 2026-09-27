# Toolkit 1.2.2 compatibility check

The project's toolkit was upgraded from 1.1.0 to 1.2.2 using `uv pip install --python .venv/bin/python 'unswbc==1.2.2'`. The automatic updater had tried to use `pip`, which is not installed inside this uv-managed environment. Installing pip was unnecessary.

The C++ helper, submission/API interfaces, and runner interfaces compared unchanged. The engine WASM file changed. Two bundled maps were added: `portals.map` and `slithery_fight.map`; both are now in `maps/`.

The bot source and strategy are unchanged. Its source fingerprint is still `396bb8d5e5b95ee2c95f15fa3eafa139e24dfa4952d4ccc47354e3ee8ebc44e6`. The ZIP was regenerated with the same contents; its manifest records the current packaging toolkit. Historical 1.1.0 evaluation results remain labeled with that version.

## Checks

- Dependency compatibility check passed.
- All six existing official-engine contract scenarios passed on 1.2.2.
- All eight submission timing checks passed.
- Six sandbox games: both sides on `arena`, `portals`, and `slithery_fight`, seed 211, against the head-hunter.
- Three wins, three losses; zero runtime errors and zero bot no-valid-action deaths.
- Highest bot turn cost: 21,920,118 of 100,000,000 points.
- Highest reported bot memory: 655,360 bytes of the 48 MB allowance.

This is a compatibility check, not a replacement for the original 38-game strategy evaluation or a full review of the online documentation. Full records: [JSON](toolkit-1.2.2.json), [log](toolkit-1.2.2.log).

No bot was submitted and no ranked timing was started by this maintenance work. No authentication key was read, changed, or copied into project files.
