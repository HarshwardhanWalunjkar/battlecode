"""Independent behavioral probes against the shipped official WASM engine."""
from pathlib import Path
from unswbc.engine import EngineModule
E=EngineModule()
def map_text(edges=(),a=(60,59,58),tiles=()):
    def coords(body):return ' '.join(f'{p%11} {p//11}' for p in body)
    return ('MAP 11 11\nTILE_COUNT '+str(len(tiles))+'\n'+''.join(f'TILE {x} {y} {lo} {hi}\n' for x,y,lo,hi in tiles)+'EDGE_COUNT '+str(len(edges))+'\n'+''.join(f'EDGE {i} {kind} {portal}\n' for i,kind,portal in edges)+f'DRAGON_COUNT 2\nDRAGON 0 {len(a)} {coords(a)}\nDRAGON 1 3 2 1 1 1 0 1\n').encode()
def run(data,commands):
    turns={};dead=[]
    def reply(i,b):
        lines=b.decode().splitlines();r=int(lines[0].split()[1]);turns[i,r]=lines
        cmd=commands.get((i,r),'MOVE E' if i==1 else 'MOVE S')
        return (cmd+'\nPROTOCOL 3\nENDTURN\n').encode()
    E.run(data,reply,lambda i,r,why:dead.append((i,r,why)),seed=4,on_notice=lambda x:None)
    return turns,dead
def body(lines,i=0):
    k=next(k for k,l in enumerate(lines) if l.startswith('DRAGON_BODIES '));n=int(lines[k].split()[1]);result=[]
    for l in lines[k+1:k+1+n]:
        t,ident,x,y,d,h=l.split()
        if int(ident)==i:result.append((int(x)+11*int(y),int(h)))
    return result
p,d=run(map_text(edges=[(138,2,17),(206,2,17)]),{(0,0):'MOVE E'})
assert (90,1) in body(p[0,1]),body(p[0,1])
assert p[0,0][0]=='ROUND 0'
p,d=run(map_text(),{(0,0):'MOVE EE'})
assert 'LENGTH 2' in p[0,1] and (62,1) in body(p[0,1])
p,d=run(map_text(a=(60,59)),{(0,0):'MOVE EE'})
assert (0,0,'A') in d
p,d=run(map_text(a=(60,59,48,49)),{(0,0):'MOVE N'})
assert (0,0,'S') in d
p,d=run(map_text(a=(60,59,58,57)),{(0,0):'SPLIT 2'})
assert (2,0) in p and 'LENGTH 2' in p[2,0]
assert (57,1) in body(p[2,0],2)
assert any(l.startswith('ECHOES ') for l in p[2,0])
p,d=run(map_text(),{(0,0):'MOVE E\ngarbage'})
assert (61,1) in body(p[0,1])
print('6 official-engine contracts passed: zero-based rounds, portals, sprint, own tail, same-round child/protocol, malformed-line preservation')
