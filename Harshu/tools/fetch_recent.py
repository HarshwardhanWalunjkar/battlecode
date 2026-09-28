"""Read and archive all completed games in the latest account battle snapshot."""
import json
from pathlib import Path
import time
from unswbc import api, auth
from fetch_battles import download

root = Path(__file__).resolve().parents[1]
out = root / 'evaluations/recent-20260928'
out.mkdir(parents=True, exist_ok=True)
snapshot = json.loads((root / 'submissions/server-snapshot.json').read_text())
(out / 'snapshot.json').write_text(json.dumps(snapshot, indent=2) + '\n')
key = auth.key()
last_request = 0


def read(operation):
    global last_request
    time.sleep(max(0, .65 - (time.monotonic() - last_request)))
    last_request = time.monotonic()
    return operation()


def details(game_id, refresh=False):
    path = out / f'{game_id}.json'
    if path.exists() and not refresh:
        cached = json.loads(path.read_text())
        if cached.get('match', {}).get('status') == 'completed':
            return cached
    data = read(lambda: api.request(f'battles/{game_id}', key=key))
    path.write_text(json.dumps(data, indent=2) + '\n')
    return data


inventory = []
for battle in snapshot['battles']:
    if battle['id'] in (409862, 409982):
        continue  # The previous audit already archived all of these games.
    parent = details(battle['id'], refresh=True)
    for index, game in enumerate(parent.get('games', []), 1):
        inventory.append({'series': battle['id'], 'game_number': index,
                          'opponent': battle['opponent'], 'ranked': battle['ranked'], **game})
    print(f"Series {battle['id']} {battle['opponent']}: {len(parent.get('games', []))} games", flush=True)
(out / 'inventory.json').write_text(json.dumps(inventory, indent=2) + '\n')
# Inspect the user's named example first while the rest download.
inventory.sort(key=lambda g: (not(g['opponent'] == 'tridev6509' and g['mapName'] == 'Queen Of Spades'), g['id']))
for game in inventory:
    if game['status'] != 'completed' or not game.get('hasReplay'):
        continue
    game_id = game['id']
    data = details(game_id)
    path = out / f'{game_id}.replay'
    if not path.exists():
        path.write_bytes(read(lambda: download(f'battles/{game_id}/replay', key)))
    print(f"Game {game_id}: {game['mapName']} / {game['opponent']} downloaded", flush=True)
