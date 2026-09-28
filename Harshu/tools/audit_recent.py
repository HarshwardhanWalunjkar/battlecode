"""Side-aware version-aware event audit with body checks against the official viewer."""
from collections import Counter, defaultdict
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1] / 'evaluations/recent-20260928'


def point(p):
    return (p['x'], p['y'])


def analyze(path):
    replay = json.loads(path.read_text())
    detail = json.loads(path.with_name(path.name.replace('.events', '')).read_text())
    m = detail['match']
    own = 'A' if m['teamAId'] == 422 else 'B'
    enemy = 'B' if own == 'A' else 'A'
    stats = {t: Counter() for t in 'AB'}
    teams, bodies, born = {}, {}, {}
    for i, line in enumerate(l for l in replay['map'].splitlines() if l.startswith('DRAGON ')):
        v = list(map(int, line.split()[1:]))
        teams[i] = 'AB'[v[0]]
        bodies[i] = list(zip(v[2::2], v[3::2]))
        born[i] = 0
    w, h = map(int, replay['map'].splitlines()[0].split()[1:])
    round_num, active = -1, None
    splits, deaths, trades, head_deaths = [], [], [], []
    validation = 0

    def finish_turn():
        if head_deaths and any(teams[i] == own for i in head_deaths):
            trades.append({'round': round_num, 'initiator': active,
                           'own_initiated': teams[active] == own,
                           'teams': ''.join(sorted(teams[i] for i in head_deaths)),
                           'ids': head_deaths[:]})

    def verify(round_number):
        nonlocal validation
        expected = {d['id']: d for d in replay['rounds'][round_number]['dragons']}
        assert set(bodies) == set(expected), (path.name, round_number, 'living IDs differ')
        for i, b in bodies.items():
            d = expected[i]
            assert (len(b), b[0]) == (d['length'], (d['x'], d['y'])), (path.name, round_number, i)
        validation += 1

    for e in replay['events']:
        kind = e['type']
        if kind == 'roundStart':
            finish_turn(); head_deaths = []
            round_num, active = e['round'], None
            verify(round_num)
        if round_num < 0:
            continue  # Initial updates describe initial state, already read from map.
        if kind == 'turnStart':
            finish_turn(); head_deaths = []; active = e['id']
        elif kind == 'dragonAction':
            s = stats[teams[e['id']]]; s['turns'] += 1
            s['timeouts'] += bool(e.get('tle')); s['exceeded'] += e.get('instructions', {}).get('exceeded', False)
            s['max_points'] = max(s['max_points'], e.get('instructions', {}).get('count', 0))
            a = e['action'] or {}
            if a.get('kind') == 'move':
                s['sprint_steps_requested'] += max(0, len(a['steps']) - 1)
        elif kind == 'tileChange' and not e['hasPearl'] and active is not None:
            stats[teams[active]]['pearls_eaten'] += 1
        elif kind == 'dragonUpdate':
            b = bodies[e['id']]; new = point(e['head']); old = b[0]
            dx, dy = abs(new[0] - old[0]), abs(new[1] - old[1])
            if min(dx, w-dx) + min(dy, h-dy) > 1:
                stats[teams[e['id']]]['distant_portal_steps'] += 1
            b.insert(0, new)
            while len(b) > 1 and b[-1] != point(e['tail']):
                b.pop()
        elif kind == 'dragonSplit':
            team = e['team']; parent = e['parentId']; child = e['childId']
            old_length = len(bodies[parent]); largest = max(len(b) for i, b in bodies.items() if teams[i] == team)
            parent_length, child_length = len(e['parentBody']), len(e['childBody'])
            splits.append({'round': round_num, 'id': parent, 'child': child, 'own': team == own,
                           'before': old_length, 'parent_after': parent_length, 'child_length': child_length,
                           'was_largest': old_length == largest, 'team_count': sum(t == team for i,t in teams.items() if i in bodies)})
            teams[child] = team; born[child] = round_num
            bodies[parent] = list(map(point, e['parentBody'])); bodies[child] = list(map(point, e['childBody']))
            stats[team]['splits'] += 1
        elif kind == 'dragonDeath':
            i = e['id']; s = stats[teams[i]]; s['death_' + e['reason']] += 1
            deaths.append({'round': round_num, 'id': i, 'own': teams[i] == own,
                           'reason': e['reason'], 'length': len(bodies[i]), 'age': round_num - born[i],
                           'head': bodies[i][0], 'active': active})
            if e['reason'] == 'H':
                head_deaths.append(i)
            del bodies[i]
    finish_turn()
    verify(replay['rounds'][-1]['round'])
    winner = replay['result']['winner']
    assert (winner.lower() if winner else None) == (m['winner'] if m['winner'] in ('a','b') else None)
    histories = defaultdict(list)
    for r in replay['rounds']:
        for d in r['dragons']:
            if d['team'] == own:
                histories[d['id']].append((r['round'], d['x'], d['y'], d['length']))
    loops = []
    for i, history in histories.items():
        episodes = []
        for offset in range(0, len(history)-79, 10):
            window = history[offset:offset+80]
            if window[-1][0]-window[0][0] != 79:
                continue
            cells = len({(x,y) for _,x,y,l in window})
            lengths = {l for r,x,y,l in window}
            if cells <= 12 and len(lengths) == 1 and max(lengths) <= 4:
                start,end = window[0][0],window[-1][0]
                if episodes and start <= episodes[-1]['end']:
                    episodes[-1]['end'] = end
                else:
                    episodes.append({'id': i, 'start': start, 'end': end, 'length': window[0][3], 'window_cells_max': 12})
        loops.extend(episodes)
    result = replay['result']
    us, them = result['team' + own], result['team' + enemy]
    return {'id':m['id'], 'series':m['seriesId'], 'map':detail['mapName'], 'own_side':own,
            'version_id':m.get('submission' + own + 'Id'), 'opponent':detail['team'+enemy+'Name'],
            'ranked':m['ranked'], 'win':result['winner']==own, 'draw':result['winner'] is None, 'reason':result['endReason'],
            'rounds':round_num+1, 'final_own':us, 'final_enemy':them,
            'stats_own':stats[own], 'stats_enemy':stats[enemy],
            'splits':splits, 'deaths':deaths, 'trades':trades, 'small_loops':loops,
            'peak_population':max(r[own]['count'] for r in replay['rounds']),
            'peak_length':max(r[own]['longest'] for r in replay['rounds']),
            'checkpoints':[{k:r[k] for k in ('round',own,enemy)} for r in replay['rounds'] if r['round'] in (0,100,200,300,400,450,500)],
            'validated_snapshots':validation}


if __name__ == '__main__':
    games = [analyze(p) for p in sorted(ROOT.glob('*.events.json'))]
    (ROOT / 'analysis.json').write_text(json.dumps({'games':games}, indent=2)+'\n')
    for version in sorted({g['version_id'] for g in games}, key=str):
        group = [g for g in games if g['version_id']==version]
        print(version, len(group), 'games', sum(g['win'] for g in group), 'wins',
              Counter(g['reason'] for g in group if not g['win']),
              'games with small flat-length loops:', sum(bool(g['small_loops']) for g in group))
