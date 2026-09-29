"""Extract a visibility-limited diagnostic snapshot; not a replayed decision."""
import json, sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[2]/'tools'))
from reassess_v6 import Board

source, out = map(Path, sys.argv[1:3])
who, target_round = map(int, sys.argv[3:5])
data=json.loads(source.read_text()); b=Board(data['map'])
rn=-1; active=None; pearls=set(); countdown={}; memory={}; edges={}; last_visit={}; last_food=0
for e in data['events']:
    k=e['type']
    if k=='roundStart': rn=e['round']
    elif k=='pearlCountdown': countdown[b.pos(e['tile'])]=e['countdown']
    elif k=='turnStart':
        active=e['id']
        if active==who:
            body=b.bodies[who]; view=b.views[body[0]]; last_visit[body[0]]=rn
            occ={p:(i,j) for i,bo in b.bodies.items() for j,p in enumerate(bo)}
            for p in view:
                old=memory.get(p); has=p in pearls
                cell=[rn,int(has),rn,old[3] if old and old[1] and has else rn if has else -1,
                      rn+countdown.get(p,-1) if countdown.get(p,-1)>=0 else -1,-1,-1,0,0]
                if p in occ:
                    i,j=occ[p]; cell[5:8]=[i,'AB'.index(b.teams[i]),int(j==0)]
                    if j: cell[8]=next(d for d,v in enumerate(b.next[p]) if v==b.bodies[i][j-1])
                memory[p]=cell
                for d in range(4): edges[p,d]=b.edges.get(b.key(p,d),(0,-1))
            if rn==target_round: break
    elif k=='tileChange':
        p=b.pos(e['tile'])
        if e['hasPearl']: pearls.add(p)
        else:
            if p in pearls and active==who: last_food=rn
            pearls.discard(p)
    elif k=='dragonUpdate' and rn>=0:
        body=b.bodies[e['id']]; body.insert(0,b.pos(e['head']))
        while len(body)>1 and body[-1]!=b.pos(e['tail']):body.pop()
    elif k=='dragonSplit' and rn>=0:
        i,j=e['parentId'],e['childId']; b.bodies[i]=[b.pos(p) for p in e['parentBody']]
        b.bodies[j]=[b.pos(p) for p in e['childBody']];b.teams[j]=b.teams[i]
    elif k=='dragonDeath' and rn>=0: del b.bodies[e['id']]
body=b.bodies[who]
heading=next((d for d,v in enumerate(b.next[body[1]]) if v==body[0]),0)
lines=[f'{b.w} {b.h} {who} {"AB".index(b.teams[who])} {rn} {sum(b.teams[i]==b.teams[who] for i in b.bodies)} {len(body)} {heading} {last_food}',
       ' '.join(map(str,body)),str(len(memory))]
lines += [' '.join(map(str,[p,*cell])) for p,cell in memory.items()]
lines += [str(len(edges))]+[' '.join(map(str,[p,d,*edge])) for (p,d),edge in edges.items()]
lines += [str(len(last_visit))]+[f'{p} {r}' for p,r in last_visit.items()]
out.write_text('\n'.join(lines)+'\n')
