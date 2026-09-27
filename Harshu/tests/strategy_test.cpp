#include "../bot/strategy.hpp"
#include <cassert>
#include <iostream>
using namespace abyss;
Brain open_board() {
    Brain b(11,11,0,0); b.round=7;b.complete=true;b.length=3;b.head=60;b.predicted={60,59,58};
    for(int p=0;p<b.n;p++) {b.cells[p].seen=b.round;for(int d=0;d<4;d++)b.set_edge(p,d,0);}
    b.transitions();return b;
}
int main() {
    { auto b=open_board();assert(b.next[0][0]==110);assert(b.next[10][1]==0); }
    { auto b=open_board();b.set_edge(60,0,1);b.transitions();State t;int hit;assert(!b.step(b.initial(),0,false,t,hit)); }
    { auto b=open_board();State t;int hit;assert(!b.step(b.initial(),3,false,t,hit)); }
    { auto b=open_board();b.predicted={60,59,48,49};b.length=4;State t;int hit;assert(!b.step(b.initial(),0,false,t,hit)); }
    { auto b=open_board();State t;int hit;assert(b.step(b.initial(),1,false,t,hit));assert((t.body==std::vector<int>{61,60,59})); }
    { auto b=open_board();b.cells[61].pearl=true;b.cells[61].pearl_round=7;State t;int hit;assert(b.step(b.initial(),1,false,t,hit));assert(t.body.size()==4&&t.body.back()==58); }
    { auto b=open_board();State t,u;int hit;assert(b.step(b.initial(),1,false,t,hit));assert(b.step(t,1,true,u,hit));assert(u.body.size()==2);assert(!b.step(u,1,true,t,hit)); }
    { auto b=open_board();b.predicted={60,59};b.length=2;b.cells[62].pearl=true;b.cells[62].pearl_round=7;State t,u;int hit;assert(b.step(b.initial(),1,false,t,hit));assert(!b.step(t,1,true,u,hit));b.cells[61].pearl=true;b.cells[61].pearl_round=7;assert(b.step(b.initial(),1,false,t,hit));assert(b.step(t,1,true,u,hit));assert(u.body.size()==3); }
    { auto b=open_board();b.cells[61].id=1;b.cells[61].team=1;b.cells[61].head=true;State t;int hit;assert(!b.step(b.initial(),1,false,t,hit)&&hit==1);b.cells[61].team=0;assert(!b.step(b.initial(),1,false,t,hit)&&hit==-1); }
    { auto b=open_board();b.set_edge(60,1,2,17);b.set_edge(90,3,2,17);b.transitions();assert(b.next[60][1]==90);assert(b.next[90][3]==60);assert(b.next[61][3]==89);assert(b.next[89][1]==61); }
    { auto b=open_board();b.set_edge(60,0,2,17);b.transitions();State t;int hit;assert(!b.step(b.initial(),0,false,t,hit)); }
    { auto b=open_board();auto packet=b.packet(60,9);Brain ally(11,11,2,0);ally.round=7;ally.messages({packet});assert(ally.allies.size()==1&&ally.allies[0].len==9);ally.messages({packet^16});assert(ally.allies.size()==1);ally.round=10;ally.messages({packet});assert(ally.allies.empty()); }
    { auto b=open_board();b.length=3;b.count=1;assert(!b.safe_split(b.initial(),true)); }
    { auto b=open_board();b.predicted={60,59,48,49};b.length=4;State t,u;int hit;assert(b.step(b.initial(),1,false,t,hit));assert(b.step(t,0,true,u,hit));assert(u.body.size()==3); }
    std::cout<<"13 strategy scenarios passed\n";
}
