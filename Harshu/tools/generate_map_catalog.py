"""Deterministic, offline catalogue. Never guesses geometry from an image.
Re-run after deliberately reviewing official map changes; revision binds sonar.
"""
from pathlib import Path
import hashlib
from collections import deque
from reassess_v6 import Board
ROOT=Path(__file__).resolve().parents[1]
# Travel pull, target lifetime, crowd penalty. Safety/splits remain general.
PROFILES={
 'autarky':(1.5,20,4,'Follow connected boxed farms; preserve productive long starters.'),
 'default':(2.0,24,3,'Seek the denser food grid through actual openings and portal links.'),
 'devil':(1.8,20,4,'Approach fast food bands through corridor gaps, spread between farms.'),
 'dilemma':(1.8,16,4,'Rescue trapped starters first; use reachable central farms and delayed outer supplies.'),
 'portals':(2.0,16,4,'Route to fast pearl fields through paired portals; unseen occupants remain unknown.'),
 'queen_of_spades':(1.6,24,4,'Distribute workers among productive pockets; retain successful growers.'),
 'schooltime':(2.2,28,3,'Cover separate resource bands across the large board through their entrances.'),
 'slithery_fight':(1.8,16,5,'Keep coil and junction rescues; send spare workers toward connected fast farms.'),
 'trauma':(1.5,24,4,'Follow maze connections toward productive patches without abandoning a working farm.'),
 'trophy':(2.2,20,4,'Leave barren corners for the richer interior through wall gaps; keep multiple approaches.'),
}
def array(name,typ,items):
 rows=[','.join(str(x) for x in items[i:i+24]) for i in range(0,len(items),24)]
 return f'inline constexpr {typ} {name}[]={{\n'+',\n'.join(rows)+'\n};\n'
def main():
 out=ROOT/'bot/maps';out.mkdir(exist_ok=True)
 digest=hashlib.sha256()
 for name in PROFILES:digest.update(name.encode()+ (ROOT/'maps'/f'{name}.map').read_bytes())
 revision=int(digest.hexdigest()[:2],16)
 common='''#pragma once
#include <array>
#include <cstdint>
#include <span>
namespace abyss::atlas {
struct Start { int offset,length,heading,team; };
struct Map {
 const char* name; int w,h; double pull; int hold; double crowd;
 std::span<const std::uint16_t> edges,lo,hi,body;
 std::span<const Start> starts;
 std::span<const std::int16_t> routes;
 std::span<const std::uint16_t> supply;
};
'''+f'inline constexpr int revision={revision};\n'+'}\n'
 (out/'types.hpp').write_text(common)
 for name,(pull,hold,crowd,comment) in PROFILES.items():
  raw=(ROOT/'maps'/f'{name}.map').read_text();b=Board(raw)
  edges=[0]*(2*b.n);lo=[0]*b.n;hi=[0]*b.n
  for k,(kind,pid) in b.edges.items():edges[k]=pid+2 if kind==2 else kind
  for line in raw.splitlines():
   t=line.split()
   if t[0]=='TILE':x,y,l,h=map(int,t[1:]);lo[y*b.w+x]=l;hi[y*b.w+x]=h
  assert max(edges)<65536 and max(hi)<65536
  bodies=[];starts=[]
  for ident,body in b.bodies.items():
   heading=next(d for d in range(4) if b.next[body[1]][d]==body[0])
   starts.append('{'+','.join(map(str,[len(bodies),len(body),heading,'AB'.index(b.teams[ident])]))+'}')
   bodies+=body
  # Expected regional income through graph distance, not a promise of food.
  supply=[]
  for p in range(b.n):
   dist={p:0};q=deque([p]);value=0
   while q:
    v=q.popleft();d=dist[v]
    if hi[v]:value+=2/(lo[v]+hi[v])/(1+d)
    if d<3:
     for v2 in b.next[v]:
      if v2>=0 and v2 not in dist:dist[v2]=d+1;q.append(v2)
   supply.append(value)
  scale=max(supply) or 1
  text='#pragma once\n#include "types.hpp"\n// '+comment+'\n'
  text+='// Exact canonical edges, spawn ranges, ordered starting bodies and routes.\n'
  text+=f'// Source SHA256: {hashlib.sha256(raw.encode()).hexdigest()}\n'
  text+=f'namespace abyss::atlas::map_{name} {{\n'
  for k,typ,values in [('edges','std::uint16_t',edges),('lo','std::uint16_t',lo),('hi','std::uint16_t',hi),('body','std::uint16_t',bodies),('starts','Start',starts),('routes','std::int16_t',[v for row in b.next for v in row]),('supply','std::uint16_t',[round(v/scale*10000) for v in supply])]:text+=array(k,typ,values)
  text+=f'inline constexpr Map data{{"{name}",{b.w},{b.h},{pull},{hold},{crowd},edges,lo,hi,body,starts,routes,supply}};\n}}\n'
  (out/f'{name}.hpp').write_text(text)
 (ROOT/'bot/map_catalog.hpp').write_text('#pragma once\n'+''.join(f'#include "maps/{x}.hpp"\n' for x in PROFILES)+'namespace abyss::atlas {\ninline constexpr std::array<const Map*,10> catalog{{'+','.join('&map_'+x+'::data' for x in PROFILES)+'}};\n}\n')
 print('Generated ten complete maps; revision',revision)
if __name__=='__main__':main()
