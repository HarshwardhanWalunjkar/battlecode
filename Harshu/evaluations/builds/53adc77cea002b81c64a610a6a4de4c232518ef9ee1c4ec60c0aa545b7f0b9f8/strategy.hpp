#pragma once
#include "helper.hpp"
#include "config.hpp"
#include "atlas.hpp"
#include <algorithm>
#include <bitset>
#include <chrono>
#include <cmath>
#include <limits>
#include <map>
#include <numeric>
#include <queue>
#include <set>

namespace abyss {
constexpr int MAX=4096, INF=100000;
constexpr char LETTERS[]="NESW";
inline int direction(char c) { for(int d=0;d<4;d++) if(LETTERS[d]==c) return d; return 0; }
using Bits=std::bitset<MAX>;
struct Cell {
    int seen=-10000, pearl_round=-10000, spawn=-1;
    int pearl_since=-1;
    bool pearl=false;
    int id=-1, team=-1, facing=0;
    bool head=false;
    std::array<int,4> kind{{-1,-1,-1,-1}}, portal{{-1,-1,-1,-1}};
};
struct Goal { int pos; double reward; std::vector<int> distance; int due=-1; };
struct Ally { int id,pos,len,round; int report_round=-10000; bool exact=false; };
struct Signal {int pos,id,round;int peer=-1;};
struct Intel {int pos,round,due;};
struct State {
    std::vector<int> body;
    Bits occupied, eaten;
    double reward=0;
    int heading=0;
    int length=0;
    int turns=0;
};
struct Candidate {
    State state;
    std::vector<int> path;
    int kill=-1;
    double base=-1e9, score=-1e9;
    int area=0, exits=0, threat=99, depth=0;
    bool forced_dead=false,standoff=false;
};
struct Decision {
    std::vector<int> path; int split=0; State after; bool exact=false, dying=false;
    int protect_child_length=0;
    bool release_child=false;
};

class Brain {
public:
    int w,h,n,id,team,round=1,length=3,count=1,limit=64,head=0,heading=0;
    std::vector<Cell> cells;
    std::vector<std::array<int,4>> next, adjacent;
    std::vector<std::vector<int>> reverse;
    std::vector<int> last_visit, threat_distance;
    std::vector<Goal> goals;
    std::map<int,std::vector<int>> portals;
    std::vector<Ally> allies;
    std::vector<double> food_share;
    std::map<int,int> portal_busy_until;
    std::map<int,int> enemy_size, exact_enemy_length;
    std::vector<Signal> station_reports,delivery_offers,delivery_accepts;
    int station=-1,station_until=-1,leader=-1,leader_site=-1;
    int pickup=-1,pickup_donor=-1,pickup_until=-1,outgoing_accept=-1,coord_event=0;
    std::vector<int> coord_distance,enemy_arrival;
    int leader_until=-1,last_enemy=-1,station_engaged_until=-1;
    bool quiet_front=false;
    // Shared terrain is a navigation hint, never movement/escape evidence.
    std::vector<std::array<int,4>> hints,nav_next;
    std::vector<std::vector<int>> nav_reverse;
    std::map<int,std::array<int,2>> portal_hints;
    std::map<int,int> intel_sent;
    std::vector<Intel> food_reports;
    atlas::Knowledge map_knowledge;
    int atlas_target=-1,atlas_until=-1;
    std::vector<int> atlas_distance;
    int rescue_history=0,seen_tiles=0;
    double wall_fraction=0,portal_fraction=0,terrain_confidence=0,regional_confidence=0,center_open_advantage=0;
    std::vector<int> enemy_heads, threat_sources;
    std::vector<int> predicted;
    bool complete=false;
    int last_split=-1000;
    int last_food=0, last_portal=-1000;
    int explore_target=-1, explore_until=-1;
    std::vector<int> explore_distance;
    bool first_observation=true;
    bool pending_portal=false;
    bool rescued_grower=false;
    int pending_portal_length=0, birth_round=-1;
    int probe_portal=-1, probe_round=-1000;
    std::chrono::steady_clock::time_point started;
    Brain(int width,int height,int dragon,int side):w(width),h(height),n(w*h),id(dragon),team(side),cells(n),next(n),adjacent(n),reverse(n),last_visit(n,-1000),threat_distance(n,99),food_share(n,1.0),started(std::chrono::steady_clock::now()) {
        hints.resize(n);nav_next.resize(n);nav_reverse.resize(n);
        for(auto& e:hints)e.fill(-1);
        for(int p=0;p<n;p++) {
            int x=p%w,y=p/w;
            adjacent[p]={{((y+h-1)%h)*w+x,y*w+(x+1)%w,((y+1)%h)*w+x,y*w+(x+w-1)%w}};
        }
    }
    int pos(unswbc::Position p) const { return p.y*w+p.x; }
    int distance(int a,int b) const { int x=std::abs(a%w-b%w),y=std::abs(a/w-b/w); return std::min(x,w-x)+std::min(y,h-y); }
    bool grower() const {
        return length>=7||(rescued_grower&&length>=4)||
            (round>=200&&(length>=6||(count>=4&&id%6==team)));
    }
    bool valuable_trade(int enemy_length) const {
        return count>=4 && length<=4 && !grower() && enemy_length>=std::max(6,length+2);
    }
    int key(int p,int d) const { if(d==1) return 2*adjacent[p][1]+1; if(d==2) return 2*adjacent[p][2]; return 2*p+(d==3); }
    void set_edge(int p,int d,int kind,int portal=-1) {
        int q=adjacent[p][d];
        cells[p].kind[d]=cells[q].kind[d^2]=kind;
        cells[p].portal[d]=cells[q].portal[d^2]=portal;
        if(kind==2) {
            auto& pair=portals[portal]; int k=key(p,d);
            if(std::find(pair.begin(),pair.end(),k)==pair.end()) pair.push_back(k);
        }
    }
    void transitions() {
        for(auto& r:reverse) r.clear();
        for(int p=0;p<n;p++) for(int d=0;d<4;d++) {
            int q=-1,kind=cells[p].kind[d];
            if(kind==0) q=adjacent[p][d];
            if(kind==2) {
                auto it=portals.find(cells[p].portal[d]);
                if(it!=portals.end() && it->second.size()==2) {
                    auto const& pair=it->second; int k=key(p,d), far=pair[0]==k?pair[1]:pair[0];
                    q=far/2;
                    if(d==0 || d==3) q=adjacent[q][d];
                }
            }
            next[p][d]=q;
            if(q>=0) reverse[q].push_back(p);
        }
        navigation();
    }
    static unsigned checksum(std::uint64_t payload) {
        payload ^= payload>>23; payload *= UINT64_C(0x9e3779b97f4a7c15); payload ^= payload>>31;
        return unsigned(payload & 16383);
    }
    std::uint64_t packet(int p,int len,int sender=-1) const {
        if(sender<0) sender=id;
        std::uint64_t v=std::uint64_t(p%w) | (std::uint64_t(p/w)<<6) | (std::uint64_t(std::min(len,4095))<<12)
            | (std::uint64_t(round)<<24) | (std::uint64_t(sender&65535)<<33) | (std::uint64_t(team)<<49);
        return v | (std::uint64_t(checksum(v))<<50);
    }
    static constexpr unsigned STATION_TAG=0x1055,OFFER_TAG=0x1c03,ACCEPT_TAG=0x2b19;
    static constexpr unsigned MAP_TAG=0x31a6,FOOD_TAG=0x0f47,PORTAL_TAG=0x3672;
    static constexpr unsigned BIRTH_TAG=0x1a97,ATLAS_TAG=0x278d;
    std::uint64_t birth_packet(Decision const& d) const {
        auto v=std::uint64_t(d.split)|(std::uint64_t(map_knowledge.mode)<<12)|
            (std::uint64_t(rescue_history&7)<<16)|(std::uint64_t(d.release_child)<<19)|
            (std::uint64_t(d.protect_child_length>0)<<20)|(std::uint64_t(round)<<21)|
            (std::uint64_t(atlas::revision)<<30)|(std::uint64_t(id&2047)<<38)|(std::uint64_t(team)<<49);
        return v|(std::uint64_t(checksum(v)^BIRTH_TAG)<<50);
    }
    std::uint64_t tagged_packet(int p,int size,unsigned tag,int sender=-1) const {
        return packet(p,size,sender)^(std::uint64_t(tag)<<50);
    }
    std::uint64_t accept_packet(int food,int receiver,int donor) const {
        std::uint64_t v=std::uint64_t(food%w)|(std::uint64_t(food/w)<<6)|(std::uint64_t(receiver)<<12)
            |(std::uint64_t(round)<<24)|(std::uint64_t(donor&65535)<<33)|(std::uint64_t(team)<<49);
        return v|(std::uint64_t(checksum(v)^ACCEPT_TAG)<<50);
    }
    void messages(const std::vector<std::uint64_t>& inbox) {
        auto prune=[&](auto& list,int ttl){list.erase(std::remove_if(list.begin(),list.end(),[&](auto x){return round-x.round>ttl;}),list.end());};
        prune(station_reports,12);prune(delivery_offers,1);prune(delivery_accepts,1);
        allies.erase(std::remove_if(allies.begin(),allies.end(),[&](auto a){return round-a.round>2;}),allies.end());
        for(auto it=portal_busy_until.begin();it!=portal_busy_until.end();) {
            if(it->second<round) it=portal_busy_until.erase(it); else ++it;
        }
        food_reports.erase(std::remove_if(food_reports.begin(),food_reports.end(),[&](auto x){return round-x.round>8||cells[x.pos].seen>=x.round;}),food_reports.end());
        int intel_count=0;
        for(auto m:inbox) {
            auto v=m&((UINT64_C(1)<<50)-1);
            unsigned tag=checksum(v)^unsigned(m>>50);
            if(int((v>>49)&1)!=team)continue;
            if(tag==BIRTH_TAG) {
                int child=v&4095,r=(v>>21)&511,rev=(v>>30)&255;
                if(round==birth_round&&r==round&&child==length&&rev==atlas::revision) {
                    map_knowledge.receive((v>>12)&15,rev,true);
                    if((v>>20)&1) {rescued_grower=((v>>19)&1)==0;rescue_history=(v>>16)&7;}
                }
                continue;
            }
            if(tag==ATLAS_TAG) {
                int r=(v>>24)&511,l=(v>>12)&4095;
                if(r<=round&&round-r<=1)map_knowledge.receive(l&15,l>>4,false);
                continue;
            }
            if(tag==PORTAL_TAG) {
                if(++intel_count>16)continue;
                int a=v&8191,b=(v>>13)&8191,pid=(v>>26)&65535,stamp=(v>>42)&127;
                if(((round-stamp)&127)>1||a>=2*n||b>=2*n||a==b||(a&1)!=(b&1))continue;
                bool conflict=false;
                for(int k:{a,b}) {int p=k/2,d=k&1?3:0;
                    conflict|=cells[p].kind[d]>=0&&(cells[p].kind[d]!=2||cells[p].portal[d]!=pid);
                }
                auto local=portals.find(pid);
                if(local!=portals.end()&&local->second.size()==2)
                    conflict|=std::set<int>(local->second.begin(),local->second.end())!=std::set<int>{a,b};
                if(!conflict)portal_hints[pid]={a,b};
                continue;
            }
            if(tag!=0&&tag!=STATION_TAG&&tag!=OFFER_TAG&&tag!=ACCEPT_TAG&&tag!=MAP_TAG&&tag!=FOOD_TAG)continue;
            int x=v&63,y=(v>>6)&63,l=(v>>12)&4095,r=(v>>24)&511,i=(v>>33)&65535;
            if(x>=w||y>=h||r>round||round-r>2) continue;
            if(tag==MAP_TAG||tag==FOOD_TAG) {
                if(++intel_count>16||round-r>1||i==id)continue;
                int p=y*w+x;
                if(tag==FOOD_TAG) {
                    if(l>6||cells[p].seen>=r)continue;
                    auto it=std::find_if(food_reports.begin(),food_reports.end(),[&](auto f){return f.pos==p;});
                    Intel f{p,r,l==0?-1:r+l};
                    if(it==food_reports.end())food_reports.push_back(f);else if(r>=it->round)*it=f;
                    if(food_reports.size()>16)food_reports.erase(food_reports.begin());
                } else {
                    // Six internal edges of a 3x2 patch. Unknown and portal
                    // encodings are ignored; portal IDs use their own packet.
                    for(int k=0;k<6;k++) {
                        int q=p,d=1;
                        if(k<4)q=((y+k/2)%h)*w+(x+k%2)%w;
                        else {q=y*w+(x+k-4)%w;d=2;}
                        int state=(l>>(2*k))&3;
                        if(state!=1&&state!=2)continue;
                        int far=adjacent[q][d];
                        if(cells[q].kind[d]<0&&hints[q][d]<0)
                            hints[q][d]=hints[far][d^2]=state-1;
                    }
                }
                continue;
            }
            if(tag==ACCEPT_TAG) {
                if(l<n&&round-r<=1) {
                    auto old=std::find_if(delivery_accepts.begin(),delivery_accepts.end(),[&](auto a){return a.id==i;});
                    Signal ack{y*w+x,i,r,l};
                    if(old==delivery_accepts.end())delivery_accepts.push_back(ack);else if(r>=old->round)*old=ack;
                }
                continue;
            }
            if(tag==STATION_TAG||tag==OFFER_TAG) {
                if(l<2||i==id)continue;
                auto& list=tag==STATION_TAG?station_reports:delivery_offers;
                if(round-r>(tag==STATION_TAG?12:1))continue;
                Signal item{y*w+x,i,r,l};
                auto old=std::find_if(list.begin(),list.end(),[&](auto a){return a.id==i;});
                if(old==list.end())list.push_back(item);else if(r>=old->round)*old=item;
                continue;
            }
            // Length zero is a traffic warning, not a dragon status. The ID
            // field then names a portal. It can veto a blind move, never clear it.
            if(l==0) {
                if(round-r<=1) portal_busy_until[i]=std::max(portal_busy_until[i],r+1);
                continue;
            }
            // A rescue parent sends this directly into the child's new rear
            // through tail-origin sonar. Only a matching newborn accepts it.
            if(l==1) {
                if(r==round&&birth_round==round&&(i&4095)==length&&length>=4) {
                    rescued_grower=(i&32768)==0;rescue_history=(i>>12)&7;
                }
                continue;
            }
            if(l<2||i==id) continue;
            int p=y*w+x;
            if(cells[p].seen==round && (cells[p].id!=i || !cells[p].head || cells[p].team!=team)) continue;
            auto it=std::find_if(allies.begin(),allies.end(),[&](auto a){return a.id==i;});
            Ally a{i,p,l,r,r,true}; if(it==allies.end()) allies.push_back(a); else if(r>=it->round) *it=a;
        }
    }
    void navigation() {
        nav_next=next;for(auto& r:nav_reverse)r.clear();
        for(int p=0;p<n;p++)for(int d=0;d<4;d++)
            if(cells[p].kind[d]<0&&hints[p][d]==0)nav_next[p][d]=adjacent[p][d];
        for(auto const& [pid,pair]:portal_hints) {
            bool conflict=false;
            for(int k:pair){int p=k/2,d=k&1?3:0;conflict|=cells[p].kind[d]>=0&&(cells[p].kind[d]!=2||cells[p].portal[d]!=pid);}
            if(conflict)continue;
            auto local=portals.find(pid);
            if(local!=portals.end()&&local->second.size()==2)continue;
            for(int j=0;j<2;j++) {
                int a=pair[j],b=pair[j^1],p=a/2,d=a&1?3:0;
                nav_next[p][d]=adjacent[b/2][d];
                nav_next[adjacent[p][d]][d^2]=b/2;
            }
        }
        if(auto m=map_knowledge.map())for(int p=0;p<n;p++)for(int d=0;d<4;d++)
            if(cells[p].kind[d]<0||(cells[p].kind[d]==2&&next[p][d]<0))
                nav_next[p][d]=m->routes[4*p+d];
        for(int p=0;p<n;p++)for(int v:nav_next[p])if(v>=0)nav_reverse[v].push_back(p);
    }
    void recover_portal_body() {
        // A blind portal was still exactly one move. Recover its new head from
        // the engine without forgetting the known neck on the other side.
        if(pending_portal) {
            if(length>pending_portal_length)last_food=round-1;
            predicted.insert(predicted.begin(),head);
            if(int(predicted.size())>length) predicted.resize(length);
            pending_portal=false;
        }
    }
    void observe(unswbc::Controller const& ct,unswbc::Game const& game) {
        started=std::chrono::steady_clock::now();
        round=game.get_round_num(); length=ct.get_length(); count=ct.get_unit_count(); limit=game.get_unit_limit();
        if(first_observation) {last_food=round;last_enemy=round;birth_round=round;first_observation=false;}
        if(round-birth_round>=24&&round-last_split>=24)rescue_history=0;
        head=pos(ct.get_position()); heading=direction(ct.get_dir().value); enemy_heads.clear(); enemy_size.clear();
        recover_portal_body();
        for(auto const& tile:ct.get_tiles()) {
            int p=pos(tile.get_position()); auto& c=cells[p];
            if(tile.has_pearl()) {if(!c.pearl||c.pearl_since<0)c.pearl_since=round;}
            else c.pearl_since=-1;
            c.seen=round; c.pearl=tile.has_pearl(); c.pearl_round=round;
            c.spawn=tile.get_pearl_time()<0?-1:round+tile.get_pearl_time(); c.id=-1; c.head=false; c.team=-1;
            if(auto part=tile.get_dragon()) {
                c.id=part->get_id(); c.team=part->get_team().value=='A'?0:1; c.head=part->is_head(); c.facing=direction(part->get_dir().value);
                if(c.team!=team) { enemy_size[c.id]++; if(c.head) enemy_heads.push_back(p); }
            }
            for(int d=0;d<4;d++) { auto const& e=tile.get_edge(unswbc::Direction(LETTERS[d])); set_edge(p,d,int(e.get_edge_type()),e.get_portal_id()); }
        }
        transitions(); messages(ct.get_sonar_messages());map_knowledge.update(*this);navigation();
        bool enemy_seen=std::any_of(cells.begin(),cells.end(),[&](auto const& c){return c.seen==round&&c.id>=0&&c.team!=team;});
        if(enemy_seen)last_enemy=round;
        quiet_front=round>=100&&round-last_enemy>=60;
        auto echoes=ct.get_sonar_echoes();
        if(probe_portal>=0&&probe_round==round-1&&
           echoes.ally+echoes.ally_head+echoes.enemy+echoes.enemy_head>0)
            portal_busy_until[probe_portal]=round+1;
        // Visible head/body counts give lower bounds even if sonar misses us.
        std::map<int,int> visible_allies;
        for(auto const& t:ct.get_tiles()) {
            auto const& c=cells[pos(t.get_position())];
            if(c.id!=id&&c.id>=0&&c.team==team) visible_allies[c.id]++;
        }
        for(auto const& t:ct.get_tiles()) {
            int p=pos(t.get_position());auto const& c=cells[p];
            if(!c.head||c.id==id||c.team!=team) continue;
            auto it=std::find_if(allies.begin(),allies.end(),[&](auto a){return a.id==c.id;});
            int l=visible_allies[c.id];
            if(it!=allies.end()) {
                bool exact=it->exact&&it->pos==p&&it->len>=l&&
                    (it->report_round==round||(it->report_round==round-1&&c.id>id));
                int reported=it->report_round;
                if(exact)l=it->len;
                *it={c.id,p,l,round,reported,exact};
            }
            else allies.push_back({c.id,p,l,round});
        }
        threat_sources=enemy_heads;
        // A tail can become a new head and attack in this same round.
        Bits has_follower;
        for(int p=0;p<n;p++) if(cells[p].seen==round&&cells[p].id>=0&&cells[p].team!=team&&!cells[p].head) {
            int q=next[p][cells[p].facing];if(q>=0)has_follower.set(q);
        }
        for(int p=0;p<n;p++) if(cells[p].seen==round&&cells[p].id>=0&&cells[p].team!=team&&!cells[p].head&&!has_follower[p]) {
            if(enemy_size[cells[p].id]>=4) threat_sources.push_back(p);
        }
        bool consistent=!predicted.empty() && predicted[0]==head && int(predicted.size())<=length;
        if(consistent) for(int p:predicted) if(cells[p].seen==round && cells[p].id!=id) {consistent=false;break;}
        if(!consistent) predicted={head};
        {
            Bits used; for(int p:predicted) used.set(p);
            while(int(predicted.size())<length) {
                int found=-1;
                for(auto const& t:ct.get_tiles()) {
                    int p=pos(t.get_position()); auto const& c=cells[p];
                    if(c.id==id && !used[p] && next[p][c.facing]==predicted.back()) {found=p;break;}
                }
                if(found<0) break;
                predicted.push_back(found); used.set(found);
            }
            complete=int(predicted.size())==length;
        }
        last_visit[head]=round;
        threat_distance.assign(n,99);
        std::queue<int> q;
        for(int p:threat_sources) {threat_distance[p]=0;q.push(p);}
        while(!q.empty()) {
            int p=q.front();q.pop(); if(threat_distance[p]>=4) continue;
            for(int v:next[p]) if(v>=0 && threat_distance[v]>threat_distance[p]+1) {
                if(cells[v].seen==round && cells[v].id>=0 && v!=head) continue;
                threat_distance[v]=threat_distance[p]+1;q.push(v);
            }
        }
        identify_enemies();profile_terrain();prepare_territory();prepare_goals();prepare_coordination();prepare_exploration();prepare_atlas();
    }
    State initial() const {
        State s; s.body=predicted;s.heading=heading;s.length=length;
        for(int p:s.body) s.occupied.set(p);
        if(!complete) for(int p=0;p<n;p++) if(cells[p].seen==round&&cells[p].id==id) s.occupied.set(p);
        return s;
    }
    double food_value() const {
        double multiplier=1;
        if(!grower()&&length<=3&&round<200&&count<32)multiplier+=.20;
        if(!grower()&&length<=4&&round-last_food>=12)multiplier+=.15;
        // Workers on an active front need food to retain a paid escape step.
        if(!grower()&&length<=3&&!enemy_heads.empty()&&round<325)multiplier+=.15;
        return policy::food_reward*std::min(1.35,multiplier);
    }
    void prepare_territory() {
        enemy_arrival.assign(n,INF);std::vector<int> q;
        for(int p:enemy_heads){enemy_arrival[p]=0;q.push_back(p);}
        for(size_t i=0;i<q.size();i++) {
            int p=q[i];if(enemy_arrival[p]>=12)continue;
            for(int v:next[p])if(v>=0&&enemy_arrival[v]==INF&&
                !(cells[v].id>=0&&round-cells[v].seen<=1)) {
                enemy_arrival[v]=enemy_arrival[p]+1;q.push_back(v);
            }
        }
    }
    // Separate arrivals handle equal-distance ties explicitly. This estimates
    // ordinary-move territory; the tactical threat checks handle sprint attacks.
    std::pair<double,double> territory(State const& s) const {
        if(enemy_arrival.empty()||enemy_heads.empty())return {0,0};
        std::vector<std::pair<int,int>> q{{s.body[0],0}};Bits seen=s.occupied;seen.set(s.body[0]);
        double ours=0,ties=0;
        for(size_t j=0;j<q.size()&&j<128;j++) {
            auto [p,d]=q[j];double value=1.0/(1+.15*d);
            if(d<enemy_arrival[p])ours+=value;
            else if(d==enemy_arrival[p])ties+=value;
            if(d>=10)continue;
            for(int v:next[p])if(v>=0&&!seen[v]&&!external_block(v)) {
                seen.set(v);q.push_back({v,d+1});
            }
        }
        return {ours,ties};
    }
    bool favorable_standoff(State const& s) const {
        if(grower()||length>3||count<6||mobility(s)<2)return false;
        bool threat=false;
        for(int p:threat_sources)for(int v:next[p])if(v==s.body[0]) {
            // A split tail may create a cheap attacker even when its parent is
            // large. Such a threat never qualifies as an expensive enemy trade.
            auto it=enemy_size.find(cells[p].id);
            if(!cells[p].head||it==enemy_size.end()||it->second<std::max(6,length+2))return false;
            threat=true;
        }
        return threat;
    }
    bool defensive_intercept(int enemy) const {
        if(grower()||length>3||count<4)return false;
        int p=-1;for(int h:enemy_heads)if(cells[h].id==enemy)p=h;
        if(p<0)return false;
        for(int v:next[p])if(v>=0&&cells[v].seen==round&&cells[v].team==team&&cells[v].head&&cells[v].id!=id)
            for(auto a:allies)if(a.id==cells[v].id&&round-a.round<=1&&a.len>=7)return true;
        return false;
    }
    // Bounded enemy-reply / best-escape check. One fully observed opponent,
    // exact bodies and up to three sprint steps; other bodies remain fixed.
    // Unknown continuations prevent declaring a forced trap.
    double tactical_reply_penalty(Candidate const& c) const {
        if(!complete||enemy_heads.size()!=1||expired(.040))return 0;
        int enemy=enemy_heads[0],who=cells[enemy].id;
        auto known=exact_enemy_length.find(who);
        if(known==exact_enemy_length.end()||known->second>16||distance(enemy,c.state.body[0])>3)return 0;
        bool exact=false;auto body=visible_body(who,enemy,exact);if(!exact)return 0;
        Bits others;
        for(int p=0;p<n;p++)if(cells[p].id>=0&&cells[p].id!=id&&cells[p].id!=who&&round-cells[p].seen<=1)others.set(p);
        struct Reply{std::vector<int> body;Bits occupied,eaten;int depth=0;};
        Reply first;first.body=body;for(int p:body)first.occupied.set(p);
        std::vector<Reply> q{first};double worst=0;
        for(size_t j=0;j<q.size()&&j<64&&!expired(.040);j++) {
            auto const current=q[j];if(current.depth>=3||(current.depth>0&&current.body.size()<=2))continue;
            for(int d=0;d<4;d++) {
                int v=next[current.body[0]][d];
                if(v<0||cells[v].seen!=round||current.occupied[v]||others[v])continue;
                if(v==c.state.body[0]){if(!c.standoff)worst=std::max(worst,80.0);continue;}
                if(c.state.occupied[v])continue;
                Reply t=current;t.depth++;
                bool pearl=cells[v].pearl&&cells[v].seen==round&&!c.state.eaten[v]&&!t.eaten[v];
                t.body.insert(t.body.begin(),v);t.occupied.set(v);
                if(!pearl){t.occupied.reset(t.body.back());t.body.pop_back();}else t.eaten.set(v);
                if(current.depth>0){t.occupied.reset(t.body.back());t.body.pop_back();}
                int exits=0;bool unknown=false;
                for(int k=0;k<4;k++) {
                    int x=next[c.state.body[0]][k];
                    unknown|=cells[c.state.body[0]].kind[k]<0||(cells[c.state.body[0]].kind[k]==2&&x<0);
                    if(x>=0&&!c.state.occupied[x]&&!t.occupied[x]&&!others[x])exits++;
                }
                if(!unknown)worst=std::max(worst,exits==0?45.0:exits==1?6.0:0.0);
                q.push_back(std::move(t));
            }
        }
        return worst;
    }
    // Confirmed terrain, not a guessed complete map. Profiles only shape targets.
    void profile_terrain() {
        seen_tiles=0;int edges=0,walls=0,portal_edges=0;
        std::array<int,2> regional_edges{},regional_open{};
        for(int p=0;p<n;p++) {
            if(cells[p].seen>=0)seen_tiles++;
            bool center=p%w>=w/4&&p%w<w-w/4&&p/w>=h/4&&p/w<h-h/4;
            for(int d:{0,3})if(cells[p].kind[d]>=0) {
                edges++;walls+=cells[p].kind[d]==1;portal_edges+=cells[p].kind[d]==2;
                regional_edges[center]++;regional_open[center]+=cells[p].kind[d]==0;
            }
        }
        wall_fraction=double(walls)/std::max(1,edges);
        portal_fraction=double(portal_edges)/std::max(1,edges);
        terrain_confidence=std::min(1.0,edges/128.0);
        regional_confidence=std::min(1.0,std::min(regional_edges[0],regional_edges[1])/64.0);
        center_open_advantage=regional_confidence*(double(regional_open[1]+2)/(regional_edges[1]+4)-double(regional_open[0]+2)/(regional_edges[0]+4));
    }
    bool central(int p) const {return p%w>=w/4&&p%w<w-w/4&&p/w>=h/4&&p/w<h-h/4;}
    int ordinary_degree(int p) const {
        int degree=0;for(int d=0;d<4;d++)degree+=cells[p].kind[d]==0;return degree;
    }
    int information_gain(int p) const {
        int unseen=0;
        for(int dy=-3;dy<=3;dy++)for(int dx=-3;dx<=3;dx++)
            unseen+=cells[((p/w+dy+h)%h)*w+(p%w+dx+w)%w].seen<0;
        return unseen;
    }
    std::vector<int> route_to(int target,bool bodies=true) const {
        std::vector<int> dist(n,INF),q{target};dist[target]=0;
        for(size_t j=0;j<q.size();j++)for(int p:reverse[q[j]])
            if(dist[p]==INF&&(!bodies||!external_block(p))){dist[p]=dist[q[j]]+1;q.push_back(p);}
        return dist;
    }
    // A visible head/body count is exact only when the entire chain and all
    // possible incoming edges to its last segment are observed.
    std::vector<int> visible_body(int who,int start,bool& exact) const {
        std::vector<int> body{start};Bits used;used.set(start);exact=false;
        while(body.size()<MAX) {
            int follower=-1;
            for(int p:reverse[body.back()]) if(cells[p].seen==round&&cells[p].id==who&&!used[p]&&
                    next[p][cells[p].facing]==body.back()) {follower=p;break;}
            if(follower>=0){body.push_back(follower);used.set(follower);continue;}
            int tail=body.back();bool closed=true;
            for(int d=0;d<4;d++) {
                int v=next[tail][d];
                if(cells[tail].kind[d]<0||(cells[tail].kind[d]==2&&v<0))closed=false;
            }
            for(int p:reverse[tail])if(cells[p].seen!=round)closed=false;
            exact=closed;break;
        }
        return body;
    }
    void identify_enemies() {
        exact_enemy_length.clear();
        for(int p:enemy_heads) {
            bool exact=false;auto body=visible_body(cells[p].id,p,exact);
            if(exact)exact_enemy_length[cells[p].id]=int(body.size());
        }
    }
    bool external_block(int p) const {
        return cells[p].id>=0 && cells[p].id!=id && round-cells[p].seen<=1;
    }
    bool step(State const& before,int d,bool extra,State& after,int& hit,bool current=true) const {
        hit=-1;
        int actual_len=before.length;
        if(extra&&actual_len<=2) return false;
        int dest=next[before.body[0]][d];
        if(dest<0 || (current&&cells[dest].seen!=round)) return false;
        if(before.occupied[dest]) return false;
        if(external_block(dest)) {
            if(cells[dest].seen==round&&cells[dest].head&&cells[dest].team!=team) hit=cells[dest].id;
            return false;
        }
        after=before; after.heading=d;after.turns+=!current;
        bool food=cells[dest].pearl && round-cells[dest].pearl_round<=15 && !before.eaten[dest];
        after.body.insert(after.body.begin(),dest);after.occupied.set(dest);
        after.length=actual_len+int(food)-int(extra);
        if(food) {after.eaten.set(dest);after.reward+=food_value()*food_share[dest];}
        // A partial prefix grows as we move. Never invent an unseen tail vacancy.
        while(int(after.body.size())>after.length) {after.occupied.reset(after.body.back());after.body.pop_back();}
        if(extra) after.reward-=(grower()?24.0:round>=450?22.0:policy::sprint_cost);
        return true;
    }
    void prepare_goals() {
        goals.clear();
        std::fill(food_share.begin(),food_share.end(),1.0);
        std::vector<std::pair<double,int>> choices;
        std::vector<int> travel(n,INF), queue{head};travel[head]=0;
        for(size_t i=0;i<queue.size();i++)for(int v:nav_next[queue[i]])if(v>=0&&travel[v]==INF){travel[v]=travel[queue[i]]+1;queue.push_back(v);}
        for(int p=0;p<n;p++) {
            auto const& c=cells[p]; double value=0;
            if(c.pearl&&round-c.pearl_round<=15) value=1.0/(1.0+0.09*(round-c.pearl_round));
            else if(c.spawn>=round&&c.spawn<=round+6) value=.45/(1+.15*(c.spawn-round));
            if(value>0&&travel[p]<INF) choices.push_back({travel[p]/value,p});
        }
        std::sort(choices.begin(),choices.end());
        if(choices.size()>12) choices.resize(12);
        for(auto [unused,p]:choices) {
            auto const& c=cells[p]; double value=c.pearl?1.0/(1+.09*(round-c.pearl_round)):.45/(1+.15*(c.spawn-round));
            Goal g{p,value,std::vector<int>(n,INF)};g.distance[p]=0;
            if(!c.pearl)g.due=c.spawn;
            std::vector<int> q;q.reserve(n);q.push_back(p);
            for(size_t i=0;i<q.size();i++) for(int v:nav_reverse[q[i]]) if(g.distance[v]==INF) {g.distance[v]=g.distance[q[i]]+1;q.push_back(v);}
            // Yield briefly to a demonstrably larger nearby ally, but do not
            // reserve old food forever or leave an immediate gift to an enemy.
            if(c.pearl&&c.pearl_since>=0&&round-c.pearl_since<8&&threat_distance[p]>2) {
                for(auto a:allies) if(round-a.round<=1&&a.len>=6&&a.len>length&&
                    g.distance[a.pos]<=4&&g.distance[a.pos]<=g.distance[head]+1&&
                    g.distance[a.pos]<=499-round&&ally_route(a.pos,p,4)) {
                    food_share[p]=.2;g.reward*=.2;break;
                }
            }
            for(auto ack:delivery_accepts)if(ack.pos==p&&ack.peer>=0&&round-ack.round<=1&&
                cells[ack.peer].seen==round&&cells[ack.peer].head&&cells[ack.peer].team==team&&
                cells[ack.peer].id!=id&&threat_distance[p]>2) {
                if(food_share[p]>.1){g.reward*=.1/food_share[p];food_share[p]=.1;}
            }
            goals.push_back(std::move(g));
        }
        // Reports attract travel only. They never set cells[].pearl or fund a
        // simulated growth/sprint. Fresh sight, even of an empty tile, wins.
        int added=0;
        for(auto f:food_reports)if(added<3&&cells[f.pos].seen<f.round&&travel[f.pos]<INF&&round-f.round<=8) {
            if(std::any_of(goals.begin(),goals.end(),[&](auto const& g){return g.pos==f.pos;}))continue;
            Goal g{f.pos,.55/(1+.2*(round-f.round)),std::vector<int>(n,INF),f.due};
            std::vector<int> q{f.pos};g.distance[f.pos]=0;
            for(size_t i=0;i<q.size();i++)for(int v:nav_reverse[q[i]])if(g.distance[v]==INF){g.distance[v]=g.distance[q[i]]+1;q.push_back(v);}
            goals.push_back(std::move(g));added++;
        }
    }
    bool ally_route(int start,int target,int max_steps) const {
        std::vector<int> dist(n,INF),q{start};dist[start]=0;
        for(size_t i=0;i<q.size();i++) {
            int p=q[i];if(p==target)return true;if(dist[p]>=max_steps)continue;
            for(int d=0;d<4;d++) {
                int v=next[p][d];
                if(v<0||dist[v]!=INF||cells[v].seen<round-1)continue;
                if(cells[v].id>=0&&round-cells[v].seen<=1)continue;
                dist[v]=dist[p]+1;q.push_back(v);
            }
        }
        return false;
    }
    void prepare_exploration() {
        explore_distance.clear();
        bool nearby_food=std::any_of(goals.begin(),goals.end(),[&](auto const& g){
            return g.reward>=.25 && g.distance[head]<=4;
        });
        bool early_scout=!grower()&&round<225&&id%3==team;
        int patience=grower()?20:early_scout||quiet_front?4:8;
        // A barren opening should not make two thirds of the workers wait
        // eight turns before exploring. Nearby productive farms keep priority.
        bool opening_scout=(early_scout||(!grower()&&!nearby_food))&&round<160&&count>=3&&
            (terrain_confidence<.5||wall_fraction<.30)&&seen_tiles<n*3/4;
        if(!opening_scout&&(round-last_food<patience || (nearby_food&&round-last_food<(!grower()&&(early_scout||quiet_front)?8:20))))return;
        std::vector<int> travel(n,INF),q{head};travel[head]=0;
        for(size_t i=0;i<q.size();i++) for(int v:nav_next[q[i]])
            if(v>=0&&travel[v]==INF&&!external_block(v)) {travel[v]=travel[q[i]]+1;q.push_back(v);}
        if(explore_target<0||head==explore_target||round>=explore_until||travel[explore_target]==INF) {
            explore_target=-1;double best=-1e9;
            for(int p:q) if(p!=head) {
                bool frontier=false,portal=false;
                for(int d=0;d<4;d++) {
                    frontier|=cells[p].kind[d]<0;
                    int v=next[p][d];
                    portal|=cells[p].kind[d]==2&&(v<0||round-last_visit[v]>=20);
                }
                int age=round-last_visit[p];
                if(!frontier&&!portal&&age<30) continue;
                int crowd=0;for(auto a:allies)if(round-a.round<=1&&distance(a.pos,p)<=3)crowd++;
                double shape=terrain_confidence*(ordinary_degree(p)-2)*1.5;
                double region=(central(p)?1.0:-1.0)*center_open_advantage*6;
                double portal_value=10+std::min(6.0,portal_fraction*60);
                double value=(frontier?12.0:portal?portal_value:3.0)+std::min(100,age)*.04-travel[p]*.7
                    +std::min(8.0,information_gain(p)*.25)+shape+region-2*crowd;
                if(value>best) {best=value;explore_target=p;}
            }
            explore_until=round+24;
        }
        if(explore_target<0) return;
        explore_distance.assign(n,INF);explore_distance[explore_target]=0;q={explore_target};
        for(size_t i=0;i<q.size();i++) for(int v:nav_reverse[q[i]])
            if(explore_distance[v]==INF&&!external_block(v)) {explore_distance[v]=explore_distance[q[i]]+1;q.push_back(v);}
    }
    void prepare_atlas() {
        atlas_distance.clear();
        auto m=map_knowledge.map();
        if(!m){atlas_target=-1;return;}
        bool nearby=std::any_of(goals.begin(),goals.end(),[&](auto const& g){return g.reward>=.25&&g.distance[head]<=4;});
        if((grower()&&round-last_food<20)||(nearby&&round-last_food<20)||leader>=0||pickup>=0)return;
        std::vector<int> travel(n,INF),q{head};travel[head]=0;
        for(size_t i=0;i<q.size();i++)for(int v:nav_next[q[i]])if(v>=0&&travel[v]==INF&&!external_block(v)) {
            travel[v]=travel[q[i]]+1;q.push_back(v);
        }
        if(atlas_target<0||head==atlas_target||round>=atlas_until||travel[atlas_target]==INF||external_block(atlas_target)) {
            atlas_target=-1;double best=-1e9;
            for(int p:q)if(p!=head&&travel[p]<=499-round) {
                int exits=0;for(int v:nav_next[p])exits+=v>=0&&!external_block(v);
                if(exits<2||m->supply[p]==0)continue;
                double crowded=0;for(auto a:allies)if(round-a.round<=1&&distance(a.pos,p)<=4)crowded+=1;
                // Small stable differences spread routes without changing safety.
                double tie=double((unsigned(p)*2654435761u^unsigned(id)*2246822519u)%1000)/1000;
                double score=12*std::log1p(16.0*m->supply[p]/10000)-.45*travel[p]-m->crowd*crowded+tie;
                if(score>best){best=score;atlas_target=p;}
            }
            atlas_until=round+m->hold;
        }
        if(atlas_target<0)return;
        atlas_distance.assign(n,INF);atlas_distance[atlas_target]=0;q={atlas_target};
        for(size_t i=0;i<q.size();i++)for(int v:nav_reverse[q[i]])if(atlas_distance[v]==INF&&!external_block(v)) {
            atlas_distance[v]=atlas_distance[q[i]]+1;q.push_back(v);
        }
        // One navigation objective: do not add contradictory generic pulls.
        explore_distance.clear();
    }
    double attraction(State const& s) const {
        double best=0;
        for(auto const& g:goals) if(!s.eaten[g.pos] && g.distance[s.body[0]]<INF) {
            double penalty=0;
            // Sharing is already represented by the bounded food_share discount.
            int steps=g.distance[s.body[0]];
            int waiting=g.due<0?0:std::max(0,g.due-round-s.turns-steps);
            double value=9*g.reward/(1+.38*steps+.6*waiting+penalty);
            // Occupying a spawn at the coming round boundary prevents its pearl.
            if(g.due==round+s.turns+1&&s.occupied[g.pos])value*=.1;
            best=std::max(best,value);
        }
        if(!explore_distance.empty()&&explore_distance[head]<INF&&explore_distance[s.body[0]]<INF)
            best+=(grower()?1.0:2.0)*std::clamp(explore_distance[head]-explore_distance[s.body[0]],-6,6);
        if(auto m=map_knowledge.map();m&&!atlas_distance.empty()&&atlas_distance[head]<INF&&atlas_distance[s.body[0]]<INF)
            best+=m->pull*std::clamp(atlas_distance[head]-atlas_distance[s.body[0]],-6,6);
        if(!coord_distance.empty()&&coord_distance[head]<INF&&coord_distance[s.body[0]]<INF)
            best+=(leader>=0?3.0:1.5)*std::clamp(coord_distance[head]-coord_distance[s.body[0]],-6,6);
        return best;
    }
    std::pair<int,int> space(State const& s,int cap=240) const {
        Bits seen=s.occupied;seen.reset(s.body[0]);
        std::vector<int> q;q.reserve(cap+4);q.push_back(s.body[0]);seen.set(s.body[0]);int frontier=0;
        for(size_t i=0;i<q.size()&&int(q.size())<cap;i++) {
            int p=q[i];
            for(int d=0;d<4;d++) {
                if(cells[p].kind[d]<0) {frontier++;continue;}
                int v=next[p][d];
                if(v<0||seen[v]||external_block(v)) continue;
                seen.set(v);q.push_back(v);
            }
        }
        if(int(q.size())>=cap)frontier++; // Search cap: enclosure is unproven.
        return {int(q.size()),frontier};
    }
    int mobility(State const& s) const {
        int k=0; for(int v:next[s.body[0]]) if(v>=0&&!s.occupied[v]&&!external_block(v)) k++;return k;
    }
    int attack_distance(State const& s) const {
        int best=99;
        for(int start:threat_sources) {
            int who=cells[start].id;auto exact=exact_enemy_length.find(who);
            bool enemy_head=cells[start].head;
            int capacity=exact==exact_enemy_length.end()?100:enemy_head?exact->second:std::max(2,exact->second-2);
            struct Node {int p,depth,len,food1=-1,food2=-1;};
            std::vector<Node> q{{start,0,capacity}};
            for(size_t i=0;i<q.size();i++) {
                auto node=q[i];if(node.depth>=3||(node.depth>0&&node.len<=2))continue;
                for(int v:next[node.p]) {
                    if(v<0)continue;
                    if(v==s.body[0]){best=std::min(best,node.depth+1);continue;}
                    if(s.occupied[v]||external_block(v)||node.depth==2)continue;
                    bool pearl=cells[v].pearl&&cells[v].seen==round&&v!=node.food1&&v!=node.food2&&!s.eaten[v];
                    Node child{v,node.depth+1,node.len+int(pearl)-int(node.depth>0),node.food1,node.food2};
                    if(pearl){if(child.food1<0)child.food1=v;else child.food2=v;}
                    q.push_back(child);
                }
            }
        }
        return best;
    }
    double danger(int d) const {return d==1?policy::danger_one:d==2?policy::danger_two:d==3?policy::danger_three:0;}
    double root_score(Candidate& c) const {
        auto [area,frontier]=space(c.state,std::min(400,std::max(80,length*2+20)));c.area=area;c.exits=mobility(c.state);c.threat=attack_distance(c.state);
        c.standoff=c.threat==1&&area>=8&&favorable_standoff(c.state);
        double score=c.state.reward+attraction(c.state)+2*std::log(1+area)-(c.standoff?12:danger(c.threat));
        auto [owned,contested]=territory(c.state);
        if(!enemy_heads.empty())score+=2.5*std::log(1+owned)+.75*std::log(1+contested);
        if(frontier==0 && area<length+5) score-=4*(length+5-area);
        if(c.exits==0) score-=180;
        else if(c.exits==1) score-=9;
        int age=round-last_visit[c.state.body[0]];if(age<8) score-=.7*(8-age);
        // Small deterministic variation only resolves close alternatives.
        unsigned salt=(unsigned(id+1)*2654435761u)^(unsigned(round)*2246822519u)^(unsigned(c.state.body[0])*3266489917u);
        score+=double(salt%1000)/2000;
        return score;
    }
    bool expired(double seconds=policy::search_seconds) const {return std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count()>seconds;}
    void enumerate(State const& s,std::vector<int>& path,std::vector<Candidate>& all,std::vector<Candidate>& attacks) const {
        for(int d=0;d<4;d++) {
            State t;int hit;
            bool ok=step(s,d,!path.empty(),t,hit,true);
            path.push_back(d);
            if(ok) {
                Candidate c;c.state=std::move(t);c.path=path;all.push_back(c);
                if(int(path.size())<policy::max_sprint) enumerate(c.state,path,all,attacks);
            } else if(hit>=0) {Candidate c;c.path=path;c.kill=hit;attacks.push_back(std::move(c));}
            path.pop_back();
        }
    }
    double lookahead(Candidate& c) const {
        std::vector<State> beam{c.state}; double best=-1000;
        int max_depth=std::min(policy::lookahead,499-round);
        for(int depth=1;depth<=max_depth;depth++) {
            if(expired(.036)) break;
            std::vector<std::pair<double,State>> children;
            for(auto const& s:beam) for(int d=0;d<4;d++) {
                State t;int hit;if(!step(s,d,false,t,hit,false)) continue;
                double gain=t.reward-s.reward;
                t.reward=s.reward+gain*std::pow(.94,depth);
                double score=t.reward-c.state.reward+attraction(t);
                int m=mobility(t);if(m==0) score-=150;else if(m==1) score-=5;
                score-=danger(threat_distance[t.body[0]])*.13/(1+depth);
                int age=round-last_visit[t.body[0]]; if(age<6) score-=.18*(6-age);
                children.push_back({score,std::move(t)});
            }
            if(children.empty()) {
                // A pearl pocket can be worth entering if a checked rear exit
                // after splitting still leaves a strictly longer dragon. Only
                // already-observed food counts; hoped-for spawns do not.
                for(auto const& s:beam) if(!expired(.030)&&complete&&count<limit&&
                    s.length>=length+3&&int(s.body.size())==s.length) {
                    auto child=split_part(s,s.length-2,true);
                    auto route=escape(child,std::min(4,500-round));
                    if(route.verified&&route.depth==std::min(4,500-round)&&attack_distance(child)>1) {
                        c.depth=depth;
                        return (s.reward-c.state.reward)-2*policy::food_reward+route.room;
                    }
                }
                bool unknown=false;
                for(auto const& s:beam)for(int d=0;d<4;d++)
                    unknown|=cells[s.body[0]].kind[d]<0||
                        (cells[s.body[0]].kind[d]==2&&next[s.body[0]][d]<0);
                c.depth=depth-1;c.forced_dead=complete&&!unknown;
                return -200+12*depth;
            }
            std::sort(children.begin(),children.end(),[](auto const& a,auto const& b){return a.first>b.first;});
            if(children.size()>policy::beam_width) children.resize(policy::beam_width);
            beam.clear();for(auto& item:children) beam.push_back(std::move(item.second));
            best=children[0].first;c.depth=depth;
        }
        if(c.depth==0) return 0;
        // At least one surviving continuation needs room beyond the search horizon.
        double end=-1e9;
        for(auto const& s:beam) {
            auto [area,frontier]=space(s,std::min(180,std::max(60,length+20)));
            double v=s.reward-c.state.reward+attraction(s)+1.2*std::log(1+area);
            if(frontier==0&&area<int(s.body.size())+3) v-=3*(int(s.body.size())+3-area);
            end=std::max(end,v);
        }
        return std::isfinite(end)?end:best;
    }
    struct Escape {
        int depth=0;
        bool verified=false;
        double room=0;
    };
    Escape escape(State const& initial,int horizon) const {
        Escape result;
        std::vector<State> beam{initial};
        for(int depth=1;depth<=horizon;depth++) {
            if(expired()) return result;
            std::vector<std::pair<double,State>> children;
            for(auto const& s:beam) for(int d=0;d<4;d++) {
                int v=next[s.body[0]][d];
                // Do not base a rescue on a blind portal arrival.
                if(v<0||(cells[s.body[0]].kind[d]==2&&cells[v].seen!=round))continue;
                State t;int hit;if(!step(s,d,false,t,hit,false))continue;
                if(depth==1&&attack_distance(t)<=1)continue;
                if(depth==horizon&&horizon<500-round&&mobility(t)==0)continue;
                auto [room,frontier]=space(t,std::min(80,initial.length+12));
                double value=2*std::log(1+room)+t.reward*.2+mobility(t);
                if(mobility(t)==0&&depth<horizon)value-=100;
                children.push_back({value,std::move(t)});
            }
            if(children.empty()) {result.verified=true;return result;}
            std::sort(children.begin(),children.end(),[](auto const& a,auto const& b){return a.first>b.first;});
            if(children.size()>4)children.resize(4);
            beam.clear();for(auto& c:children)beam.push_back(std::move(c.second));
            result.depth=depth;result.room=children.front().first;
        }
        result.verified=true;return result;
    }
    State split_part(State const& s,int child_size,bool rear) const {
        State part=s;part.reward=0;
        if(rear) {
            part.body.assign(s.body.rbegin(),s.body.rbegin()+child_size);
            part.length=child_size;
            // The child faces away from its former neck, including portals.
            part.heading=cells[part.body[0]].facing^2;
        } else {
            part.body.resize(s.length-child_size);part.length=s.length-child_size;
        }
        // Keep the other half blocked. Moving this half frees only its own tail.
        return part;
    }
    // One further split can expose a junction occupied by our middle. Prove
    // the resulting child's ordinary escape while EVERY discarded half stays
    // blocked. No hoped-for deaths, future spawns, or unseen portal exits.
    int relay_escape(State const& start,int& nodes) const {
        if(start.length<2||int(start.body.size())!=start.length)return 0;
        std::vector<State> beam{start};int best=0;
        for(int turns=0;turns<=3&&!beam.empty();turns++) {
            std::vector<std::pair<double,State>> children;
            for(auto const& s:beam) {
                if(expired(.049)||++nodes>64)return best;
                if(s.length>=4)for(int c:{s.length-2,s.length/2,2}) {
                    if(c<2||c>s.length-2)continue;
                    auto child=split_part(s,c,true);
                    if(attack_distance(child)<=1)continue;
                    auto proof=escape(child,6);
                    if(proof.verified&&proof.depth==6)best=std::max(best,c);
                }
                if(turns==3)continue;
                for(int d=0;d<4;d++) {
                    int v=next[s.body[0]][d];if(v<0||cells[v].seen!=round)continue;
                    State t;int hit;
                    if(!step(s,d,false,t,hit,false)||attack_distance(t)<=1)continue;
                    children.push_back({t.length+mobility(t)*2.0,std::move(t)});
                }
            }
            std::sort(children.begin(),children.end(),[](auto const& a,auto const& b){return a.first>b.first;});
            if(children.size()>4)children.resize(4);
            beam.clear();for(auto& x:children)beam.push_back(std::move(x.second));
        }
        return best;
    }
    int relay_split(State const& s) const {
        if(!complete||length<4||length>16||count+1>=limit||round>=490||expired(.044))return 0;
        int best=0,best_saved=0,nodes=0;
        // A small rear cut leaves the old head enough body to make the next
        // checked cut. Prefer this on ties; no blanket midpoint reversal.
        for(int c=2;c<=std::min(length-2,8);c++) {
            auto front=split_part(s,c,false),rear=split_part(s,c,true);
            if(attack_distance(front)<=1||attack_distance(rear)<=1)continue;
            int saved_front=relay_escape(front,nodes),saved_rear=relay_escape(rear,nodes);
            int saved=saved_front+saved_rear;
            if(saved>best_saved){best_saved=saved;best=c;}
            if(expired(.049)||nodes>64)break;
        }
        return best;
    }
    int rescue_split(State const& s) const {
        if(!complete||length<4||count>=limit)return 0;
        int horizon=std::min(6,500-round);
        int best=0,best_length=0;double best_quality=-1e9;
        std::vector<int> sizes{length-2,2,length-3,3,length-4,4,length/2};
        bool expansion_rescue=rescue_history>0&&round<180&&count<16&&length>=8;
        if(expansion_rescue)sizes.insert(sizes.begin(),length/2);
        std::set<int> tried;
        for(int child_size:sizes) {
            if(child_size<2||child_size>length-2||!tried.insert(child_size).second||expired())continue;
            auto child=split_part(s,child_size,true);
            auto rear=escape(child,horizon);
            bool child_safe=rear.verified&&rear.depth==horizon&&attack_distance(child)>1;
            if(child_safe&&child_size==length-2&&!expansion_rescue)return child_size;
            auto parent=split_part(s,child_size,false);
            int parent_horizon=std::min(6,499-round); // The parent waits on its split turn.
            auto front=escape(parent,parent_horizon);
            bool parent_safe=front.verified&&front.depth==parent_horizon&&attack_distance(parent)>1;
            if(expansion_rescue&&child_size==length/2&&child_safe&&parent_safe)return child_size;
            int saved=std::max(child_safe?child_size:0,parent_safe?length-child_size:0);
            double quality=(child_safe&&parent_safe?20:0)+(child_safe?rear.room:0)+(parent_safe?front.room:0);
            if(saved>best_length||(saved==best_length&&saved>0&&quality>best_quality)) {
                best=child_size;best_length=saved;best_quality=quality;
            }
        }
        return best;
    }
    bool safe_split(State const& s,bool emergency) const {
        if(length<4||count>=limit||!complete) return false;
        // Full occupation killed long scorers with otherwise viable rescues.
        // Ordinary growth leaves two slots; emergency cuts can still use all 64.
        if(!emergency&&limit>=16&&count>=limit-2)return false;
        if(int(s.body.size())<2) return false;
        State child;child.body={s.body.back(),s.body[s.body.size()-2]};child.length=2;
        child.occupied=s.occupied;
        int options=mobility(child);auto [room,frontier]=space(child,50);
        int min_room=emergency?4:8;
        if(options<1||room<min_room||attack_distance(child)<=1) return false;
        // Verify two moves for the actual child, holding its parent's body fixed.
        bool continuation=false;
        for(int d=0;d<4;d++) {State one;int hit;
            if(!step(child,d,false,one,hit,false)) continue;
            for(int e=0;e<4;e++) {State two;if(step(one,e,false,two,hit,false)) continuation=true;}
        }
        if(!continuation) return false;
        int nearby_heads=0;
        for(int p=0;p<n;p++) if(cells[p].seen==round&&cells[p].team==team&&cells[p].head&&distance(p,child.body[0])<=4) nearby_heads++;
        int local_food=0;
        for(auto const& g:goals)if(g.reward>=.25&&g.distance[head]<=6)local_food++;
        // Replenish an outnumbered working front while there is still time to
        // repay the split. Quiet farms and protected scorers keep their length.
        bool replenish=round>=policy::split_before&&round<325&&count<24&&length<=5&&
            !enemy_heads.empty()&&local_food>=2&&options>=2;
        if(!emergency&&(grower()||(count>=8&&(nearby_heads>=3||(nearby_heads>=2&&local_food<2)))||length<policy::split_length||length>6||count>=policy::max_team||
            (round>=policy::split_before&&!replenish)||round-last_split<policy::split_cooldown||attack_distance(s)<=1)) return false;
        if(!emergency) {
            // Under nearby pressure, keep a segment for an emergency second
            // step. Two length-two workers have no such escape. Low population
            // or an immediate safe rear meal still justifies expansion.
            bool pressure=false;for(int p:enemy_heads)pressure|=distance(head,p)<=6;
            bool rear_food=false;for(int v:next[child.body[0]])if(v>=0&&!child.occupied[v]&&!external_block(v)&&cells[v].seen==round&&cells[v].pearl)rear_food=true;
            if(length==4&&count>=4&&pressure&&!rear_food&&!replenish)return false;
            auto parent=split_part(s,2,false);
            auto route=escape(parent,2);
            if(!route.verified||route.depth<2)return false;
        }
        return true;
    }
    bool opening_division(State const& s) const {
        if(round!=0||birth_round!=0||complete||rescue_history||length<8||count>8||count>=limit||s.body.size()<4||ordinary_degree(head)>2)return false;
        int soon=0;for(auto const& c:cells)soon+=c.seen==round&&c.spawn>=round&&c.spawn<=round+4;
        if(soon<2||attack_distance(s)<=1)return false;
        State front=s;front.length=length-length/2;front.body.resize(std::min(int(front.body.size()),front.length));
        for(int d=0;d<4;d++){State one;int hit;
            if(!step(front,d,false,one,hit,true)||attack_distance(one)<=1)continue;
            for(int e=0;e<4;e++){State two;if(step(one,e,false,two,hit,true)&&attack_distance(two)>1)return true;}
        }
        return false;
    }
    Decision split_decision(State s,int child_size,bool rescue=false,bool opening=false) {
        bool release=opening||(rescue&&rescue_history>0&&round<180&&child_size<=6);
        s.length=length-child_size;
        s.body.resize(std::min(int(s.body.size()),s.length));
        s.occupied.reset();for(int p:s.body)s.occupied.set(p);
        if(rescue&&length>=7&&s.length>=4)rescued_grower=true;
        if(rescue)rescue_history=std::min(7,rescue_history+1);
        last_split=round;
        return {{},child_size,s,int(s.body.size())==s.length,false,
                rescue&&length>=7&&child_size>=4?child_size:0,release};
    }
    int exploratory_portal(State const& s,bool last_resort=false) const {
        int best=-1;double score=-1e9;
        for(int d=0;d<4;d++) if(cells[head].kind[d]==2) {
            // Even a newborn with an unseen neck must never reverse into it.
            if(d==(heading^2))continue;
            auto busy=portal_busy_until.find(cells[head].portal[d]);
            if(busy!=portal_busy_until.end()&&busy->second>=round)continue;
            int v=next[head][d];
            if(v>=0&&(s.occupied[v]||external_block(v)||cells[v].seen==round)) continue;
            if(v>=0&&std::any_of(allies.begin(),allies.end(),[&](auto a){
                // A nearby report is a caution, not an occupied destination.
                // With no ordinary move left, do not prefer certain death to
                // that uncertainty. A reported head ON the exit still vetoes.
                return round-a.round<=1&&distance(a.pos,v)<=(last_resort?0:1);
            }))continue;
            double value=v<0?1.0:2.0+std::min(30,round-last_visit[v])*.1;
            value+=double((id*7+round+d*3)%11)*.01;
            if(value>score) {score=value;best=d;}
        }
        return best;
    }
    bool worker_for_delivery() const {
        return round>=350&&count>=4&&complete&&length<=(round<425?4:6)&&
            (!grower()||round>=425)&&round<499;
    }
    bool can_collect() const {return round>=300&&count>=4&&length>=6&&complete;}
    void prepare_coordination() {
        coord_distance.clear();outgoing_accept=-1;coord_event=0;
        int old_leader=leader;
        for(auto offer:delivery_offers)if(round-offer.round<=1&&cells[offer.pos].seen==round&&
            cells[offer.pos].id==offer.id&&cells[offer.pos].team==team&&distance(head,offer.pos)<=4)
            station_engaged_until=round+8;
        if(pickup_until<round){pickup=-1;pickup_donor=-1;}
        if(!can_collect()||threat_distance[head]<=3)station=-1;
        if(can_collect()&&threat_distance[head]>3) {
            bool subordinate=false;
            for(auto a:allies)if(round-a.round<=1&&distance(head,a.pos)<=6&&
                (a.len>length||(a.len==length&&a.id<id)))subordinate=true;
            if(subordinate)station=-1;
            else if(station<0||round>=station_until||distance(head,station)>6||threat_distance[station]<=3) {
                // Select a local circulation area. Never rally everyone into a
                // portal mouth or a one-exit pocket just because it has food.
                std::vector<int> q{head},dist(n,INF);dist[head]=0;station=-1;double best=-1e9;
                for(size_t j=0;j<q.size();j++) {
                    int p=q[j];if(dist[p]>4)continue;
                    bool portal=false;for(int d=0;d<4;d++)portal|=cells[p].kind[d]==2;
                    if(ordinary_degree(p)>=3&&!portal&&threat_distance[p]>4&&cells[p].seen>=round-8&&
                       (p==head||!initial().occupied[p])) {
                        State probe=initial();probe.body[0]=p;probe.occupied.reset(head);probe.occupied.set(p);
                        auto [room,frontier]=space(probe,std::max(24,length+8));
                        if(room>=std::max(12,length+4)) {
                            double crop=0;for(auto const& g:goals)if(g.distance[p]<=4)crop+=g.reward;
                            double score=std::min(room,40)-2*dist[p]+2*std::min(3.0,crop);
                            if(score>best){best=score;station=p;}
                        }
                    }
                    if(dist[p]==4)continue;
                    for(int v:next[p])if(v>=0&&dist[v]==INF&&!external_block(v)){dist[v]=dist[p]+1;q.push_back(v);}
                }
                station_until=round+20;
            }
        }
        leader=-1;leader_site=-1;
        if(!worker_for_delivery()) {
            // Circulate in the advertised area rather than towing followers
            // across the map. Local pearls still outweigh this small pull.
            if(station>=0&&round<=station_engaged_until) {
                coord_distance=route_to(station);
                for(auto& d:coord_distance)if(d<INF)d=std::max(0,d-2);
            }
            return;
        }
        double best=-1e9;
        for(auto sig:station_reports)if(round-sig.round<=12&&sig.id!=id) {
            int size=sig.peer;
            int target=sig.pos;
            for(auto a:allies)if(a.id==sig.id&&round-a.round<=2) {
                size=a.exact?a.len:std::max(size,a.len);
                if(cells[a.pos].seen==round&&cells[a.pos].id==sig.id&&cells[a.pos].head&&distance(a.pos,sig.pos)>6)size=0;
                // The area is the long-range destination. Once in direct
                // contact, use the live head to finish the short handshake.
                if(cells[a.pos].seen==round&&cells[a.pos].id==sig.id&&cells[a.pos].head&&distance(head,a.pos)<=4)target=a.pos;
            }
            if(size<length+2)continue;
            if(cells[target].seen==round&&threat_distance[target]<=3)continue;
            auto dist=route_to(target);int steps=dist[head];
            if(steps> (round<425?10:16)||steps+5>499-round)continue;
            double score=2*std::min(size,round<425?12:64)-steps;
            if(sig.id==old_leader&&round<leader_until)score+=4;
            if(score>best){best=score;leader=sig.id;leader_site=target;coord_distance=std::move(dist);}
        }
        if(leader!=old_leader)leader_until=round+8;
    }
    int sonar_recipient(State const& state,int d) const {
        if(state.body.empty())return -1;
        int p=state.body[0];
        if(d==(state.heading^2)) {
            if(int(state.body.size())!=state.length||state.body.size()<2)return -1;
            p=state.body.back();int toward=-1;
            for(int k=0;k<4;k++)if(next[p][k]==state.body[state.body.size()-2])toward=k;
            if(toward<0)return -1;
            d=toward^2;
        }
        for(int steps=0;steps<w+h;steps++) {
            if(cells[p].kind[d]==1)return -1;
            p=next[p][d];if(p<0||cells[p].seen!=round)return -1;
            if(state.occupied[p])return id;
            if(cells[p].id>=0&&cells[p].id!=id)return cells[p].id;
        }
        return -1;
    }
    bool can_message(State const& s,int recipient) const {
        for(int d=0;d<4;d++)if(sonar_recipient(s,d)==recipient)return true;
        return false;
    }
    void remove_donor(int donor) {
        for(int p=0;p<n;p++)if(cells[p].id==donor) {
            cells[p].id=-1;cells[p].team=-1;cells[p].head=false;
        }
    }
    bool pickup_route(State const& from,int target,State& meal,int& steps,int max_steps=2) const {
        // Future ordinary turns, not a paid sprint. On a checkerboard, a
        // one-turn-only agreement is impossible for half the relative phases.
        for(int d=0;d<4;d++)if(cells[from.body[0]].kind[d]==0&&next[from.body[0]][d]==target) {
            int hit;if(step(from,d,false,meal,hit,true)&&attack_distance(meal)>1){steps=1;return true;}
        }
        if(max_steps<2)return false;
        for(int d=0;d<4;d++)if(cells[from.body[0]].kind[d]==0) {
            State first;int hit;if(!step(from,d,false,first,hit,true)||attack_distance(first)<=1)continue;
            for(int e=0;e<4;e++)if(cells[first.body[0]].kind[e]==0&&next[first.body[0]][e]==target&&
                step(first,e,false,meal,hit,true)&&attack_distance(meal)>1){steps=2;return true;}
        }
        return false;
    }
    bool accept_collection(std::vector<Candidate> const& all,Decision& out,double normal_score=-1e9) {
        if(!can_collect()||station<0||all.empty()||expired(.040)||round>=499)return false;
        double best=std::max(all.front().score,normal_score);int chosen=-1,donor=-1,drop=-1,examined=0;
        for(auto offer:delivery_offers) {
            if(examined>=3||expired(.043))break;
            if(round-offer.round>1||cells[offer.pos].seen!=round||cells[offer.pos].id!=offer.id||
               cells[offer.pos].team!=team||!cells[offer.pos].head||offer.id==id||distance(head,offer.pos)>4)continue;
            bool exact=false;auto body=visible_body(offer.id,offer.pos,exact);
            if(!exact||body.size()<2||body.size()>6||length<int(body.size())+2||threat_distance[offer.pos]<=4)continue;
            examined++;
            Brain after_drop=*this;after_drop.remove_donor(offer.id);
            for(size_t k=0;k<body.size();k+=2) {
                auto& cell=after_drop.cells[body[k]];cell.pearl=true;cell.pearl_round=round;cell.pearl_since=round;
            }
            int index=0;
            for(auto const& c:all) {
                int j=index++;
                if(j>=8||expired(.044))break;
                if(c.threat<=1||c.path.size()!=1||c.exits<1||!can_message(c.state,offer.id))continue;
                State meal;int walking=0;
                if(!after_drop.pickup_route(c.state,offer.pos,meal,walking,std::min(2,499-round)))continue;
                int horizon=std::min(policy::lookahead,499-round-walking);auto continuation=after_drop.escape(meal,horizon);
                if(!continuation.verified||continuation.depth<horizon)continue;
                // Only credit donor pearls on the certified pickup route. The
                // rest of its corpse may be stolen, blocked or never collected.
                int verified_meals=0;for(size_t k=0;k<body.size();k+=2)
                    verified_meals+=meal.eaten[body[k]]&&!c.state.eaten[body[k]];
                double value=c.score+8.0*verified_meals-3*(walking-1);
                if(value>best){best=value;chosen=j;donor=offer.id;drop=offer.pos;}
            }
        }
        if(chosen<0)return false;
        auto const& c=all[chosen];out={c.path,0,c.state,true,false};
        outgoing_accept=donor;pickup=drop;pickup_donor=donor;pickup_until=std::min(499,round+2);coord_event=1;
        return true;
    }
    bool collect_promised_food(std::vector<Candidate> const& all,Decision& out) {
        if(pickup<0||round>pickup_until)return false;
        int target=pickup;
        if(cells[target].seen!=round||!cells[target].pearl||cells[target].id>=0){pickup=-1;pickup_donor=-1;return false;}
        for(auto const& c:all)if(!c.forced_dead&&c.threat>1&&c.exits>0&&c.path.size()==1&&
            c.state.body[0]==target&&c.state.length>length) {
            int horizon=std::min(policy::lookahead,499-round);auto proof=escape(c.state,horizon);
            if(!proof.verified||proof.depth<horizon)continue;
            out={c.path,0,c.state,int(c.state.body.size())==c.state.length,false};pickup=-1;pickup_donor=-1;coord_event=2;return true;
        }
        if(round<pickup_until)for(auto const& c:all)if(!c.forced_dead&&c.threat>1&&c.exits>0&&c.path.size()==1) {
            State meal;int steps=0;
            if(!pickup_route(c.state,target,meal,steps,1))continue;
            int horizon=std::min(policy::lookahead,498-round);auto continuation=escape(meal,horizon);
            if(!continuation.verified||continuation.depth<horizon)continue;
            out={c.path,0,c.state,int(c.state.body.size())==c.state.length,false};
            outgoing_accept=pickup_donor;coord_event=4;return true;
        }
        pickup=-1;pickup_donor=-1;
        return false;
    }
    int agreed_donation(State const& s) {
        if(round<350||count<4||!complete||length>(round<425?4:6)||
            (grower()&&round<425)||threat_distance[head]<=4)return -1;
        for(auto ack:delivery_accepts) {
            if(ack.id!=id||ack.pos!=head||ack.peer<0||round-ack.round>1||cells[ack.peer].seen!=round||!cells[ack.peer].head||cells[ack.peer].team!=team)continue;
            int collector=cells[ack.peer].id;
            if(collector==id||(round==499&&collector<id))continue;
            // An earlier-round acknowledgement is valid only if its author has
            // not acted again; its head must still be exactly where promised.
            if(ack.round<round&&collector<id)continue;
            bool exact=false;auto body=visible_body(collector,ack.peer,exact);
            int lower=int(body.size());
            for(auto a:allies)if(a.id==collector&&a.exact&&round-a.round<=1)lower=std::max(lower,a.len);
            if(lower<length+2||threat_distance[ack.peer]<=4)continue;
            std::vector<std::pair<int,int>> approaches;
            for(int d=0;d<4;d++)if(cells[ack.peer].kind[d]==0) {
                int v=next[ack.peer][d];
                if(v==head)approaches.push_back({1,ack.peer});
                else if(v>=0&&cells[v].seen==round&&cells[v].id<0&&threat_distance[v]>2)
                    for(int e=0;e<4;e++)if(cells[v].kind[e]==0&&next[v][e]==head)approaches.push_back({2,v});
            }
            int turns_left=499-round+(collector>id);
            // Recheck the immediate exit after the meal with every visible body
            // still blocked except us. The approach tile becomes the neck and
            // therefore cannot double as the only supposedly safe exit.
            bool exit=false;
            for(auto [travel,neck]:approaches)if(travel<=turns_left)
                for(int v:next[head])if(v>=0&&cells[v].seen==round&&v!=neck&&v!=ack.peer&&
                    (cells[v].id<0||cells[v].id==id)&&threat_distance[v]>1)exit=true;
            if(!exit)continue;
            for(int d=0;d<4;d++)if(next[head][d]==s.body[1]){coord_event=3;return d;}
        }
        return -1;
    }
    int offer_recipient(State const& s) const {
        if(!worker_for_delivery())return -1;
        if(leader>=0&&can_message(s,leader))return leader;
        // Contact need not wait for a station broadcast to find its way back.
        // This is only an offer; the receiver must still certify its full size
        // and route before it can authorise a death.
        for(auto a:allies)if(round-a.round<=1&&a.len>=std::max(4,length+2)&&can_message(s,a.id))return a.id;
        return -1;
    }
    bool coordination_sonar(State const& s) const {
        return outgoing_accept>=0||offer_recipient(s)>=0||
            (station>=0&&can_collect());
    }
    std::uint64_t portal_packet(int portal,std::array<int,2> edges) const {
        std::uint64_t v=std::uint64_t(edges[0])|(std::uint64_t(edges[1])<<13)|
            (std::uint64_t(portal)<<26)|(std::uint64_t(round&127)<<42)|(std::uint64_t(team)<<49);
        return v|(std::uint64_t(checksum(v)^PORTAL_TAG)<<50);
    }
    std::uint64_t choose_intel(int recipient_pos,bool education,State const& after) {
        // Directly seen food, then a locally confirmed portal pair, then six
        // ordinary/wall edges. Send each item at most once per eight turns.
        int food=-1;double value=-1e9;
        for(int p=0;p<n;p++)if(cells[p].seen==round&&cells[p].id<0&&!after.eaten[p]&&!after.occupied[p]&&
            (cells[p].pearl||(cells[p].spawn>round&&cells[p].spawn<=round+6))) {
            int token=2*n+p;auto it=intel_sent.find(token);
            if(it!=intel_sent.end()&&round-it->second<8&&!education)continue;
            double score=(cells[p].pearl?12:6)-distance(recipient_pos,p);
            if(score>value){value=score;food=p;}
        }
        if(food>=0&&(education||(round+id)%4==1)) {
            intel_sent[2*n+food]=round;
            return tagged_packet(food,cells[food].pearl?0:cells[food].spawn-round,FOOD_TAG);
        }
        int portal=-1;double best=-1e9;
        for(auto const& [pid,pair]:portals)if(pid>=0&&pid<=65535&&pair.size()==2) {
            int token=4*n+pid;auto it=intel_sent.find(token);
            if(it!=intel_sent.end()&&round-it->second<8)continue;
            double score=20-std::min(distance(recipient_pos,pair[0]/2),distance(recipient_pos,pair[1]/2));
            if(score>best){best=score;portal=pid;}
        }
        if(portal>=0) {
            intel_sent[4*n+portal]=round;auto const& pair=portals.at(portal);
            return portal_packet(portal,{pair[0],pair[1]});
        }
        int anchor=-1,encoded=0;best=-1e9;
        // One bounded local patch per transmission; relay is deliberately not
        // used, so no report is laundered into direct-observation authority.
        for(int p=0;p<n;p++)if(cells[p].seen>=0) {
            auto it=intel_sent.find(p);if(it!=intel_sent.end()&&round-it->second<8)continue;
            int bits=0,known=0,novel=0,x=p%w,y=p/w;
            for(int k=0;k<6;k++) {
                int q=p,d=1;
                if(k<4)q=((y+k/2)%h)*w+(x+k%2)%w;
                else {q=y*w+(x+k-4)%w;d=2;}
                int kind=cells[q].kind[d];
                if(kind==0||kind==1){bits|=(kind+1)<<(2*k);known++;novel+=distance(recipient_pos,q)>3;}
            }
            if(known<3)continue;
            double score=known+novel*2-.25*distance(recipient_pos,p)+.02*std::max(-50,cells[p].seen-round);
            if(score>best){best=score;anchor=p;encoded=bits;}
        }
        if(anchor>=0){intel_sent[anchor]=round;return tagged_packet(anchor,encoded,MAP_TAG);}
        if(food>=0){intel_sent[2*n+food]=round;return tagged_packet(food,cells[food].pearl?0:cells[food].spawn-round,FOOD_TAG);}
        return 0;
    }
    void send_intelligence(Decision const& action,unswbc::Controller& ct) {
        if(action.dying||action.split||!action.exact||action.after.body.empty()||probe_round==round)return;
        if(!action.split&&coordination_sonar(action.after))return;
        bool education=action.split||round-last_split==1;
        if(!education&&(round+id)%2==0)return;
        std::vector<std::pair<int,int>> rays;
        for(int d=0;d<4;d++) {
            int who=sonar_recipient(action.after,d);if(who<0||who==id)continue;
            int location=-1;
            for(int p=0;p<n;p++)if(cells[p].seen==round&&cells[p].id==who&&cells[p].team==team) {
                location=p;if(cells[p].head)break;
            }
            if(location>=0)rays.push_back({d,location});
        }
        if(rays.empty())return;
        auto [d,p]=rays[(round+id)%rays.size()];
        auto msg=(map_knowledge.mode!=0&&(round+id)%8==1)?
            tagged_packet(action.after.body[0],(atlas::revision<<4)|map_knowledge.mode,ATLAS_TAG):
            choose_intel(p,education,action.after);
        if(msg)ct.send_sonar(unswbc::Direction(LETTERS[d]),msg);
    }
    void send_coordination(Decision const& action,unswbc::Controller& ct) {
        if(action.dying||action.split||!action.exact||action.after.body.empty())return;
        auto const& s=action.after;
        if(outgoing_accept>=0) {
            auto msg=accept_packet(pickup,s.body[0],outgoing_accept);
            for(int d=0;d<4;d++)ct.send_sonar(unswbc::Direction(LETTERS[d]),msg);
        } else if(offer_recipient(s)>=0) {
            auto msg=tagged_packet(s.body[0],s.length,OFFER_TAG);
            for(int d=0;d<4;d++)ct.send_sonar(unswbc::Direction(LETTERS[d]),msg);
        } else if(station>=0&&can_collect()) {
            // Alternating rays retain position/length reports while advertising
            // a circulation area. Delivery itself remains opportunistic.
            for(int d=0;d<4;d++)if((d&1)==((round+id)&1))
                ct.send_sonar(unswbc::Direction(LETTERS[d]),tagged_packet(station,s.length,STATION_TAG));
        }
    }
    Decision choose() {
        outgoing_accept=-1;coord_event=0;
        State s=initial();std::vector<Candidate> all,attacks;std::vector<int> path;
        enumerate(s,path,all,attacks);
        for(auto& c:all) c.base=c.score=root_score(c);
        // Honour a meal before shortlist pruning; use the same full horizon
        // that certified it, then recheck against newly visible obstacles.
        Decision coordinated;
        if(collect_promised_food(all,coordinated))return coordinated;
        std::vector<Candidate> offer_options;
        if(can_collect()&&!delivery_offers.empty())for(auto const& c:all)if(c.path.size()==1)offer_options.push_back(c);
        bool sheltered=std::any_of(all.begin(),all.end(),[](auto const& c){return c.threat>1&&c.exits>0&&c.area>=6;});
        if(sheltered) all.erase(std::remove_if(all.begin(),all.end(),[](auto const& c){return c.threat<=1&&!c.standoff;}),all.end());
        std::sort(all.begin(),all.end(),[](auto const& a,auto const& b){return a.base>b.base;});
        if(all.size()>policy::candidate_limit) all.resize(policy::candidate_limit);
        for(auto& c:all) {
            if(expired(.036)) break;
            c.score+=lookahead(c);
            if(c.depth>0)c.score-=attraction(c.state);
        }
        if(round>=425||all.size()<=3||enemy_heads.size()==1)for(size_t j=0;j<std::min<size_t>(6,all.size());j++)
            all[j].score-=tactical_reply_penalty(all[j]);
        // Food must not outscore a route that survives when another branch has
        // actually exhausted all continuations inside the search horizon.
        if(std::any_of(all.begin(),all.end(),[](auto const& c){return !c.forced_dead&&c.depth>0;}))
            all.erase(std::remove_if(all.begin(),all.end(),[](auto const& c){return c.forced_dead;}),all.end());
        std::sort(all.begin(),all.end(),[](auto const& a,auto const& b){return a.score>b.score;});
        bool trapped=all.empty() || all[0].forced_dead || (all[0].exits==0 && all[0].area<=2) || (all[0].threat==1&&!all[0].standoff);
        // Compare targets by visible lower bounds, preferring the largest trade.
        Candidate const* trade=nullptr;
        for(auto const& a:attacks) if((valuable_trade(enemy_size[a.kill])||defensive_intercept(a.kill))&&
            (!trade||enemy_size[a.kill]>enemy_size[trade->kill])) trade=&a;
        int donation=agreed_donation(s);
        if(donation>=0)return {{donation},0,s,false,true};
        if(trade) return {trade->path,0,s,false,true};
        if(opening_division(s))return split_decision(s,length/2,true,true);
        if(trapped) {
            int child=rescue_split(s);
            if(!child&&(all.empty()||all[0].forced_dead))child=relay_split(s);
            if(child) return split_decision(s,child,true);
        } else if(safe_split(s,false)) return split_decision(s,2);
        // No automatic opening cut: fresh children with useful moves must not
        // repeatedly reverse and shed two more segments in this same round.
        if(!complete&&length>=4&&count<limit&&s.body.size()>=2 &&
           (all.empty()||(!all.empty()&&all[0].forced_dead))) {
            return split_decision(s,rescue_history>0&&round<180&&count<16&&length>=8?length/2:length-2,true);
        }
        if(!trapped) {
            for(auto& c:offer_options)for(auto const& tested:all)if(c.path==tested.path)c.score=tested.score;
            std::sort(offer_options.begin(),offer_options.end(),[](auto const& a,auto const& b){return a.score>b.score;});
            if(accept_collection(offer_options,coordinated,all.empty()?-1e9:all.front().score))return coordinated;
        }
        int portal=exploratory_portal(s,all.empty());
        bool portal_risk_allowed=!grower()&&length<=6&&round-last_portal>=8;
        bool stagnant=round-last_food>=8;
        bool immediate_food=std::any_of(all.begin(),all.end(),[](auto const& c){return c.state.eaten.any();});
        bool doomed=all.empty()||(!all.empty()&&all[0].forced_dead);
        if(portal>=0&&(doomed||(portal_risk_allowed&&!immediate_food&&stagnant))) {
            last_portal=round;return {{portal},0,{},false,false};
        }
        for(auto const& a:attacks) {
            bool no_moves=all.empty();
            bool can_survive=count>1;
            bool draw_chance=count==1&&enemy_heads.size()<=1;
            if(no_moves&&(can_survive||draw_chance))
                return {a.path,0,s,false,true};
        }
        if(!all.empty()) return {all[0].path,0,all[0].state,int(all[0].state.body.size())==all[0].state.length,false};
        // Avoid killing an ally's head even when our own loss is unavoidable.
        for(int k=0;k<4;k++) {int d=(heading+k)%4,v=next[head][d];
            if(v<0||cells[v].team!=team||!cells[v].head) return {{d},0,s,false,true};}
        return {{heading},0,s,false,true};
    }
    void commit(Decision const& d,unswbc::Controller& ct) {
        if(d.split) ct.do_split(d.split);
        else {std::vector<unswbc::Direction> dirs;for(int v:d.path)dirs.emplace_back(LETTERS[v]);ct.make_moves(dirs);}
        if(!d.dying&&d.after.body.empty()&&d.path.size()==1&&cells[head].kind[d.path[0]]==2) {
            pending_portal=true;pending_portal_length=length;
            int portal=cells[head].portal[d.path[0]];
            if(portal>=0&&portal<=65535)
                for(int k=0;k<4;k++)ct.send_sonar(unswbc::Direction(LETTERS[k]),packet(head,0,portal));
        } else if(!d.dying&&!d.after.body.empty()) {
            if(d.split)for(int p=0;p<n;p++)if(cells[p].id==id&&cells[p].seen==round&&
                std::find(d.after.body.begin(),d.after.body.end(),p)==d.after.body.end()) {
                // The child's actual new ID is unknown until observed. Retain
                // its body as a recent allied obstacle under a placeholder ID.
                cells[p].id=std::numeric_limits<int>::max();cells[p].team=team;
                cells[p].head=complete&&!predicted.empty()&&p==predicted.back();
            }
            predicted=d.after.body;
            if(d.after.eaten.any()) last_food=round;
            if(d.split) {
                // All rays: a bend can block even tail-origin education. An
                // uneducated child safely locks itself into general mode.
                auto msg=birth_packet(d);
                for(int k=0;k<4;k++)ct.send_sonar(unswbc::Direction(LETTERS[k]),msg);
                return;
            }
            if(d.exact&&(count>1||d.split)) {
                auto msg=packet(predicted[0],d.after.length);
                int probe=-1,busy=-1;
                for(int k=0;k<4;k++)if(cells[predicted[0]].kind[k]==2) {
                    int portal=cells[predicted[0]].portal[k],v=next[predicted[0]][k];
                    if(portal<0||portal>65535)continue;
                    busy=portal;
                    // Exactly one sonar makes its next-turn echo attributable.
                    // Opposite-facing sonar starts at the tail, so cannot probe
                    // a portal next to this head reliably.
                    if(k!=(d.after.heading^2)&&(v<0||cells[v].seen!=round))probe=k;
                }
                // Portal echoes have no direction labels. A coordination turn
                // sends several rays, so it cannot also claim a single-ray probe.
                if(probe>=0&&!coordination_sonar(d.after)) {
                    probe_portal=cells[predicted[0]].portal[probe];probe_round=round;
                    ct.send_sonar(unswbc::Direction(LETTERS[probe]),packet(predicted[0],0,probe_portal));
                } else for(int k=0;k<4;k++) {
                    auto value=(busy>=0&&(k&1)==((round+id)&1))?packet(predicted[0],0,busy):msg;
                    ct.send_sonar(unswbc::Direction(LETTERS[k]),value);
                }
            }
        } else predicted.clear();
        send_coordination(d,ct);
        send_intelligence(d,ct);
    }
};
}
