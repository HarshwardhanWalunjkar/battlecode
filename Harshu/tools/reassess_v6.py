"""Independent full-event audit: verified bodies, visibility, sonar and concentration.
Uses hindsight only for labelled structural/opportunity metrics, never as bot input.
Downloaded replay maps redact spawn timers; no decision replay is claimed.
"""
import argparse,json
from pathlib import Path
from collections import Counter,defaultdict,deque

class Board:
 def __init__(self,text):
  self.w,self.h=map(int,text.splitlines()[0].split()[1:]);self.n=self.w*self.h
  self.edges={};self.portals=defaultdict(set);self.bodies={};self.teams={}
  for line in text.splitlines():
   t=line.split()
   if t[0]=='EDGE':
    idx,kind,pid=map(int,t[1:]);row,x=divmod(idx,self.w+1);p=(row//2%self.h)*self.w+x%self.w;k=2*p+(row%2)
    self.edges[k]=(kind,pid)
    if kind==2:self.portals[pid].add(k)
   elif t[0]=='DRAGON':
    v=list(map(int,t[1:]));i=len(self.bodies);self.teams[i]='AB'[v[0]];self.bodies[i]=[y*self.w+x for x,y in zip(v[2::2],v[3::2])]
  self.next=[[self.cross(p,d) for d in range(4)] for p in range(self.n)]
  self.views=[{((p//self.w+y)%self.h)*self.w+(p%self.w+x)%self.w for y in range(-3,4) for x in range(-3,4)} for p in range(self.n)]
 def pos(self,p):return p['y']*self.w+p['x']
 def adj(self,p,d):return ((p//self.w+(d==2)-(d==0))%self.h)*self.w+(p%self.w+(d==1)-(d==3))%self.w
 def key(self,p,d):return 2*(self.adj(p,d) if d in [1,2] else p)+(d in [1,3])
 def cross(self,p,d):
  k=self.key(p,d);kind,pid=self.edges.get(k,(0,-1))
  if kind==1:return -1
  if kind==0:return self.adj(p,d)
  far=next(a for a in self.portals[pid] if a!=k)//2
  return self.adj(far,d) if d in [0,3] else far
 def distances(self,start,occupied,cap=4):
  dist={start:0};q=deque([start])
  while q:
   p=q.popleft()
   if dist[p]>=cap:continue
   for v in self.next[p]:
    if v<0 or v in occupied or v in dist:continue
    dist[v]=dist[p]+1;q.append(v)
  return dist

def audit(path):
 data=json.loads(path.read_text());b=Board(data['map']);pearl=set();r=-1;active=None;chosen=None
 counts=defaultdict(Counter);ever=defaultdict(set);teamseen=defaultdict(set);born={i:0 for i in b.bodies};splits=[];deaths=[];meals=[];drops={};snapshots={x['round']:x for x in data['rounds']};checks=0;examples=[];initial={i:len(x) for i,x in b.bodies.items()};turnview=set()
 def check(round):
  nonlocal checks
  state=snapshots.get(round)
  if not state:return
  expected={d['id']:(d['team'],d['length'],d['y']*b.w+d['x']) for d in state['dragons']}
  actual={i:(b.teams[i],len(body),body[0]) for i,body in b.bodies.items()}
  assert expected==actual,(path,round);checks+=1
 for e in data['events']:
  k=e['type']
  if k=='roundStart':r=e['round'];active=None;check(r)
  elif k=='turnStart':
   active=e['id'];chosen=None;body=b.bodies[active];s=b.teams[active];view=b.views[body[0]];turnview=view
   c=counts[r//100,s];c['turns']+=1;c['head_length_sum']+=len(body)
   c['new_personal_tiles']+=len(view-ever[active]);c['new_team_tiles']+=len(view-teamseen[s]);ever[active]|=view;teamseen[s]|=view
   visible_prefix=0
   for p in body:
    if p not in view:break
    visible_prefix+=1
   if visible_prefix<len(body):c['partial_body_turns']+=1
   # Classify possible food captures using actual state, not a replayed bot decision.
   occ={p for x in b.bodies.values() for p in x};legal=[(d,v) for d,v in enumerate(b.next[body[0]]) if v>=0 and v in view and v not in occ]
   candidates=[]
   for d,v in legal:
    if v not in pearl:continue
    exits=sum(q>=0 and q not in occ and q!=body[0] for q in b.next[v])
    visible_enemies=[i for i,x in b.bodies.items() if b.teams[i]!=s and x[0] in view]
    dist=min((b.distances(b.bodies[i][0],occ-{v},3).get(v,99) for i in visible_enemies),default=99)
    candidates.append((d,v,exits,dist))
   turn={'r':r,'id':active,'team':s,'length':len(body),'legal_pearl_candidates':candidates,'body_prefix':visible_prefix,'body':list(body)}
   if r>=300 and len(body)<=5:
    # Labelled hindsight opportunity, not proof donor can certify it or send sonar.
    routes=b.distances(body[0],occ-{body[0]},4)
    for j,other in b.bodies.items():
     if j==active or b.teams[j]!=s or len(other)<max(6,len(body)+2) or other[0] not in view:continue
     adjacent=[p for p in b.next[other[0]] if p>=0 and p in routes]
     if adjacent:
      c['worker_turns_near_grower']+=1
      if j<active:c['near_grower_acted_already']+=1
      break
  elif k=='dragonAction':
   chosen=e['action'] or {};c=counts[r//100,b.teams[active]]
   if chosen.get('kind')=='move':
    steps=chosen['steps'];c['sprint_cost_requested']+=len(steps)-1
    dest=b.next[b.bodies[active][0]]['NESW'.index(steps[0])]
    if turn['legal_pearl_candidates'] and dest not in [v for _,v,_,_ in turn['legal_pearl_candidates']]:
     c['skipped_adjacent_food']+=1
     if any(exits>=2 and dist>1 for _,v,exits,dist in turn['legal_pearl_candidates']):
      c['skipped_food_two_exits_no_one_step_head']+=1
      if len(examples)<20:examples.append({**turn,'action':steps})
   elif chosen.get('kind')=='suicide':c['explicit_suicides']+=1
  elif k=='tileChange':
   p=b.pos(e['tile'])
   if e['hasPearl']:pearl.add(p)
   else:
    if p in pearl and active in b.bodies:
     c=counts[r//100,b.teams[active]];c['food']+=1
     if p in drops:
      donor=drops.pop(p);meals.append({'r':r,'recipient':active,'team':b.teams[active],'length_before':len(b.bodies[active]),'cell':p,**donor})
    pearl.discard(p)
  elif k=='dragonUpdate' and r>=0:
   body=b.bodies[e['id']];body.insert(0,b.pos(e['head']))
   while len(body)>1 and body[-1]!=b.pos(e['tail']):body.pop()
  elif k=='dragonSplit' and r>=0:
   i,j=e['parentId'],e['childId'];before=len(b.bodies[i]);s=b.teams[i];c=counts[r//100,s];c['splits']+=1
   splits.append({'r':r,'id':i,'child':j,'team':s,'before':before,'front':len(e['parentBody']),'rear':len(e['childBody']),'age':r-born[i],'prefix':turn['body_prefix']})
   b.bodies[i]=[b.pos(p) for p in e['parentBody']];b.bodies[j]=[b.pos(p) for p in e['childBody']];b.teams[j]=s;born[j]=r
  elif k=='dragonDeath' and r>=0:
   i=e['id'];s=b.teams[i];body=b.bodies[i];counts[r//100,s]['death_'+e['reason']]+=1
   deaths.append({'r':r,'id':i,'team':s,'length':len(body),'reason':e['reason'],'age':r-born[i],'action':chosen if active==i else None})
   for p in body[::2]:drops[p]={'donor':i,'donor_team':s,'death_r':r,'donor_length':len(body),'death_reason':e['reason']}
   del b.bodies[i]
  elif k=='sonarPing' and r>=0:
   sender=e['senderId'];s=b.teams[sender];c=counts[r//100,s];c['sonar_'+str(e['hitKind'])]+=1
   dest=b.pos(e['end']);receiver=next((i for i,x in b.bodies.items() if dest in x),None)
   if receiver is not None and b.teams[receiver]==s:
    c['sonar_self' if receiver==sender else 'sonar_other_ally']+=1
    if receiver<sender:c['sonar_lower_id']+=1
 check(data['rounds'][-1]['round'])
 return {'game':path.stem,'checks':checks,'result':data['result'],'phase_metrics':[{'phase':k[0],'team':k[1],**v} for k,v in sorted(counts.items())],'food_skip_examples':examples,'splits':splits,'deaths':deaths,'corpse_meals':meals,'checkpoints':[x for x in data['rounds'] if x['round'] in [0,50,100,150,200,250,300,350,400,450,500]],'initial_lengths':initial}

def main():
 p=argparse.ArgumentParser();p.add_argument('files',nargs='+',type=Path);p.add_argument('--output',required=True,type=Path);a=p.parse_args()
 out=[]
 for f in a.files:
  x=audit(f);out.append(x);print(f.name,'verified',x['checks'],'final',x['result'],flush=True)
 a.output.write_text(json.dumps(out,indent=2)+'\n')
if __name__=='__main__':main()
