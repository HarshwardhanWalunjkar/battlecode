#include "../../versions/v7/bot/strategy.hpp"
#include <fstream>
#include <iostream>
using namespace abyss;
int main(int argc,char**argv) {
    std::ifstream f(argv[1]);int w,h,id,team,r,count,len,heading,lastfood;
    f>>w>>h>>id>>team>>r>>count>>len>>heading>>lastfood;
    Brain b(w,h,id,team);b.round=r;b.count=count;b.length=len;b.heading=heading;b.last_food=lastfood;b.complete=true;
    b.predicted.resize(len);for(auto&v:b.predicted)f>>v;b.head=b.predicted[0];
    int nn;f>>nn;while(nn--){int p;f>>p;auto&c=b.cells[p];f>>c.seen>>c.pearl>>c.pearl_round>>c.pearl_since>>c.spawn>>c.id>>c.team>>c.head>>c.facing;}
    f>>nn;while(nn--){int p,d,k,portal;f>>p>>d>>k>>portal;b.set_edge(p,d,k,portal);}
    f>>nn;while(nn--){int p,r;f>>p>>r;b.last_visit[p]=r;}
    b.transitions();
    for(int p=0;p<b.n;p++)if(b.cells[p].seen==r&&b.cells[p].id>=0&&b.cells[p].team!=team) {
        b.enemy_size[b.cells[p].id]++;if(b.cells[p].head)b.enemy_heads.push_back(p);
    }
    b.threat_sources=b.enemy_heads;b.identify_enemies();b.profile_terrain();b.prepare_territory();b.prepare_goals();b.prepare_exploration();
    b.station=b.head;b.pickup=139;b.pickup_donor=387;b.pickup_until=462;
    auto s=b.initial();std::vector<Candidate>all,attacks;std::vector<int>path;b.enumerate(s,path,all,attacks);
    for(auto&c:all)c.base=c.score=b.root_score(c);
    std::sort(all.begin(),all.end(),[](auto&a,auto&b){return a.base>b.base;});
    int rank=0;for(auto&c:all){
        b.started=std::chrono::steady_clock::now();double look=b.lookahead(c);
        std::cout<<"rank "<<rank++<<" path ";for(int d:c.path)std::cout<<LETTERS[d];
        std::cout<<" dest "<<c.state.body[0]<<" base "<<c.base<<" look "<<look<<" forced "<<c.forced_dead<<" depth "<<c.depth<<" threat "<<c.threat<<" exits "<<c.exits<<'\n';
    }
    b.started=std::chrono::steady_clock::now();auto d=b.choose();std::cout<<"chosen ";for(int x:d.path)std::cout<<LETTERS[x];std::cout<<" coord "<<b.coord_event<<'\n';
}
