#include "../bot/strategy.hpp"
#include <cassert>
#include <iostream>
using namespace abyss;
Brain scene(int code,int ident=0,bool flip=false) {
 auto const& m=*atlas::catalog[code-1];auto start=m.starts[ident];
 Brain b(m.w,m.h,ident,start.team^flip);b.round=b.birth_round=0;b.length=start.length;b.head=m.body[start.offset];b.heading=start.heading;
 for(int k=0;k<start.length;k++)b.predicted.push_back(m.body[start.offset+k]);
 b.complete=true;
 for(int p=0;p<b.n;p++)if(b.distance(p,b.head)<=3 ||
   (std::min(std::abs(p%m.w-b.head%m.w),m.w-std::abs(p%m.w-b.head%m.w))<=3&&std::min(std::abs(p/m.w-b.head/m.w),m.h-std::abs(p/m.w-b.head/m.w))<=3)) {
  auto& c=b.cells[p];c.seen=0;c.spawn=m.hi[p]?m.hi[p]:-1;
  for(int d=0;d<4;d++){int e=m.edges[b.key(p,d)];b.set_edge(p,d,e>=2?2:e,e>=2?e-2:-1);}
 }
 for(int j=0;j<int(m.starts.size());j++) {auto s=m.starts[j];for(int k=0;k<s.length;k++) {
  int p=m.body[s.offset+k];if(b.cells[p].seen<0)continue;
  b.cells[p].id=j;b.cells[p].team=s.team^flip;b.cells[p].head=k==0;
 }}
 b.transitions();return b;
}
int main(){
 int starts=0,confirmed=0;
 for(int code=1;code<=10;code++)for(int i=0;i<int(atlas::catalog[code-1]->starts.size());i++)for(bool flip:{false,true}) {
  auto b=scene(code,i,flip);b.map_knowledge.update(b);starts++;
  assert(b.map_knowledge.candidate==code);assert(b.map_knowledge.mode==0||b.map_knowledge.mode==code);confirmed+=b.map_knowledge.mode==code;
  // Every starting signature is checked with both side assignments.
  auto c=scene(code,i,flip);c.set_edge(c.head,0,c.cells[c.head].kind[0]==0?1:0);c.map_knowledge.update(c);assert(c.map_knowledge.mode==15);
  c=scene(code,i,flip);c.cells[c.head].spawn=c.cells[c.head].spawn<0?0:-1;c.map_knowledge.update(c);assert(c.map_knowledge.mode==15);
 }
 // All map geometries must eventually confirm with sufficient observations.
 for(int code=1;code<=10;code++) {
  auto b=scene(code);auto const& m=*atlas::catalog[code-1];
  for(int p=0;p<b.n;p++){b.cells[p].seen=0;b.cells[p].spawn=m.hi[p]?m.hi[p]:-1;
   for(int d=0;d<4;d++){int e=m.edges[b.key(p,d)];b.set_edge(p,d,e>=2?2:e,e>=2?e-2:-1);}}
  // Only initial own body needs exact occupancy for catalogue validation.
  for(int p:b.predicted)b.cells[p].id=b.id;
  b.map_knowledge.update(b);assert(b.map_knowledge.mode==code);
  b.map_knowledge.receive(15,atlas::revision,false);b.map_knowledge.receive(code,atlas::revision,true);assert(b.map_knowledge.mode==15);
 }
 {auto b=scene(8);b.map_knowledge.update(b);assert(b.map_knowledge.mode==8);
  auto before=b.next;b.navigation();assert(b.next==before);
  int unseen=-1;for(int p=0;p<b.n;p++)if(b.cells[p].seen<0){unseen=p;break;}assert(unseen>=0&&b.cells[unseen].seen<0);
  b.cells[unseen].seen=1;b.round=1;b.cells[unseen].spawn=-1;
  b.set_edge(unseen,0,atlas::catalog[7]->edges[2*unseen]==0?1:0);b.map_knowledge.update(b);b.navigation();assert(b.map_knowledge.mode==15&&b.next==before);}
 {auto p=scene(8);p.map_knowledge.update(p);p.round=31;p.rescue_history=3;
  Decision d;d.split=6;d.protect_child_length=6;
  auto child=scene(8);child.id=100;child.length=6;child.round=child.birth_round=31;
  child.messages({p.birth_packet(d)});child.map_knowledge.update(child);
  assert(child.map_knowledge.mode==8&&child.rescued_grower&&child.rescue_history==3);
  p.map_knowledge.reject();child=scene(8);child.id=101;child.length=6;child.round=child.birth_round=31;
  child.messages({p.birth_packet(d)});child.map_knowledge.update(child);assert(child.map_knowledge.mode==15);
  auto no_message=scene(8);no_message.id=102;no_message.birth_round=no_message.round=31;
  no_message.map_knowledge.update(no_message);assert(no_message.map_knowledge.mode==0&&no_message.map_knowledge.candidate==0);
  // It can later receive a validated map report; location alone never guesses.
  no_message.map_knowledge.receive(8,atlas::revision,false);no_message.map_knowledge.update(no_message);assert(no_message.map_knowledge.mode==8);
 }
 {auto b=scene(8);b.map_knowledge.receive(8,atlas::revision+1,true);assert(!b.map_knowledge.educated);b.heading^=2;b.map_knowledge.update(b);assert(b.map_knowledge.mode==15);}
 {auto b=scene(8);b.map_knowledge.update(b);int p=-1;auto m=b.map_knowledge.map();
  for(int i=0;i<b.n;i++)if(b.cells[i].seen>=0&&m->hi[i]){p=i;break;}assert(p>=0);
  b.cells[p].spawn=m->hi[p]+1;b.map_knowledge.update(b);assert(b.map_knowledge.mode==15);}
 {auto b=scene(8);b.map_knowledge.update(b);bool changed=false;
  for(int p=0;p<b.n&&!changed;p++)for(int d=0;d<4;d++)if(b.cells[p].kind[d]==2){b.set_edge(p,d,2,b.cells[p].portal[d]+1);changed=true;break;}
  if(changed){b.map_knowledge.update(b);assert(b.map_knowledge.mode==15);}}
 {auto b=scene(8);b.map_knowledge.update(b);b.round=31;b.birth_round=30;b.length=6;
  Decision d;d.split=6;d.protect_child_length=6;auto msg=b.birth_packet(d);b.messages({msg});assert(!b.rescued_grower);}
 std::cout<<starts<<" starting signatures; "<<confirmed<<" confirm in starting view; mismatch, handoff and isolation checks passed\n";
}
