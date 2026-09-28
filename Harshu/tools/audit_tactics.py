"""Reconstruct portal collisions and long-dragon splits from archived events.

Uses the official viewer decoder's events; it does not play new games. Portal
destinations are independently checked against every successful movement event.
"""
import argparse
from collections import Counter
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
DIRECTIONS = 'NESW'


def analyze(path, *, detail=None, own=None):
    data = json.loads(path.read_text())
    if detail is None:
        detail = json.loads(path.with_name(path.name.replace('.events', '')).read_text())
        match = detail['match']
        own = 'A' if match['teamAId'] == 422 else 'B' if match['teamBId'] == 422 else None
    match = detail['match']
    lines = data['map'].splitlines()
    w, h = map(int, lines[0].split()[1:])
    edges, pairs, bodies, teams = {}, {}, {}, {}

    def pos(p):
        return p['y'] * w + p['x']

    def neighbor(p, d):
        x, y = p % w, p // w
        return ((y + (d == 2) - (d == 0)) % h) * w + (x + (d == 1) - (d == 3)) % w

    # Map-file edge indices include duplicated boundary rows/columns, unlike
    # the bot's canonical toroidal edge keys.
    for line in lines:
        t = line.split()
        if t[0] == 'EDGE':
            index, kind, portal = map(int, t[1:])
            row, x = divmod(index, w + 1)
            p, d = (row // 2 % h) * w + x % w, 3 if row % 2 else 0
            k = 2 * p + (d == 3)
            edges[k] = (kind, portal)
            if kind == 2:
                pairs.setdefault(portal, set()).add(k)
        elif t[0] == 'DRAGON':
            v = list(map(int, t[1:]))
            i = len(bodies)
            bodies[i] = [y*w+x for x, y in zip(v[2::2], v[3::2])]
            teams[i] = 'AB'[v[0]]

    def crossing(p, d):
        k = 2 * (neighbor(p, d) if d in (1, 2) else p) + (d in (1, 3))
        kind, portal = edges.get(k, (0, -1))
        dest = neighbor(p, d)
        if kind == 1:
            return None, -1
        if kind == 2:
            assert len(pairs[portal]) == 2
            far = next(x for x in pairs[portal] if x != k)
            dest = far // 2
            if d in (0, 3):
                dest = neighbor(dest, d)
        return dest, portal if kind == 2 else -1

    round_number, active, start, step = -1, None, None, 0
    action = {}
    collisions, splits, recent = [], [], {}
    deaths, drops, collections = [], {}, []
    validated = 0
    for e in data['events']:
        kind = e['type']
        if kind == 'roundStart':
            round_number = e['round']
        if round_number < 0:
            continue
        if kind == 'turnStart':
            active, step = e['id'], 0
            start = bodies[active][0]
        elif kind == 'dragonAction':
            action = e['action'] or {}
        elif kind == 'dragonUpdate':
            b = bodies[e['id']]
            d = DIRECTIONS.index(action['steps'][step])
            dest, portal = crossing(b[0], d)
            assert dest == pos(e['head']), (path.name, round_number, e, dest)
            validated += 1
            if portal >= 0:
                recent[teams[active], portal] = (round_number, active, start, dest)
            step += 1
            b.insert(0, dest)
            while len(b) > 1 and b[-1] != pos(e['tail']):
                b.pop()
        elif kind == 'dragonSplit':
            i, child = e['parentId'], e['childId']
            if len(bodies[i]) >= 6:
                occupied = {p for b in bodies.values() for p in b}
                destinations = [crossing(bodies[i][0], d)[0] for d in range(4)]
                splits.append({'round': round_number, 'id': i, 'child': child,
                               'own': teams[i] == own, 'team': teams[i], 'before': len(bodies[i]),
                               'empty_first_steps': sum(p is not None and p not in occupied for p in destinations),
                               'parent': len(e['parentBody']), 'rear': len(e['childBody'])})
            bodies[i] = [pos(p) for p in e['parentBody']]
            bodies[child] = [pos(p) for p in e['childBody']]
            teams[child] = e['team']
        elif kind == 'tileChange' and not e['hasPearl'] and active in bodies:
            p = pos(e['tile'])
            if p in drops:
                donor = drops.pop(p)
                collections.append({'round': round_number, 'recipient': active,
                    'recipient_team': teams[active], 'recipient_length': len(bodies[active]),
                    **donor})
        elif kind == 'dragonDeath':
            i = e['id']
            occupied = {p for b in bodies.values() for p in b}
            empty_steps = []
            for direction in range(4):
                dest, portal = crossing(bodies[i][0], direction)
                if dest is not None and dest not in occupied:
                    dx, dy = abs(dest % w - bodies[i][0] % w), abs(dest // w - bodies[i][0] // w)
                    empty_steps.append({'direction': DIRECTIONS[direction], 'portal': portal,
                                        'visible': min(dx, w-dx) <= 3 and min(dy, h-dy) <= 3})
            deaths.append({'round': round_number, 'id': i, 'team': teams[i],
                           'length': len(bodies[i]), 'reason': e['reason'],
                           'active': active, 'empty_first_steps': len(empty_steps),
                           'empty_steps': empty_steps,
                           'action': action if i == active else None})
            for p in bodies[i][::2]:
                drops[p] = {'donor': i, 'donor_team': teams[i], 'donor_length': len(bodies[i]),
                            'donor_round': round_number, 'donor_reason': e['reason']}
            # Target head dies first. Capture the collision while both bodies
            # are still present, and avoid counting its initiator twice.
            if action.get('kind') == 'move' and active in bodies and step < len(action['steps']):
                dest, portal = crossing(bodies[active][0], DIRECTIONS.index(action['steps'][step]))
                victim = next((j for j, b in bodies.items() if dest in b), None)
                if portal >= 0 and (i != active or e['reason'] != 'H'):
                    dx = abs(dest % w - start % w)
                    dy = abs(dest // w - start // w)
                    last = recent.get((teams[active], portal))
                    collisions.append({'round': round_number, 'id': active,
                        'own': teams[active] == own, 'team': teams[active],
                        'length': len(bodies[active]),
                        'portal': portal, 'reason': e['reason'], 'victim': victim,
                        'ally': victim is not None and victim != active and teams[victim] == teams[active],
                        'visible_exit': min(dx, w-dx) <= 3 and min(dy, h-dy) <= 3,
                        'recent_ally_crossing': last if last and round_number-last[0] <= 2 else None})
            del bodies[i]
    return {'id': match['id'], 'own': own, 'version': match.get('submission'+own+'Id') if own else None,
            'map': detail['mapName'], 'teams': {s:detail['team'+s+'Name'] for s in 'AB'},
            'validated_moves': validated, 'portal_collisions': collisions, 'long_splits': splits,
            'deaths': deaths, 'death_pearl_collections': collections}


def main():
    p = argparse.ArgumentParser()
    p.add_argument('replays', nargs='+', type=Path)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--local-results', type=Path,
                   help='evaluate.py results for these decoded local replay files')
    args = p.parse_args()
    games = []
    local = json.loads(args.local_results.read_text()) if args.local_results else None
    for path in args.replays:
        if local is None:
            games.append(analyze(path))
            continue
        index = int(path.name.split('-', 1)[0])
        row = local['records'][index]
        expected = args.local_results.parent / (args.local_results.stem + '-replays') / f'{index:04d}-{row["map"]}.events.json'
        if path.resolve() != expected.resolve():
            raise ValueError(f'Replay does not match the local result record: {path}')
        detail = {'match': {'id': f'{args.local_results.stem}/{index:04d}'},
                  'mapName': row['map'],
                  **{'team'+side+'Name': row['projects'][side] for side in 'AB'}}
        game = analyze(path, detail=detail, own=row['bot_side'])
        game['source_fingerprints'] = {side: local['fingerprints'][row['projects'][side]] for side in 'AB'}
        game['sandbox'] = local['sandbox']
        games.append(game)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps({'games': games}, indent=2)+'\n')
    for version in sorted({g['version'] for g in games}, key=str):
        group = [g for g in games if g['version'] == version]
        hits = [e for g in group for e in g['portal_collisions'] if e['own']]
        print(version, len(group), 'games; portal collisions', len(hits),
              'allied', sum(e['ally'] for e in hits),
              'unseen exits', sum(not e['visible_exit'] for e in hits),
              'reasons', dict(Counter(e['reason'] for e in hits)))


if __name__ == '__main__':
    main()
