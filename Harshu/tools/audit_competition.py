"""Phase-level competition evidence from decoded official replays (no bot games).

A skipped visible pearl is a diagnostic, not proof that taking it was safe.
Natural-spawn density is hindsight, never claimed as information the bot had.
"""
import argparse,json
from pathlib import Path
from collections import Counter,defaultdict

def analyze(path):
 data=json.loads(path.read_text());lines=data['map'].splitlines();w,h=map(int,lines[0].split()[1:])
 pos=lambda p:p['y']*w+p['x']
 def adjacent(p,d):return ((p//w+(d==2)-(d==0))%h)*w+(p%w+(d==1)-(d==3))%w
 def distance(p,q):
  dx,dy=abs(p%w-q%w),abs(p//w-q//w)
  return min(dx,w-dx)+min(dy,h-dy)
 def visible(p,q):
  dx,dy=abs(p%w-q%w),abs(p//w-q//w)
  return min(dx,w-dx)<=3 and min(dy,h-dy)<=3
 edges={};portals=defaultdict(set);bodies={};teams={};pearls=set()
 for line in lines:
  t=line.split()
  if t[0]=='EDGE':
   index,kind,pid=map(int,t[1:]);row,x=divmod(index,w+1);p=(row//2%h)*w+x%w;key=2*p+(row%2==1)
   edges[key]=(kind,pid)
   if kind==2:portals[pid].add(key)
  elif t[0]=='DRAGON':
   v=list(map(int,t[1:]));i=len(bodies);teams[i]='AB'[v[0]];bodies[i]=[y*w+x for x,y in zip(v[2::2],v[3::2])]
 def crossing(p,d):
  key=2*(adjacent(p,d) if d in (1,2) else p)+(d in (1,3));kind,pid=edges.get(key,(0,-1))
  if kind==1:return None
  if kind==0:return adjacent(p,d)
  far=next(k for k in portals[pid] if k!=key)//2
  return adjacent(far,d) if d in (0,3) else far
 phases=defaultdict(Counter);births={i:0 for i in bodies};food=Counter();spawns=Counter();heads=Counter();skips=[];trades=[];r=-1;active=None;turn=None
 for e in data['events']:
  kind=e['type']
  if kind=='roundStart':r=e['round'];active=None
  elif kind=='turnStart':
   active=e['id'];body=bodies[active];side=teams[active];occupied={p for b in bodies.values() for p in b}
   enemy=[(i,b[0],len(b)) for i,b in bodies.items() if teams[i]!=side and visible(body[0],b[0])]
   turn={'round':r,'id':active,'team':side,'length':len(body),'head':body[0],
    'pearls':[(d,crossing(body[0],d)) for d in range(4) if crossing(body[0],d) in pearls and crossing(body[0],d) not in occupied and visible(body[0],crossing(body[0],d))],
    'enemy':enemy}
   phases[r//100,side]['turns']+=1;heads[r//100,side,body[0]]+=1
  elif kind=='dragonAction' and r>=0:
   a=e['action'] or {};side=teams[active];phase=phases[r//100,side]
   if a.get('kind')=='suicide':phase['explicit_suicides']+=1
   if a.get('kind')=='move':
    phase['sprint_cost_requested']+=max(0,len(a['steps'])-1)
    d='NESW'.index(a['steps'][0]);dest=crossing(bodies[active][0],d)
    if turn['pearls'] and dest not in [p for _,p in turn['pearls']]:
     phase['skipped_legal_adjacent_pearl']+=1
     if len(skips)<150:skips.append({**turn,'action':a['steps'],'enemy_distance_to_food':min((distance(p,x) for _,p in turn['pearls'] for _,x,_ in turn['enemy']),default=99)})
  elif kind=='tileChange':
   p=pos(e['tile'])
   if e['hasPearl']:
    pearls.add(p)
    if active is None and r>=0:spawns[r//100,p]+=1
   else:
    pearls.discard(p)
    if active in bodies and r>=0:
     phases[r//100,teams[active]]['pearls']+=1;food[r//100,teams[active],p]+=1
  elif kind=='dragonUpdate' and r>=0:
   b=bodies[e['id']];b.insert(0,pos(e['head']))
   while len(b)>1 and b[-1]!=pos(e['tail']):b.pop()
  elif kind=='dragonSplit' and r>=0:
   bodies[e['parentId']]=[pos(p) for p in e['parentBody']];bodies[e['childId']]=[pos(p) for p in e['childBody']];teams[e['childId']]=e['team'];births[e['childId']]=r
   phases[r//100,e['team']]['splits']+=1
  elif kind=='dragonDeath' and r>=0:
   i=e['id'];phases[r//100,teams[i]]['deaths_'+e['reason']]+=1
   if e['reason']=='H':trades.append({'round':r,'initiator':active,'initiator_team':teams[active], 'id':i,'team':teams[i],'length':len(bodies[i])})
   del bodies[i]
 snapshots=[]
 for state in data['rounds']:
  if state['round']%50==0 or state is data['rounds'][-1]:
   snapshots.append({'round':state['round'],**{s:{**state[s],'length_histogram':dict(Counter(d['length'] for d in state['dragons'] if d['team']==s))} for s in 'AB'}})
 productive={p for (_,p),count in spawns.items() if count}
 # Central half by each axis is a descriptive region, not a strategy assumption.
 center={y*w+x for y in range(h//4,h-h//4) for x in range(w//4,w-w//4)}
 return {'id':int(path.name.split('.')[0]),'dimensions':[w,h],
  'phases':[{'phase':phase,'team':s,**c} for (phase,s),c in sorted(phases.items())],
  'snapshots':snapshots,'skipped_pearl_examples':skips,'head_collision_participants':trades,
  'central_natural_spawns':sum(c for (_,p),c in spawns.items() if p in center),'all_natural_spawns':sum(spawns.values()),
  'spatial':[{'phase':phase,'team':s,'head_turns_center':sum(c for (a,b,p),c in heads.items() if a==phase and b==s and p in center),
   'head_turns_productive':sum(c for (a,b,p),c in heads.items() if a==phase and b==s and p in productive),
   'unique_head_tiles':len({p for a,b,p in heads if a==phase and b==s}),
   'pearls_center':sum(c for (a,b,p),c in food.items() if a==phase and b==s and p in center)} for phase,s in sorted(phases)]}

def main():
 p=argparse.ArgumentParser();p.add_argument('folder',type=Path);a=p.parse_args()
 games=[analyze(f) for f in sorted(a.folder.glob('[0-9]*.events.json'))]
 (a.folder/'competition.json').write_text(json.dumps({'games':games},indent=2)+'\n')
 for g in games:
  print(g['id'],'natural spawns center/all',g['central_natural_spawns'],g['all_natural_spawns'])
  print('phases',g['phases'])
  print('first100 spatial',g['spatial'][:2])
if __name__=='__main__':main()
