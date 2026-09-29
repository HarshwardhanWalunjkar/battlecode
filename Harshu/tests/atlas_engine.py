"""Two opening rounds on all templates: real sensors, ID ordering and newborns.
This checks integration, not competitive strength. One native compile, no sweeps.
"""
import json,shutil,tempfile
from pathlib import Path
from unswbc.engine import EngineModule
from unswbc.bot import Pool,Bot
from unswbc.run import _resolve
from unswbc.project import Project
ROOT=Path(__file__).resolve().parents[1]
NAMES=['autarky','default','devil','dilemma','portals','queen_of_spades','schooltime','slithery_fight','trauma','trophy']
class Done(Exception):pass

def main():
 rows=[]
 with tempfile.TemporaryDirectory(prefix='atlas-contract-') as tmp:
  tmp=Path(tmp);project=Project.from_dir(ROOT/'bot');project.collect_sources()
  for name in sorted(set(['bot.toml',*project.sources])):
   (tmp/name).parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/'bot'/name,tmp/name)
  p=tmp/'main.cpp';p.write_text(p.read_text().replace('auto decision=brain.choose();','ct.output_log("ATLAS",brain.round,brain.id,brain.map_knowledge.mode,brain.map_knowledge.candidate,brain.rescued_grower); auto decision=brain.choose();'))
  argv,cwd,_=_resolve(str(tmp))
  for code,name in enumerate(NAMES,1):
   pool=Pool(argv,cwd=str(cwd),size=1);live={};turns=[];deaths=[]
   raw=(ROOT/'maps'/f'{name}.map').read_bytes();initial=sum(line.startswith(b'DRAGON ') for line in raw.splitlines())
   def spawn(i,init):live[i]=Bot(pool,init=init,name=str(i))
   def reply(i,data):
    r=int(data.splitlines()[0].split()[1])
    if r>=2:raise Done()
    out=live[i].ask(data);assert not live[i].error,str(live[i].error)
    log=next(x for x in out.decode().splitlines() if x.startswith('LOG ATLAS '));v=list(map(int,log.split()[2:]))
    turns.append(v)
    if i<initial and r==0:assert v[3]==code,(name,v)
    assert v[2] in [0,code],(name,v)
    return out
   def death(i,r,why):deaths.append([i,r,why]);live.pop(i).stop()
   try:
    try:EngineModule().run(raw,reply,death,spawn,seed=911,on_notice=lambda _:None)
    except Done:pass
   finally:
    for b in live.values():b.stop()
    pool.close()
   rows.append({'map':name,'turns':turns,'deaths':deaths})
   print(name,'turns',len(turns),'newborns',len({t[1] for t in turns if t[1]>=initial}),flush=True)
 out=ROOT/'evaluations/v8-review/atlas-engine.json';out.write_text(json.dumps(rows,indent=2)+'\n')
if __name__=='__main__':main()
