#include "strategy.hpp"
int main(){try{auto [ct,g]=unswbc::init();abyss::Brain b(g.width,g.height,ct.get_id(),ct.get_team().value=='A'?0:1);
while(unswbc::update(ct,g)){b.observe(ct,g);auto s=b.initial();std::vector<abyss::Candidate> c,a;std::vector<int> p;b.enumerate(s,p,c,a);
abyss::Decision decision;double best=-1e20;
for(auto &v:c){double score=v.state.reward+b.attraction(v.state);auto [room,frontier]=b.space(v.state,100);score+=2*std::log(1+room);if(b.mobility(v.state)==0)score-=200;
score-=b.danger(b.attack_distance(v.state))*.4;
if(score>best){best=score;decision={v.path,0,v.state,b.complete,false};}}
if(decision.path.empty()&&!decision.split)decision={{b.heading},0,s,false,true};b.commit(decision,ct);unswbc::end_turn();}
}catch(std::exception const&){return std::cin.eof()?0:1;}}
