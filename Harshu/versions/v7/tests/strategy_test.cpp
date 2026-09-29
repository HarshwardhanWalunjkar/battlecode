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
Brain donation_scene() {
    auto b=open_board(420);b.id=2;b.count=8;b.length=2;b.predicted={60,59};
    for(int p:b.predicted){b.cells[p].id=2;b.cells[p].team=0;}
    std::vector<int> body{61,62,63,64,75,74,73,72};
    for(size_t k=0;k<body.size();k++) {
        auto& c=b.cells[body[k]];c.id=10;c.team=0;c.head=k==0;
        if(k)for(int d=0;d<4;d++)if(b.next[body[k]][d]==body[k-1])c.facing=d;
    }
    b.allies={{10,61,8,420,419,true}};return b;
}
void v7_checks();
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
    { auto b=open_board(205);b.count=8;b.length=4;b.predicted={60,59,58,57};
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
    { auto b=open_board();b.id=2;b.count=4;assert(b.valuable_trade(6));
      assert(!b.valuable_trade(3));b.count=1;assert(!b.valuable_trade(100));
      b.count=8;b.id=0;b.round=205;assert(!b.valuable_trade(100)); }
    // An expendable small dragon can attack a verified larger head despite safe moves.
    { auto b=open_board();b.id=2;b.count=4;b.enemy_size[1]=6;
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
      b.length=7;assert(b.grower());b.length=6;b.round=200;assert(b.grower()); }
    // No birth past the stopping round, and no emergency birth at the true cap.
    { auto b=open_board(225);b.id=2;b.length=4;b.predicted={60,59,58,57};
      assert(!b.safe_split(b.initial(),false));b.count=64;assert(b.rescue_split(b.initial())==0); }
    // The final-round rear needs to survive this round, not invented extra turns.
    { auto b=open_board(499);b.length=4;b.predicted={60,59,58,57};
      for(int p:{56,57})for(int d:{0,2})b.set_edge(p,d,1);
      b.set_edge(56,3,1);b.transitions();assert(b.rescue_split(b.initial())==2); }
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
    v7_checks();
    std::cout<<"V7 strategy checks passed (41 retained + 31 focused scenarios)\n";
}

void v7_checks() {
    // Ordinary lengths 2 and 3 must remain status reports, not control messages.
    { auto b=open_board(400);b.cells[30].seen=399;b.cells[40].seen=399;
      b.messages({b.packet(30,2,10),b.packet(40,3,11)});
      assert(b.allies.size()==2&&b.station_reports.empty()&&b.delivery_offers.empty()); }
    // Coordination tags are disjoint, and ACKs carry both participants' squares.
    { auto b=open_board(450);b.id=2;
      b.messages({b.tagged_packet(40,8,Brain::STATION_TAG,10),b.tagged_packet(60,2,Brain::OFFER_TAG,3),b.accept_packet(60,61,2)});
      assert(b.station_reports.size()==1&&b.station_reports[0].peer==8);
      assert(b.delivery_offers.size()==1&&b.delivery_accepts.size()==1);
      assert(b.delivery_accepts[0].pos==60&&b.delivery_accepts[0].peer==61&&b.allies.empty()); }
    // A donor never dies merely because a larger ally is adjacent.
    { auto b=donation_scene();assert(b.agreed_donation(b.initial())==-1); }
    // Higher-ID receiver: acknowledgement from last round, pickup this round.
    { auto b=donation_scene();b.delivery_accepts={{60,2,419,61}};
      assert(b.agreed_donation(b.initial())==3);
      b.delivery_accepts[0].pos=49;assert(b.agreed_donation(b.initial())==-1); }
    // Lower-ID receiver has acted: fresh agreement schedules next-round pickup.
    { auto b=donation_scene();for(auto& c:b.cells)if(c.id==10)c.id=1;b.allies[0].id=1;
      b.delivery_accepts={{60,2,420,61}};assert(b.agreed_donation(b.initial())==3);
      b.delivery_accepts[0].round=419;assert(b.agreed_donation(b.initial())==-1); }
    // Population and enemy checks are revalidated immediately before death.
    { auto b=donation_scene();b.delivery_accepts={{60,2,420,61}};b.count=3;
      assert(b.agreed_donation(b.initial())==-1);b.count=8;b.threat_distance[61]=3;
      assert(b.agreed_donation(b.initial())==-1); }
    // A last-round donation cannot feed a collector that has already acted.
    { auto b=donation_scene();b.round=499;for(auto& c:b.cells)c.seen=499;
      for(auto& c:b.cells)if(c.id==10)c.id=1;
      b.allies[0].id=1;b.allies[0].round=499;
      b.delivery_accepts={{60,2,499,61}};assert(b.agreed_donation(b.initial())==-1); }
    // Receiver has an explicit collection priority after agreed food appears.
    { auto b=open_board(450);b.length=6;b.predicted={60,59,58,57,56,55};
      b.pickup=61;b.pickup_until=450;b.cells[61].pearl=true;b.cells[61].pearl_round=450;
      State meal;int hit;assert(b.step(b.initial(),1,false,meal,hit));
      Candidate c;c.state=meal;c.path={1};c.exits=2;c.threat=99;c.score=-100;
      Decision chosen;assert(b.collect_promised_food({c},chosen)&&chosen.path==std::vector<int>{1}&&b.coord_event==2); }
    // No phantom collection or walking into a donor whose agreement was lost.
    { auto b=open_board(450);b.pickup=61;b.pickup_until=450;b.cells[61].id=5;
      Decision chosen;assert(!b.collect_promised_food({},chosen)); }
    // ACK reservation suppresses a third worker's incentive to steal the meal.
    { auto b=donation_scene();b.id=3;b.cells[49].pearl=true;b.cells[49].pearl_round=420;
      b.delivery_accepts={{49,2,420,61}};b.prepare_goals();assert(b.food_share[49]==.1); }
    // Refraction: the reverse ray starts from the post-action tail, not the head.
    { auto b=donation_scene();auto state=b.initial();state.heading=1;
      b.cells[58].id=12;b.cells[58].team=0;
      assert(b.sonar_recipient(state,1)==10);assert(b.sonar_recipient(state,3)==12); }
    // Enemy length-two without food cannot finance a second sprint step.
    { auto b=open_board();b.cells[62].id=1;b.cells[62].head=true;b.cells[62].team=1;
      b.cells[63].id=1;b.cells[63].team=1;b.cells[63].facing=3;
      b.enemy_heads={62};b.threat_sources={62};b.identify_enemies();
      assert(b.exact_enemy_length.at(1)==2&&b.attack_distance(b.initial())==99);
      b.cells[61].pearl=true;b.cells[61].pearl_round=b.round;
      assert(b.attack_distance(b.initial())==2); }
    // A partial enemy tail never supplies an invented exact sprint capacity.
    { auto b=open_board();b.cells[62].id=1;b.cells[62].head=true;b.cells[62].team=1;
      b.cells[63].id=1;b.cells[63].team=1;b.cells[63].facing=3;b.cells[64].seen=6;
      b.enemy_heads={62};b.identify_enemies();assert(b.exact_enemy_length.empty()); }
    // A low-cost worker may hold ground against an expensive head, not a cheap tail.
    { auto b=open_board();b.id=2;b.count=6;b.length=2;b.predicted={60,59};
      b.cells[61].id=10;b.cells[61].head=true;b.enemy_size[10]=8;b.threat_sources={61};
      assert(b.favorable_standoff(b.initial()));b.cells[61].head=false;
      assert(!b.favorable_standoff(b.initial())); }
    // Equal-distance territory is contested; starting positions are not tie-breakers.
    { auto b=open_board();b.enemy_heads={62};b.cells[62].id=1;b.cells[62].head=true;b.cells[62].team=1;
      b.prepare_territory();auto [ours,ties]=b.territory(b.initial());assert(ours>0&&ties>0); }
    // Known portals are graph edges with cost one, including territory arrivals.
    { auto b=open_board();b.set_edge(60,1,2,17);b.set_edge(90,3,2,17);b.transitions();
      b.enemy_heads={60};b.prepare_territory();assert(b.enemy_arrival[90]==1); }
    // The flood cap does not prove that a big connected region is enclosed.
    { auto b=open_board();auto [area,frontier]=b.space(b.initial(),5);assert(area>=5&&frontier>0); }
    // Food urgency is bounded and never reduces a long scorer's food reward.
    { auto b=open_board(20);b.length=2;b.last_food=0;assert(b.food_value()>12&&b.food_value()<=16.2);
      b.length=12;assert(b.food_value()==12); }
    // Newly observed map structure changes confidence, not collision legality.
    { auto b=open_board();b.profile_terrain();assert(b.wall_fraction==0&&b.terrain_confidence==1);
      b.set_edge(60,1,1);b.transitions();b.profile_terrain();State t;int hit;
      assert(b.wall_fraction>0&&!b.step(b.initial(),1,false,t,hit)); }
    // Repeated partial rescue changes early expansion; late score is protected.
    { auto b=open_board(10);b.length=13;b.complete=false;b.rescue_history=1;
      for(int d:{0,1,2})b.set_edge(60,d,1);
      b.transitions();auto d=b.choose();assert(d.split==6&&d.release_child);
      b.round=450;b.started=std::chrono::steady_clock::now();assert(b.choose().split==11); }
    // A role packet carries lineage without protecting a deliberately released worker.
    { auto b=open_board(10);b.birth_round=10;b.length=6;
      b.messages({b.packet(60,1,6|(2<<12)|32768)});assert(b.rescue_history==2&&!b.rescued_grower); }
    // Duplicate attraction cannot reward postponing an immediately free pearl.
    { auto b=open_board(420);b.length=8;b.predicted={60,59,58,57,56,55,66,67};
      b.cells[61].pearl=true;b.cells[61].pearl_round=420;b.prepare_goals();
      auto d=b.choose();assert(d.path==std::vector<int>{1}&&d.after.length==9); }
    // A collector agrees only after an actual safe approach and reachable sonar.
    { auto b=open_board(450);b.id=10;b.count=8;b.length=8;b.head=50;b.heading=2;
      b.predicted={50,39,28,17,6,5,4,3};b.station=50;
      b.cells[60].id=2;b.cells[60].team=0;b.cells[60].head=true;
      b.cells[59].id=2;b.cells[59].team=0;b.cells[59].facing=1;
      b.delivery_offers={{60,2,450,2}};
      State near;int hit;assert(b.step(b.initial(),3,false,near,hit));
      Candidate c;c.state=near;c.path={3};c.exits=2;c.threat=99;c.score=0;
      Decision out;assert(b.accept_collection({c},out)&&b.outgoing_accept==2&&b.pickup==60&&out.after.body[0]==49);
      b.cells[49].kind[2]=1;b.transitions();b.started=std::chrono::steady_clock::now();
      assert(!b.accept_collection({c},out)); }
    // Control packets reject stale, future, wrong-team and corrupt messages;
    // four copies of an acknowledgement count as one agreement.
    { auto b=open_board(450);auto ack=b.accept_packet(60,61,2);
      b.messages({ack,ack,ack,ack});assert(b.delivery_accepts.size()==1);
      auto other=b;other.team=1;auto future=b;future.round=451;auto stale=b;stale.round=448;
      b.delivery_accepts.clear();
      b.messages({ack^16,other.accept_packet(60,61,2),future.accept_packet(60,61,2),stale.accept_packet(60,61,2)});
      assert(b.delivery_accepts.empty()); }
    // A late collector's several sonar rays cannot masquerade as one portal probe.
    { auto b=open_board(450);b.id=2;b.length=8;b.count=8;b.predicted={60,59,58,57,56,55,66,67};b.station=60;
      b.set_edge(61,0,2,17);b.transitions();State moved;int hit;
      assert(b.step(b.initial(),1,false,moved,hit));
      Decision d{{1},0,moved,true,false};
      unswbc::Controller ct(2,unswbc::Team('A'),unswbc::Direction('E'),unswbc::Vision{},64);
      std::ostringstream output;auto old=std::cout.rdbuf(output.rdbuf());b.commit(d,ct);std::cout.rdbuf(old);
      assert(b.probe_round!=450&&output.str().find("SONAR")!=std::string::npos); }
    // Depleted, food-producing contested fronts can replace workers after 225.
    // Quiet farms, crowded teams and the consolidation phase cannot.
    { auto b=open_board(250);b.id=2;b.count=8;b.length=4;b.predicted={60,59,58,57};
      b.enemy_heads={90};b.goals={{61,1,std::vector<int>(b.n,2)},{49,1,std::vector<int>(b.n,3)}};
      assert(b.safe_split(b.initial(),false));b.enemy_heads.clear();assert(!b.safe_split(b.initial(),false));
      b.enemy_heads={90};b.count=24;assert(!b.safe_split(b.initial(),false));
      b.count=8;b.round=325;assert(!b.safe_split(b.initial(),false)); }
    // Protect a large ally only when a visible enemy actually has a head attack.
    { auto b=open_board();b.id=2;b.count=4;b.length=2;b.enemy_heads={61};
      b.cells[61].id=10;b.cells[61].head=true;b.cells[62].id=12;b.cells[62].team=0;b.cells[62].head=true;
      b.allies={{12,62,8,b.round}};assert(b.defensive_intercept(10));
      b.set_edge(61,1,1);b.transitions();assert(!b.defensive_intercept(10)); }
    // Opposite checkerboard phases need two ordinary turns, not a costly sprint.
    { auto b=open_board(450);b.cells[62].pearl=true;b.cells[62].pearl_round=450;
      State meal;int steps=0;assert(!b.pickup_route(b.initial(),62,meal,steps,1));
      assert(b.pickup_route(b.initial(),62,meal,steps)&&steps==2&&meal.length==4);
      State near;int hit;assert(b.step(b.initial(),1,false,near,hit));
      Candidate c;c.state=near;c.path={1};c.threat=99;c.exits=2;
      b.pickup=62;b.pickup_donor=7;b.pickup_until=451;Decision out;
      assert(b.collect_promised_food({c},out)&&b.coord_event==4&&b.pickup==62&&b.outgoing_accept==7);
      b.round=499;b.pickup_until=499;assert(!b.collect_promised_food({c},out)); }
    // Two-turn feeding is rejected when an intruder blocks the approach or the
    // final round arrives before the second collection move.
    { auto b=donation_scene();b.remove_donor(10);
      std::vector<int> body{62,63,64,75,74,73,72,71};
      for(size_t k=0;k<body.size();k++) {
          auto& c=b.cells[body[k]];c.id=10;c.team=0;c.head=k==0;
          if(k)for(int d=0;d<4;d++)if(b.next[body[k]][d]==body[k-1])c.facing=d;
      }
      b.allies={{10,62,8,420,420,true}};b.delivery_accepts={{60,2,420,62}};
      assert(b.agreed_donation(b.initial())==3);b.cells[61].id=12;
      assert(b.agreed_donation(b.initial())==-1);b.cells[61].id=-1;
      b.round=499;for(auto& c:b.cells)c.seen=499;b.delivery_accepts={{60,2,499,62}};
      assert(b.agreed_donation(b.initial())==-1); }
    // Seeing fewer segments does not establish that a previously reported large
    // collector shrank. A fresh exact shorter report does cancel the assignment.
    { auto b=donation_scene();b.round=450;for(auto& c:b.cells)c.seen=450;
      b.station_reports={{61,10,450,8}};b.allies={{10,61,1,450,449,false}};
      b.prepare_coordination();assert(b.leader==10);
      b.allies[0].exact=true;b.prepare_coordination();assert(b.leader==-1); }
    // A length-two enemy cannot sprint into us, but its legal single move can
    // close our sole exit. The reply check catches this distinct choke threat.
    { auto b=open_board(450);b.cells[62].id=1;b.cells[62].team=1;b.cells[62].head=true;
      b.cells[63].id=1;b.cells[63].team=1;b.cells[63].facing=3;
      b.enemy_heads={62};b.threat_sources={62};b.identify_enemies();
      b.set_edge(60,0,1);b.set_edge(60,2,1);b.transitions();
      Candidate c;c.state=b.initial();assert(b.attack_distance(c.state)==99);
      assert(b.tactical_reply_penalty(c)==45);
      b.cells[61].pearl=true;b.cells[61].pearl_round=450;
      assert(b.tactical_reply_penalty(c)==80); }
}
