"""Small real-engine handoff fixtures, not a match-strength benchmark.

Shift round headers to 450 so the real policy reaches its late phase without
450 rounds of warmup. All sonar routing, movement, deaths and pearls are handled
by the official engine. Two remote allies provide the real population floor.
"""
import json
import hashlib
from pathlib import Path
import shutil
import tempfile
from unswbc.engine import EngineModule
from unswbc.run import _resolve
from unswbc.bot import Pool, Bot

ROOT = Path(__file__).resolve().parents[1]

def fixture(reverse):
    width, height = 21, 15
    def edge(x, y, d):
        if d == 'E': x = (x+1) % width
        if d == 'S': y = (y+1) % height
        return (2*y+(d in 'EW'))*(width+1)+x
    walls = set()
    for k in range(4, 11):
        walls.update((edge(k, 4, 'N'), edge(k, 10, 'S'), edge(4, k, 'W'), edge(10, k, 'E')))
    collector = [(8, 8), (8, 7), (8, 6), (7, 6), (6, 6), (6, 7), (6, 8), (7, 8)]
    worker = [(8, 10), (7, 10)]
    pair = [collector, worker] if reverse else [worker, collector]
    bodies = pair + [[(1, 1), (0, 1)], [(1, 13), (0, 13)], [(18, 2), (17, 2)]]
    lines = [f'MAP {width} {height}', 'TILE_COUNT 0', f'EDGE_COUNT {len(walls)}']
    lines += [f'EDGE {k} 1 -1' for k in sorted(walls)]
    lines += ['DRAGON_COUNT 5']
    for i, body in enumerate(bodies):
        lines.append(f'DRAGON {int(i==4)} {len(body)} '+' '.join(f'{x} {y}' for x, y in body))
    return ('\n'.join(lines)+'\n').encode(), int(not reverse), int(reverse)

class Finished(Exception): pass

def run(folder, reverse):
    argv, cwd, _ = _resolve(str(folder))
    pool = Pool(argv, cwd=str(cwd), size=1)
    live, events, turns, deaths = {}, [], [], []
    data, collector, donor = fixture(reverse)
    engine = EngineModule()
    def spawn(i, init):
        if i < 2: live[i] = Bot(pool, init=init, name=str(i))
    def reply(i, data):
        r = int(data.splitlines()[0].split()[1])
        if r >= 28: raise Finished()
        if i >= 2: return b'MOVE E\nPROTOCOL 3\nENDTURN\n'
        shifted = data.replace(f'ROUND {r}\n'.encode(), f'ROUND {r+450}\n'.encode(), 1)
        out = live[i].ask(shifted)
        assert not live[i].error, live[i].error
        turns.append({'id':i, 'round':r+450, 'reply':out.decode()})
        for line in out.decode().splitlines():
            if line.startswith('LOG COORD '):
                v=list(map(int,line.split()[2:]));events.append({'id':i,'round':r+450,'event':v[0],'length':v[1],'station':v[2],'leader':v[3],'offers':v[4],'head':v[5],'complete':v[6]})
        return out
    def death(i, r, why):
        deaths.append({'id':i,'round':r+450,'reason':why})
        if i in live: live.pop(i).stop()
    try:
        try: engine.run(data,reply,death,spawn,seed=731,on_notice=lambda x:None)
        except Finished: pass
    finally:
        for bot in live.values(): bot.stop()
        pool.close()
    accepted=[e for e in events if e['id']==collector and e['event']==1]
    donated=[e for e in events if e['id']==donor and e['event']==3]
    collected=[e for e in events if e['id']==collector and e['event']==2]
    return {'collector':collector,'donor':donor,'accepted':accepted,'donated':donated,'collected':collected,'deaths':deaths,'events':events,'turns':turns}

def main():
    digest=hashlib.sha256()
    for name in sorted(('bot.toml','main.cpp','helper.hpp','strategy.hpp','config.hpp')):
        digest.update(name.encode()+b'\0'+(ROOT/'bot'/name).read_bytes()+b'\0')
    with tempfile.TemporaryDirectory(prefix='v7-coordination-') as tmp:
        folder=Path(tmp)
        for name in ('bot.toml','main.cpp','helper.hpp','strategy.hpp','config.hpp'):
            shutil.copyfile(ROOT/'bot'/name,folder/name)
        p=folder/'main.cpp'
        p.write_text(p.read_text().replace('brain.commit(decision,ct);',
            'ct.output_log("COORD",brain.coord_event,brain.length,brain.station,brain.leader,brain.delivery_offers.size(),brain.head,brain.complete); '
            'for(auto o:brain.delivery_offers){bool exact=false;auto body=brain.visible_body(o.id,o.pos,exact);ct.output_log("OFFER",o.id,o.pos,exact,body.size(),brain.distance(brain.head,o.pos),brain.expired(.040));}'
            'brain.commit(decision,ct);'))
        results=[run(folder,False),run(folder,True)]
        for row in results: row['source_fingerprint']=digest.hexdigest()
    output=ROOT/'evaluations/v7-review/coordination-engine.json'
    output.write_text(json.dumps(results,indent=2)+'\n')
    for row in results:
        assert row['accepted'] and row['donated'] and row['collected'], (row['collector'], 'No completed handshake; inspect saved fixture')
        assert any(x['id']==row['donor'] and x['reason']=='S' for x in row['deaths'])
        meal=row['collected'][0]
        assert any(e['id']==row['collector'] and e['round']==meal['round']+1 and e['length']==meal['length']+1 for e in row['events']), 'Engine did not confirm the planned length gain'
        print(f"Collector {row['collector']} / donor {row['donor']}: agreement, self-death and actual collection verified")

if __name__=='__main__': main()
