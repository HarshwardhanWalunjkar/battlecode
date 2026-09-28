#include "../bot/strategy.hpp"
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
using namespace abyss;
Brain open_board(int round=7) {
    Brain b(11,11,0,0); b.round=round;b.complete=true;b.length=3;b.head=60;b.predicted={60,59,58};
    for(int p=0;p<b.n;p++) {b.cells[p].seen=b.round;for(int d=0;d<4;d++)b.set_edge(p,d,0);}
    b.transitions();return b;
}
Brain slithery_coil() {
    Brain b(63,27,4,0);b.round=0;b.count=7;b.length=25;b.complete=true;
    b.head=22*63+3;b.heading=0;
    auto visible=[&](int p){int x=std::abs(p%63-3),y=std::abs(p/63-22);return std::min(x,63-x)<=3&&std::min(y,27-y)<=3;};
    std::ifstream file("Harshu/maps/slithery_fight.map");assert(file.good());
    std::string line;int dragon=0;
    while(std::getline(file,line)) {
        std::istringstream in(line);std::string kind;in>>kind;
        if(kind=="EDGE") {
            int index,type,portal;in>>index>>type>>portal;
            int row=index/64,x=index%64,p=(row/2%27)*63+x%63,d=row%2?3:0;
            if(visible(p)||visible(b.adjacent[p][d]))b.set_edge(p,d,type,portal);
        } else if(kind=="DRAGON") {
            int side,len,x,y;in>>side>>len;
            for(int k=0;k<len;k++) {in>>x>>y;int p=y*63+x;
                if(dragon==4)b.predicted.push_back(p);
                if(visible(p)){b.cells[p].id=dragon;b.cells[p].team=side;b.cells[p].head=k==0;}
            }
            dragon++;
        }
    }
    b.transitions();
    for(int p=0;p<b.n;p++)if(visible(p))b.cells[p].seen=0;
    for(size_t k=1;k<b.predicted.size();k++)for(int d=0;d<4;d++)
        if(b.next[b.predicted[k]][d]==b.predicted[k-1])b.cells[b.predicted[k]].facing=d;
    return b;
}
Brain donation_scene(int r=420) {
    auto b=open_board(r);b.id=2;b.count=8;b.length=2;b.predicted={60,59};
    for(int p:b.predicted){b.cells[p].id=2;b.cells[p].team=0;}
    std::vector<int> body{61,62,63,64,75,74,73,72};
    for(size_t k=0;k<body.size();k++) {
        auto& c=b.cells[body[k]];c.id=10;c.team=0;c.head=k==0;
        if(k)for(int d=0;d<4;d++)if(b.next[body[k]][d]==body[k-1])c.facing=d;
    }
    b.allies={{10,61,8,r,r-1,true}};return b;
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
    // Expansion can use the final slot, but the engine cap is still binding.
    { auto b=open_board();b.id=2;b.count=63;b.length=4;b.predicted={60,59,58,57};
      assert(b.safe_split(b.initial(),false));b.count=64;assert(!b.safe_split(b.initial(),true)); }
    // Growers retain length while scouts may reproduce in the same open position.
    { auto b=open_board(255);b.count=8;b.length=4;b.predicted={60,59,58,57};
      assert(b.grower());assert(!b.safe_split(b.initial(),false));
      b.id=2;assert(!b.grower());assert(b.safe_split(b.initial(),false));
      b.length=6;b.predicted={60,59,58,57,56,55};assert(b.grower()); }
    // The end of a visible prefix is not the tail; no child-safety claim is valid.
    { auto b=open_board();b.complete=false;b.length=8;b.predicted={60,59,58,57};
      assert(!b.safe_split(b.initial(),true)); }
    // One available child move into a two-cell dead end is insufficient.
    { auto b=open_board();b.length=4;b.predicted={60,59,58,57};
      for(int p:{56,57})for(int d:{0,2})b.set_edge(p,d,1);
      b.set_edge(56,3,1);b.transitions();assert(!b.safe_split(b.initial(),true)); }
    // Length advantage alone cannot justify sacrificing the last dragon or a grower.
    { auto b=open_board();b.id=2;b.count=3;assert(b.valuable_trade(6));assert(!b.valuable_trade(4));
      assert(!b.valuable_trade(3));b.count=1;assert(!b.valuable_trade(100));
      b.count=8;b.id=0;b.round=255;assert(!b.valuable_trade(100)); }
    // An expendable small dragon can attack a verified larger head despite safe moves.
    { auto b=open_board();b.id=2;b.count=3;b.enemy_size[1]=6;
      b.cells[61].id=1;b.cells[61].team=1;b.cells[61].head=true;
      auto d=b.choose();assert(d.dying&&d.path==std::vector<int>{1}); }
    // Exploration has a persistent destination and rewards actual progress toward it.
    { auto b=open_board();b.round=40;b.id=2;b.last_food=0;
      std::fill(b.last_visit.begin(),b.last_visit.end(),40);
      b.set_edge(64,1,-1);b.transitions();b.prepare_exploration();
      assert(b.explore_target==64);auto s=b.initial();s.body[0]=61;auto away=s;away.body[0]=49;
      assert(b.attraction(s)>b.attraction(away));b.round++;
      b.prepare_exploration();assert(b.explore_target==64); }
    // A promising nearby food supply delays exploration instead of abandoning a farm.
    { auto b=open_board();b.id=2;b.round=10;b.last_food=0;
      Goal g{61,1,std::vector<int>(b.n,2)};b.goals={g};b.prepare_exploration();
      assert(b.explore_distance.empty()); }
    // A blind move retains the far-side neck, with and without eating on arrival.
    { auto b=open_board();b.pending_portal=true;b.pending_portal_length=3;b.head=90;
      b.recover_portal_body();assert((b.predicted==std::vector<int>{90,60,59}));
      b.set_edge(60,1,2,17);b.set_edge(90,3,2,17);b.transitions();
      State t;int hit;assert(!b.step(b.initial(),3,false,t,hit)); }
    { auto b=open_board();b.pending_portal=true;b.pending_portal_length=3;b.head=90;b.length=4;
      b.recover_portal_body();assert((b.predicted==std::vector<int>{90,60,59,58}));assert(b.last_food==6); }
    // An unseen neck remains a forbidden reversal even for a newborn.
    { auto b=open_board();b.heading=1;b.predicted={60};b.complete=false;
      b.set_edge(60,3,2,17);b.transitions();assert(b.exploratory_portal(b.initial())==-1); }
    // Portal warnings expire and never become a report of a real dragon.
    { auto b=open_board();b.set_edge(60,1,2,17);b.transitions();
      b.messages({b.packet(60,0,17)});assert(b.allies.empty());assert(b.exploratory_portal(b.initial())==-1);
      b.round+=2;b.messages({});assert(b.exploratory_portal(b.initial())==1); }
    // Recent ally positions veto blind arrival beside another head.
    { auto b=open_board();b.set_edge(60,1,2,17);b.set_edge(90,3,2,17);b.transitions();
      b.cells[90].seen=6;b.allies={{2,91,10,7}};assert(b.exploratory_portal(b.initial())==-1); }
    // Portals replay: a nearby ally report must not force a wall collision when
    // the only possible escape is this unseen exit. It remains a blind risk.
    { auto b=open_board();b.heading=1;b.set_edge(60,1,2,17);b.set_edge(90,3,2,17);
      b.set_edge(60,0,1);b.set_edge(60,2,1);b.transitions();
      b.cells[90].seen=6;b.allies={{2,91,10,7}};
      assert(b.exploratory_portal(b.initial())==-1);
      auto d=b.choose();assert(!d.dying&&d.path==std::vector<int>{1}&&d.after.body.empty()); }
    // Desperation does not permit an exact reported ally head or traffic warning.
    { auto b=open_board();b.heading=1;b.set_edge(60,1,2,17);b.set_edge(90,3,2,17);b.transitions();
      b.cells[90].seen=6;b.allies={{2,90,10,7}};
      assert(b.exploratory_portal(b.initial(),true)==-1);
      b.allies={{2,91,10,7}};b.portal_busy_until[17]=7;
      assert(b.exploratory_portal(b.initial(),true)==-1); }
    // Having a safe ordinary move keeps the adjacent-head veto in force.
    { auto b=open_board(20);b.heading=1;b.last_food=0;
      b.set_edge(60,1,2,17);b.set_edge(90,3,2,17);b.transitions();
      b.cells[90].seen=19;b.allies={{2,91,10,20}};
      auto d=b.choose();assert(d.path!=std::vector<int>{1}&&!d.after.body.empty()); }
    // The actual opening coil can preserve 23 segments instead of peeling two.
    { auto b=slithery_coil();auto d=b.choose();assert(d.split==23&&d.protect_child_length==23); }
    // A rescue newborn keeps its role even below the normal protection threshold.
    { auto b=open_board(0);b.id=20;b.length=6;b.birth_round=0;
      assert(!b.grower());b.messages({b.packet(60,1,6)});assert(b.grower());
      b.rescued_grower=false;b.messages({b.packet(60,1,5)});assert(!b.grower()); }
    // Partial initial bodies with a usable route do not automatically reverse/split.
    { auto b=open_board(0);b.length=12;b.complete=false;auto d=b.choose();assert(d.split==0); }
    // Short early breeders remain available; a long starting dragon is protected.
    { auto b=open_board(0);b.count=1;b.length=6;assert(!b.grower());
      b.length=7;assert(b.grower());b.length=6;b.round=250;assert(b.grower()); }
    // No birth past the stopping round, and no emergency birth at the true cap.
    { auto b=open_board(300);b.id=2;b.length=4;b.predicted={60,59,58,57};
      assert(!b.safe_split(b.initial(),false));b.count=64;assert(b.rescue_split(b.initial())==0); }
    // The final-round rear needs to survive this round, not invented extra turns.
    { auto b=open_board(499);b.length=4;b.predicted={60,59,58,57};
      for(int p:{56,57})for(int d:{0,2})b.set_edge(p,d,1);
      b.set_edge(56,3,1);b.transitions();assert(b.rescue_split(b.initial())==2); }
    // Late donations need a larger later-moving recipient and enough survivors.
    { auto b=donation_scene();assert(b.donation_move(b.initial())==3);
      b.count=7;assert(b.donation_move(b.initial())==-1); }
    { auto b=donation_scene();b.allies[0].id=1;assert(b.donation_move(b.initial())==-1); }
    { auto b=donation_scene();b.allies[0].exact=false;assert(b.donation_move(b.initial())==-1); }
    { auto b=donation_scene();b.threat_distance[b.head]=2;assert(b.donation_move(b.initial())==-1); }
    { auto b=donation_scene();b.cells[50].pearl=true;assert(b.donation_move(b.initial())==-1); }
    { auto b=donation_scene();b.cells[72].seen=419;assert(b.donation_move(b.initial())==3); }
    // Sharing stops after eight unclaimed rounds, and uses a clear terrain route.
    { auto b=donation_scene();b.length=2;b.cells[50].pearl=true;b.cells[50].pearl_round=420;b.cells[50].pearl_since=415;
      b.prepare_goals();assert(b.food_share[50]==.2);
      b.cells[50].pearl_since=410;b.prepare_goals();assert(b.food_share[50]==1.0); }
    // A trapped parent can legally wait by splitting on the final round.
    { auto b=open_board(499);b.length=7;b.predicted={60,59,58,57,56,55,66};
      for(int p:{60,66})for(int d=0;d<4;d++)b.set_edge(p,d,1);
      b.transitions();assert(b.rescue_split(b.initial())==2); }
    // A pocket's escape must recover the two-segment split cost and still gain.
    { auto b=slithery_coil();auto s=b.initial();b.length=22;
      Candidate c;c.state=s;b.lookahead(c);assert(!c.forced_dead&&c.depth>0); }
    { auto b=slithery_coil();auto s=b.initial();b.length=24;
      Candidate c;c.state=s;b.lookahead(c);assert(c.forced_dead); }
    // A not-yet-due pearl is less attractive than the same ready resource.
    { auto b=open_board();Goal g{61,1,std::vector<int>(b.n,1),-1};b.goals={g};
      double ready=b.attraction(b.initial());b.goals[0].due=b.round+5;
      assert(b.attraction(b.initial())<ready); }
    // Dilemma's repeated partial-body trap changes strategy without inventing a tail.
    { auto b=open_board(10);b.length=13;b.complete=false;b.rescue_chain=1;
      for(int d:{0,1,2}){b.set_edge(60,d,1);} b.transitions();
      auto decision=b.choose();assert(decision.split==6&&decision.release_child); }
    // An isolated partial-body rescue still preserves the greatest legal rear.
    { auto b=open_board(10);b.length=13;b.complete=false;
      for(int d:{0,1,2}){b.set_edge(60,d,1);} b.transitions();
      auto decision=b.choose();assert(decision.split==11&&!decision.release_child); }
    // Late-game rescues do not halve the scorer merely because of its history.
    { auto b=open_board(450);b.length=13;b.complete=false;b.rescue_chain=3;
      for(int d:{0,1,2}){b.set_edge(60,d,1);} b.transitions();
      assert(b.choose().split==11); }
    // Newborns receive the chain history and a released worker role, with size checks.
    { auto b=open_board(10);b.birth_round=10;b.length=6;
      b.messages({b.packet(60,1,6|(2<<12)|32768)});
      assert(b.rescue_chain==2&&!b.rescued_grower&&!b.grower()); }
    // Fully known repeated rescues consider a checked balanced escape too.
    { auto b=slithery_coil();b.rescue_chain=1;int cut=b.rescue_split(b.initial());
      assert(cut==12||cut==13); }
    // A two-step threat must not veto the only legal food-bearing escape.
    { auto b=open_board();b.cells[61].pearl=true;b.cells[61].pearl_round=7;
      b.cells[63].id=1;b.cells[63].team=1;b.cells[63].head=true;b.threat_sources={63};
      b.set_edge(60,0,1);b.set_edge(60,2,1);b.transitions();
      auto d=b.choose();assert(!d.dying&&!d.path.empty()&&d.path[0]==1); }
    // Immediate enemy capture remains expensive even for a hungry worker.
    { auto b=open_board();assert(b.danger(1)==130&&b.danger(2)==14);
      b.length=8;assert(b.danger(2)==36); }
    // A rich known patch creates a persistent route without assuming map center.
    { auto b=open_board();for(int p:{4,5,6}){b.cells[p].spawn=8;b.cells[p].spawn_interval=1;}
      b.prepare_farms();assert(b.farm_target>=0&&b.farm_distance[b.head]>0);
      int old=b.farm_target;b.prepare_farms();assert(b.farm_target==old); }
    // Unknown resource locations never manufacture a farm target.
    { auto b=open_board();b.prepare_farms();assert(b.farm_target==-1); }
    // Earlier population building does not reserve food for a distant bigger ally.
    { auto b=donation_scene(100);b.cells[50].pearl=true;b.cells[50].pearl_round=100;b.cells[50].pearl_since=100;
      b.prepare_goals();assert(b.food_share[50]==1); }
    // A remote larger scorer no longer vetoes building a useful local recipient.
    { auto b=donation_scene();b.allies.push_back({50,20,30,420,419,true});
      assert(b.donation_move(b.initial())==3); }
    // Late feeding keeps three survivors; the old eight-dragon floor was too rigid.
    { auto b=donation_scene(450);b.count=4;assert(b.donation_move(b.initial())==3);
      b.count=3;assert(b.donation_move(b.initial())==-1); }
    // A recipient with an unseen neck cannot be safely modeled for a donation.
    { auto b=donation_scene();b.cells[62].seen=419;assert(b.donation_move(b.initial())==-1); }
    // Do not double-count the promise of a pearl and reward postponing the meal.
    { auto b=open_board(420);b.length=8;b.predicted={60,59,58,57,56,55,66,67};
      b.cells[61].pearl=true;b.cells[61].pearl_round=420;b.prepare_goals();
      auto d=b.choose();assert(!d.path.empty()&&d.path[0]==1&&d.after.length>=9); }
    // A long unseen opening in a visibly fertile area can seed one rear worker.
    { auto b=open_board(0);b.birth_round=0;b.length=12;b.complete=false;
      for(int p:{49,50,51})b.cells[p].spawn=1;
      auto d=b.choose();assert(d.split==6&&d.release_child&&b.rescued_grower); }
    // The child handoff prevents a second automatic opening cut that same round.
    { auto b=open_board(0);b.birth_round=0;b.length=12;b.complete=false;b.rescue_chain=1;
      for(int p:{49,50,51})b.cells[p].spawn=1;
      assert(!b.opening_seed(b.initial())); }
    std::cout<<"63 strategy scenarios passed (logic checks, no matches)\n";
}
