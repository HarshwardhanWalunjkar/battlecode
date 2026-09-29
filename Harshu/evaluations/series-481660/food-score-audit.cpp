#include "../../versions/v4.1/bot/strategy.hpp"
#include <iostream>
int main(){using namespace abyss;Brain b(11,11,10,0);b.round=420;b.count=8;b.complete=true;b.length=8;b.head=61;b.heading=3;
 b.predicted={61,62,63,64,75,74,73,72};
 for(int p=0;p<b.n;p++){b.cells[p].seen=420;for(int d=0;d<4;d++)b.set_edge(p,d,0);}b.transitions();
 for(int p:b.predicted){b.cells[p].id=10;b.cells[p].team=0;}
 b.cells[60].pearl=true;b.cells[60].pearl_round=420;b.prepare_goals();
 auto s=b.initial();std::vector<Candidate> all,attacks;std::vector<int> path;b.enumerate(s,path,all,attacks);
 for(auto& c:all)c.base=c.score=b.root_score(c);
 std::sort(all.begin(),all.end(),[](auto&a,auto&b){return a.base>b.base;});if(all.size()>14)all.resize(14);
 double old=-1e9,fixed=-1e9;std::vector<int> oldpath,newpath;
 for(auto& c:all){c.score+=b.lookahead(c);double corrected=c.score-(c.depth>0?b.attraction(c.state):0);
 if(c.score>old){old=c.score;oldpath=c.path;}if(corrected>fixed){fixed=corrected;newpath=c.path;}}
 std::cout<<"Frozen v4.1, unchanged food/danger weights. Old scoring: ";for(int d:oldpath)std::cout<<LETTERS[d];std::cout<<" score "<<old;
 std::cout<<"; removing duplicate root attraction: ";for(int d:newpath)std::cout<<LETTERS[d];std::cout<<" score "<<fixed<<"\n";
}
