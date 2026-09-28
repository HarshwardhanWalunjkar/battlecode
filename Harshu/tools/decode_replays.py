"""Decode archived replays with the parser shipped in unswbc's official viewer.

Requires Node.js. No network access or third-party parser downloads.
The viewer adapter deliberately fails if its entry-point names change.
"""
import argparse
from pathlib import Path
import subprocess
import tempfile
import zipfile
import unswbc

ADAPTER = r'''
const fs = require('fs'), zlib = require('zlib');
for (const filename of process.argv.slice(2)) {
  const loaded = replayDecode(zlib.gunzipSync(fs.readFileSync(filename)));
  const {match, events, result, map} = loaded;
  const rounds = [];
  for (let round = 0; round <= match.maxRound; round++) {
    const state = match.roundAt(round);
    const teams = {A: {count: 0, longest: 0, total: 0}, B: {count: 0, longest: 0, total: 0}};
    for (const dragon of state.bodies.dragons.values()) {
      const team = teams[dragon.team];
      team.count++;
      team.longest = Math.max(team.longest, dragon.body.length);
      team.total += dragon.body.length;
    }
    const dragons = [...state.bodies.dragons.values()].map(d => ({id:d.id, team:d.team,
      length:d.body.length, x:d.body[0].x, y:d.body[0].y}));
    rounds.push({round, ...teams, ...(globalThis.includeDragonStates ? {dragons} : {})});
  }
  fs.writeFileSync(filename.replace(/\.replay$/, '.events.json'),
    JSON.stringify({result, map, rounds, events}, (_, v) => typeof v === 'bigint' ? String(v) : v));
  console.log(filename + ': decoded ' + events.length + ' events');
}
'''


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--states', action='store_true', help='Include per-round dragon heads and lengths')
    parser.add_argument('replays', nargs='+')
    args = parser.parse_args()
    viewer = Path(unswbc.__file__).with_name('replay-viewer.vsix')
    with zipfile.ZipFile(viewer) as archive:
        source = archive.read('extension/dist/webview/webview.js').decode()
    marker = 'var t_=document.getElementById'
    if source.count(marker) != 1 or 'function Hf(e)' not in source:
        raise RuntimeError('Viewer changed; review the decoder adapter')
    source = ('globalThis.includeDragonStates=' + str(args.states).lower() + ';\n'
              + 'globalThis.document={contentType:"text/html"};\n' + source.split(marker)[0]
              + 'globalThis.replayDecode=Hf;})();\n' + ADAPTER)
    with tempfile.TemporaryDirectory(prefix='battlecode-decoder-') as folder:
        script = Path(folder) / 'decode.cjs'
        script.write_text(source)
        result = subprocess.run(['node', str(script), *args.replays], capture_output=True, text=True)
        print(result.stdout, end='')
        if result.returncode:
            # Minified source in Node's traceback can be hundreds of kilobytes.
            print('\n'.join(line[:300] for line in result.stderr.splitlines()[-15:]))
            raise SystemExit(result.returncode)


if __name__ == '__main__':
    main()
