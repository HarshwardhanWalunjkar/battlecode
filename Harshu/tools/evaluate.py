#!/usr/bin/env python3
"""Reproducible matches through the official engine; never submits anything."""
import argparse, dataclasses, hashlib, json, pathlib, statistics, time, shutil
from collections import Counter
from importlib.metadata import version
from unswbc.engine import EngineModule
from unswbc.bot import Pool, Bot
from unswbc.run import _resolve
from unswbc.sandbox import WasmPool, SandboxBot

ROOT=pathlib.Path(__file__).resolve().parents[1]
def fingerprint(folder):
    from unswbc.project import Project
    p=Project.from_dir(folder);p.collect_sources()
    h=hashlib.sha256()
    for name in sorted(set(['bot.toml',*p.sources])):
        h.update(name.encode()+b'\0'+(p.path/name).read_bytes()+b'\0')
    return h.hexdigest()

def match(engine, map_path, projects, resolved, seed, sandbox=False, replay=None):
    pools={};live={};teams={};metrics=[];errors=[];deaths=[];actions=Counter();notices=[]
    started=time.monotonic()
    for team,path in projects.items():
        argv,cwd,kind=resolved[path]
        pools[team]=WasmPool(argv,cwd=str(cwd),key=f'{seed:016x}-{team.lower()}') if sandbox else Pool(argv,cwd=str(cwd),size=1)
    cls=SandboxBot if sandbox else Bot
    def spawn(i,init):
        team=next(line.split()[1] for line in init.decode().splitlines() if line.startswith('TEAM '))
        teams[i]=team;live[i]=cls(pools[team],init=init,name=str(i))
    def reply(i,data):
        bot=live[i];out=bot.ask(data)
        if bot.error:errors.append({'id':i,'team':teams[i],'error':str(bot.error)})
        if sandbox and getattr(bot,'live',None):metrics.append((teams[i],*bot.live))
        for line in out.decode().splitlines():
            if line.startswith('SPLIT '):actions[teams[i]+'_splits']+=1
            if line.startswith('MOVE '):actions[teams[i]+'_moves']+=1;actions[teams[i]+'_sprint_steps']+=max(0,len(line.split()[1])-1)
        return out
    def death(i,r,reason):
        deaths.append({'id':i,'team':teams[i],'round':r,'reason':reason})
        if i in live:live.pop(i).stop()
    try:
        result=engine.run(map_path.read_bytes(),reply,death,spawn,notices.append,seed=seed)
        if replay:
            replay.parent.mkdir(parents=True,exist_ok=True);replay.write_bytes(engine.replay(projects['A'],projects['B']))
    finally:
        for b in live.values():b.stop()
        for p in pools.values():p.close()
    data=dataclasses.asdict(result)
    data.update(map=map_path.stem,seed=seed,projects=projects,seconds=round(time.monotonic()-started,3),errors=errors,deaths=deaths,actions=dict(actions),notices=notices)
    for team in 'AB':
        m=[v for v in metrics if v[0]==team]
        if m:data[team+'_budget']={'turns':len(m),'max_points':max(v[1] for v in m),'p99_points':sorted(v[1] for v in m)[min(len(m)-1,int(len(m)*.99))],'max_memory':max(v[2] for v in m)}
    return data

def main():
    p=argparse.ArgumentParser();p.add_argument('--bot',default='bot');p.add_argument('--opponents',nargs='+',default=['baselines/starter']);p.add_argument('--maps',nargs='+',default=['arena','default_small','autarky','devil','dilemma','stronghold']);p.add_argument('--seeds',nargs='+',type=int,default=[11,29]);p.add_argument('--sandbox',action='store_true');p.add_argument('--output',default='evaluations/results.json');p.add_argument('--replays',action='store_true');a=p.parse_args()
    paths=set([a.bot,*a.opponents]);resolved={}
    fingerprints={path:fingerprint(path) for path in paths}
    from unswbc.project import Project
    for path in sorted(paths):
        frozen=ROOT/'evaluations'/'builds'/fingerprints[path]
        frozen.mkdir(parents=True,exist_ok=True)
        project=Project.from_dir(path);project.collect_sources()
        for name in sorted(set(['bot.toml',*project.sources])):
            target=frozen/name;target.parent.mkdir(parents=True,exist_ok=True)
            if not target.exists():shutil.copyfile(project.path/name,target)
        resolved[path]=_resolve(str(frozen),a.sandbox)
    engine=EngineModule();records=[];out=pathlib.Path(a.output)
    metadata={'toolkit':version('unswbc'),'wasmtime':version('wasmtime'),'sandbox':a.sandbox,'fingerprints':fingerprints,'immutable_builds':True,'records':records}
    for opponent in a.opponents:
        for m in a.maps:
            for seed in a.seeds:
                for side in 'AB':
                    projects={'A':a.bot if side=='A' else opponent,'B':opponent if side=='A' else a.bot}
                    replay=out.parent/(out.stem+'-replays')/f'{len(records):04d}-{m}.replay' if a.replays else None
                    row=match(engine,ROOT/'maps'/f'{m}.map',projects,resolved,seed,a.sandbox,replay)
                    row.update(bot_side=side,opponent=opponent,score=0.5 if row['winner'] is None else int(row['winner']==side))
                    records.append(row);out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(metadata,indent=2))
                    score=sum(r['score'] for r in records)/len(records)
                    print(f"{len(records):3} {m:18} {seed:3} {side} vs {opponent}: {row['score']} ({row['rounds']+1} rounds), cumulative {score:.1%}",flush=True)
    print(json.dumps({'games':len(records),'score':statistics.mean(r['score'] for r in records),'wins':sum(r['score']==1 for r in records),'draws':sum(r['score']==.5 for r in records),'errors':sum(len(r['errors']) for r in records)},indent=2))
if __name__=='__main__':main()
