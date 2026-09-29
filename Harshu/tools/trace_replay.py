"""Replay recorded actions locally while tracing v1 decisions (not a new match).

Recorded sonar is replayed too. Compare observed decisions/deaths to the archive;
do not treat this fixed action stream as an adaptive opponent for evaluation.
"""
import argparse
from collections import defaultdict
import json
from pathlib import Path
import shutil
import tempfile
from unswbc.engine import EngineModule
from unswbc.run import _resolve
from unswbc.bot import Pool, Bot

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('battle_id', type=int)
    parser.add_argument('--rounds', type=int, default=10)
    args = parser.parse_args()
    folder = ROOT / 'evaluations/online'
    replay = json.loads((folder / f'{args.battle_id}.events.json').read_text())
    details = json.loads((folder / f'{args.battle_id}.json').read_text())
    turns = defaultdict(list)
    expected_deaths = []
    round_num = -1
    for event in replay['events']:
        if event['type'] == 'roundStart':
            round_num = event['round']
        if event['type'] == 'dragonAction':
            action = event['action'] or {}
            command = ('MOVE ' + ''.join(action['steps']) if action.get('kind') == 'move'
                       else f"SPLIT {action['childSegmentCount']}" if action.get('kind') == 'split'
                       else '')
            turns[round_num, event['id']].append(command)
        if event['type'] == 'sonarPing':
            turns[round_num, event['senderId']].append(f"SONAR {event['direction']} {event['value']}")
        if event['type'] == 'dragonDeath' and round_num < args.rounds:
            expected_deaths.append([round_num, event['id'], event['reason']])
    trace, deaths, live, teams = [], [], {}, {}
    with tempfile.TemporaryDirectory(prefix='abyss-online-trace-') as tmp:
        temp = Path(tmp)
        from unswbc.project import Project
        project = Project.from_dir(ROOT / 'bot'); project.collect_sources()
        for name in sorted(set(['bot.toml', *project.sources])):
            (temp / name).parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(ROOT / 'bot' / name, temp / name)
        path = temp / 'main.cpp'
        path.write_text(path.read_text().replace('brain.commit(decision,ct);',
            'ct.output_log("TRACE",brain.complete,brain.length,brain.predicted.size(),'
            'decision.exact,decision.dying,brain.head); brain.commit(decision,ct);'))
        argv, cwd, _ = _resolve(str(temp))
        pool = Pool(argv, cwd=str(cwd), size=1)

        def spawn(i, init):
            teams[i] = next(line.split()[1] for line in init.decode().splitlines() if line.startswith('TEAM '))
            if teams[i] == 'A':
                live[i] = Bot(pool, init=init, name=str(i))

        class Done(Exception):
            pass

        def reply(i, data):
            r = int(data.decode().splitlines()[0].split()[1])
            if r >= args.rounds:
                raise Done()
            commands = turns[r, i]
            if i in live:
                output = live[i].ask(data).decode()
                chosen = next((line for line in output.splitlines() if line.startswith(('MOVE ', 'SPLIT '))), '')
                info = next((line for line in output.splitlines() if line.startswith('LOG TRACE')), '')
                trace.append({'round': r, 'id': i, 'expected': commands[0], 'chosen': chosen,
                              'same': chosen == commands[0], 'trace': info, 'error': str(live[i].error) if live[i].error else None})
            return ('PROTOCOL 3\n' + '\n'.join(commands) + '\nENDTURN\n').encode()

        def death(i, r, reason):
            deaths.append([r, i, reason])
            if i in live:
                live.pop(i).stop()

        try:
            EngineModule().run(replay['map'].encode(), reply, death, spawn,
                               seed=int(details['match']['seed'], 16), on_notice=lambda _: None)
        except Done:
            pass
        finally:
            for bot in live.values():
                bot.stop()
            pool.close()
    result = {'trace_fields': 'complete, length, known_body_size, exact_decision, deliberate_fatal, head_tile',
              'deaths_match': deaths == expected_deaths, 'deaths': deaths,
              'expected_deaths': expected_deaths, 'trace': trace}
    (folder / f'{args.battle_id}.trace.json').write_text(json.dumps(result, indent=2) + '\n')
    print(f"{args.battle_id}: {len(trace)} decisions, {sum(t['same'] for t in trace)} match; deaths match: {result['deaths_match']}")


if __name__ == '__main__':
    main()
