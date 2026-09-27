"""Check that wall/self deaths never follow a move the bot claimed was exact and safe."""
from pathlib import Path
import tempfile,shutil,subprocess
from unswbc.engine import EngineModule
from unswbc.run import _resolve
from unswbc.bot import Pool,Bot
ROOT=Path(__file__).resolve().parents[1]
with tempfile.TemporaryDirectory(prefix='abyss-audit-') as tmp:
    folder=Path(tmp)
    for name in ('bot.toml','main.cpp','helper.hpp','strategy.hpp','config.hpp'):shutil.copyfile(ROOT/'bot'/name,folder/name)
    p=folder/'main.cpp';p.write_text(p.read_text().replace('brain.commit(decision,ct);','ct.output_log("AUDIT",decision.exact,decision.dying); brain.commit(decision,ct);'))
    argv,cwd,kind=_resolve(str(folder));other,othercwd,_=_resolve(str(ROOT/'baselines/greedy'))
    pools={'A':Pool(argv,cwd=str(cwd),size=1),'B':Pool(other,cwd=str(othercwd),size=1)}
    live={};teams={};last={};errors=[];checked=[]
    def spawn(i,init):
        team=next(l.split()[1] for l in init.decode().splitlines() if l.startswith('TEAM '));teams[i]=team;live[i]=Bot(pools[team],init=init,name=str(i))
    def reply(i,data):
        out=live[i].ask(data)
        if live[i].error:errors.append(str(live[i].error))
        for line in out.decode().splitlines():
            if line.startswith('LOG AUDIT '):last[i]=line.split()[2:]
        return out
    def death(i,r,reason):
        if teams[i]=='A' and reason in ('W','S'):
            checked.append((i,r,reason,last.get(i)))
            if last.get(i)==['1','0']:errors.append(f'Unexpected {reason} at round {r}, dragon {i}')
        live.pop(i).stop()
    try:EngineModule().run((ROOT/'maps/stronghold.map').read_bytes(),reply,death,spawn,seed=211,on_notice=lambda x:None)
    finally:
        for bot in live.values():bot.stop()
        for pool in pools.values():pool.close()
    assert not errors,errors
    assert checked, "No wall/self deaths occurred; audit had no coverage"
    print(f'PASS: {len(checked)} wall/self deaths audited; all came from explicitly unsafe last resorts, none from supposedly exact safe moves')
