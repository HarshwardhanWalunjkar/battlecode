"""Bounded official-engine opening probes; no competitive full game."""
import json, shutil, tempfile
from pathlib import Path
from unswbc.engine import EngineModule
from unswbc.run import _resolve
from unswbc.bot import Pool, Bot
ROOT=Path(__file__).resolve().parents[2]
OUT=Path(__file__).resolve().parent
class Done(Exception):pass

def run(rounds=5, override=None, label='baseline'):
    trace=[];deaths=[];live={};spawns={};inputs={}
    with tempfile.TemporaryDirectory(prefix='slithery-v7-audit-') as tmp:
        tmp=Path(tmp)
        for p in (ROOT/'versions/v7/bot').iterdir():
            if p.is_file():shutil.copy(p,tmp/p.name)
        main=tmp/'main.cpp'
        diagnostic='''
            auto original=brain.initial();
            ct.output_log("AUDIT",brain.round,brain.id,brain.complete,brain.length,brain.predicted.size(),brain.head,brain.rescue_history);
            std::cout << "LOG BODY";for(int p:brain.predicted)std::cout<<" "<<p;std::cout<<"\\n";
            if(brain.complete&&brain.length>=4)for(int c=2;c<=brain.length-2;c++){
                auto rear=brain.escape(brain.split_part(original,c,true),6);
                auto front=brain.escape(brain.split_part(original,c,false),6);
                ct.output_log("CUT",c,rear.depth,rear.verified,front.depth,front.verified);
            }
            for(int d=0;d<4;d++)ct.output_log("EDGE",d,brain.cells[brain.head].kind[d],brain.next[brain.head][d]);
'''
        main.write_text(main.read_text().replace('auto decision=brain.choose();',diagnostic+'\n            auto decision=brain.choose();'))
        argv,cwd,_=_resolve(str(tmp));pool=Pool(argv,cwd=str(cwd),size=1)
        def spawn(i,data):
            spawns[i]=data.decode();live[i]=Bot(pool,init=data,name=str(i))
        def reply(i,data):
            r=int(data.decode().splitlines()[0].split()[1])
            if r>=rounds:raise Done()
            output=live[i].ask(data).decode()
            inp=data.decode();inputs[f'{r},{i}']=inp
            command=next((l for l in output.splitlines() if l.startswith(('MOVE ','SPLIT '))), '')
            overridden=override(r,i,inp,command) if override else None
            if overridden:
                output=output.replace(command,overridden);command=overridden
            trace.append({'r':r,'id':i,'command':command,'log':[l for l in output.splitlines() if l.startswith('LOG ')], 'error':str(live[i].error) if live[i].error else None})
            return output.encode()
        def death(i,r,why):
            deaths.append([r,i,why]);live.pop(i).stop()
        try:EngineModule().run((ROOT/'maps/slithery_fight.map').read_bytes(),reply,death,spawn,seed=823,on_notice=lambda _:None)
        except Done:pass
        finally:
            for bot in live.values():bot.stop()
            pool.close()
    result={'label':label,'max_rounds':rounds,'seed':823,'trace':trace,'deaths':deaths,'inputs':inputs,'spawns':spawns}
    (OUT/f'slithery-{label}.json').write_text(json.dumps(result,indent=2)+'\n')
    print(label,'deaths:',deaths,'decisions:',len(trace))
    return result
if __name__=='__main__':run()
