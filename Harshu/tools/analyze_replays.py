"""Summarize actual online replay evidence, independent of bot heuristics."""
from collections import Counter
import json
from pathlib import Path

root = Path('evaluations/online')
games = []
totals = Counter()
for path in sorted(root.glob('*.events.json')):
    replay = json.loads(path.read_text())
    details = json.loads(path.with_name(path.name.replace('.events', '')).read_text())
    assert details['match']['teamAId'] == 422, 'Review team orientation'
    assert replay['result']['winner'] == details['match']['winner'].upper()
    teams = {i: 'AB'[int(line.split()[1])] for i, line in enumerate(
        line for line in replay['map'].splitlines() if line.startswith('DRAGON '))}
    stats = {team: Counter() for team in 'AB'}
    round_number, active, head_deaths = -1, None, []
    collisions = Counter()

    def end_turn():
        if head_deaths and any(teams[i] == 'A' for i in head_deaths):
            pair = ''.join(sorted(teams[i] for i in head_deaths))
            collisions[f'{teams[active]} initiated {pair}'] += 1

    for event in replay['events']:
        kind = event['type']
        if kind == 'roundStart':
            round_number = event['round']
        elif kind == 'turnStart':
            end_turn()
            active, head_deaths = event['id'], []
        elif kind == 'dragonSplit':
            teams[event['childId']] = event['team']
            stats[event['team']]['splits'] += 1
        elif kind == 'dragonAction':
            row = stats[teams[event['id']]]
            row['turns'] += 1
            row['timeouts'] += bool(event.get('tle'))
            row['budget_exceeded'] += event.get('instructions', {}).get('exceeded', False)
            row['max_points'] = max(row['max_points'], event.get('instructions', {}).get('count', 0))
        elif kind == 'dragonDeath':
            stats[teams[event['id']]][event['reason']] += 1
            if event['reason'] == 'H':
                head_deaths.append(event['id'])
    end_turn()
    for k, v in stats['A'].items():
        if k != 'max_points':
            totals[k] += v
    totals['max_points'] = max(totals['max_points'], stats['A']['max_points'])
    totals.update(collisions)
    totals[replay['result']['endReason']] += 1
    games.append({'id': details['match']['id'], 'map': details['mapName'],
                  'opponent': details['teamBName'], 'rounds_played': round_number + 1,
                  'result': replay['result'], 'stats': stats, 'collisions': collisions,
                  'peak_count': {team: max(r[team]['count'] for r in replay['rounds']) for team in 'AB'}})
output = {'our_team': 'A', 'games': games, 'totals': totals}
(root / 'summary.json').write_text(json.dumps(output, indent=2) + '\n')
print(json.dumps(totals, indent=2))
