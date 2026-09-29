#pragma once
#include "helper.hpp"
#include "config.hpp"
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
    double spawn_interval=0;
    bool pearl=false;
    int id=-1, team=-1, facing=0;
    bool head=false;
    std::array<int,4> kind{{-1,-1,-1,-1}}, portal{{-1,-1,-1,-1}};
};
struct Goal { int pos; double reward; std::vector<int> distance; int due=-1; };
struct Ally { int id,pos,len,round; int report_round=-10000; bool exact=false; };
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
    bool forced_dead=false;
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
    std::map<int,int> enemy_size;
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
    int rescue_chain=0;
    bool feeding_simulation=false;
    int farm_target=-1,farm_until=-1;
    std::vector<int> farm_distance;
    int pending_portal_length=0, birth_round=-1;
    int probe_portal=-1, probe_round=-1000;
    std::chrono::steady_clock::time_point started;
    Brain(int width,int height,int dragon,int side):w(width),h(height),n(w*h),id(dragon),team(side),cells(n),next(n),adjacent(n),reverse(n),last_visit(n,-1000),threat_distance(n,99),food_share(n,1.0),started(std::chrono::steady_clock::now()) {
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
        return count>=3 && length<=4 && !grower() &&
            enemy_length>=std::max(round<400?6:0,length+2);
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
    void messages(const std::vector<std::uint64_t>& inbox) {
        allies.erase(std::remove_if(allies.begin(),allies.end(),[&](auto a){return round-a.round>2;}),allies.end());
        for(auto it=portal_busy_until.begin();it!=portal_busy_until.end();) {
            if(it->second<round) it=portal_busy_until.erase(it); else ++it;
        }
        for(auto m:inbox) {
            auto v=m&((UINT64_C(1)<<50)-1);
            if(checksum(v)!=(m>>50) || int((v>>49)&1)!=team) continue;
            int x=v&63,y=(v>>6)&63,l=(v>>12)&4095,r=(v>>24)&511,i=(v>>33)&65535;
            if(x>=w||y>=h||r>round||round-r>2) continue;
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
                    rescued_grower=(i&32768)==0;rescue_chain=(i>>12)&7;
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
        if(round-birth_round>=24&&round-last_split>=24)rescue_chain=0;
        if(first_observation) {last_food=round;birth_round=round;first_observation=false;}
        head=pos(ct.get_position()); heading=direction(ct.get_dir().value); enemy_heads.clear(); enemy_size.clear();
        recover_portal_body();
        for(auto const& tile:ct.get_tiles()) {
            int p=pos(tile.get_position()); auto& c=cells[p];
            // A consecutively observed countdown reset is an actual interval
            // sample. A single countdown is not an assumed spawn distribution.
            if(c.seen==round-1&&c.spawn==round&&tile.get_pearl_time()>0)
                c.spawn_interval=c.spawn_interval>0?.75*c.spawn_interval+.25*tile.get_pearl_time():tile.get_pearl_time();
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
        transitions(); messages(ct.get_sonar_messages());
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
        prepare_goals();prepare_farms();prepare_exploration();
    }
    State initial() const {
        State s; s.body=predicted;s.heading=heading;s.length=length;
        for(int p:s.body) s.occupied.set(p);
        if(!complete) for(int p=0;p<n;p++) if(cells[p].seen==round&&cells[p].id==id) s.occupied.set(p);
        return s;
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
        if(food) {after.eaten.set(dest);after.reward+=policy::food_reward*food_share[dest];}
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
        for(size_t i=0;i<queue.size();i++)for(int v:next[queue[i]])if(v>=0&&travel[v]==INF){travel[v]=travel[queue[i]]+1;queue.push_back(v);}
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
            for(size_t i=0;i<q.size();i++) for(int v:reverse[q[i]]) if(g.distance[v]==INF) {g.distance[v]=g.distance[q[i]]+1;q.push_back(v);}
            // Yield briefly to a demonstrably larger nearby ally, but do not
            // reserve old food forever or leave an immediate gift to an enemy.
            if(c.pearl&&c.pearl_since>=0&&round-c.pearl_since<8&&threat_distance[p]>3&&
               (round>=250||count>=16)) {
                for(auto a:allies) if(round-a.round<=1&&a.len>=6&&a.len>length&&
                    g.distance[a.pos]<=4&&g.distance[a.pos]<=g.distance[head]&&
                    g.distance[a.pos]<=499-round&&ally_route(a.pos,p,4)) {
                    food_share[p]=.2;g.reward*=.2;break;
                }
            }
            goals.push_back(std::move(g));
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
    void prepare_farms() {
        farm_distance.clear();
        if(grower()||round>=300)return;
        std::vector<double> density(n,0);
        for(int source=0;source<n;source++)if(cells[source].spawn>=0) {
            auto const& c=cells[source];
            double interval=c.spawn_interval>0?c.spawn_interval:std::max(4,c.spawn-c.seen);
            double supply=1.0/(4+interval);
            Bits seen;seen.set(source);std::vector<std::pair<int,int>> local{{source,0}};
            for(size_t j=0;j<local.size();j++) {
                auto [p,steps]=local[j];density[p]+=supply/(1+steps);
                if(steps==3)continue;
                for(int v:next[p])if(v>=0&&!seen[v]) {seen.set(v);local.push_back({v,steps+1});}
            }
        }
        std::vector<int> travel(n,INF),q{head};travel[head]=0;
        for(size_t j=0;j<q.size();j++)for(int v:next[q[j]])
            if(v>=0&&travel[v]==INF&&!external_block(v)){travel[v]=travel[q[j]]+1;q.push_back(v);}
        auto quality=[&](int p) {
            int crowd=0;for(auto a:allies)if(round-a.round<=1&&distance(a.pos,p)<=3)crowd++;
            return density[p]/(1+.35*crowd);
        };
        double local=quality(head);
        if(farm_target<0||head==farm_target||round>=farm_until||travel[farm_target]==INF||quality(farm_target)<=local*1.15) {
            farm_target=-1;double best=std::max(.025,local*1.3);
            for(int p:q)if(p!=head) {
                double value=quality(p)/(1+.08*travel[p]);
                if(value>best){best=value;farm_target=p;}
            }
            farm_until=round+24;
        }
        if(farm_target<0)return;
        farm_distance.assign(n,INF);farm_distance[farm_target]=0;q={farm_target};
        for(size_t j=0;j<q.size();j++)for(int v:reverse[q[j]])
            if(farm_distance[v]==INF&&!external_block(v)){farm_distance[v]=farm_distance[q[j]]+1;q.push_back(v);}
    }
    void prepare_exploration() {
        explore_distance.clear();
        bool nearby_food=std::any_of(goals.begin(),goals.end(),[&](auto const& g){
            return g.reward>=.25 && g.distance[head]<=4;
        });
        bool early_scout=!grower()&&round<300&&id%3==team;
        int patience=grower()?20:early_scout?4:8;
        if(round-last_food<patience || (nearby_food&&round-last_food<(early_scout?8:20))) return;
        std::vector<int> travel(n,INF),q{head};travel[head]=0;
        for(size_t i=0;i<q.size();i++) for(int v:next[q[i]])
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
                double value=(frontier?12.0:portal?10.0:3.0)+std::min(100,age)*.04-travel[p]*.7;
                if(value>best) {best=value;explore_target=p;}
            }
            explore_until=round+24;
        }
        if(explore_target<0) return;
        explore_distance.assign(n,INF);explore_distance[explore_target]=0;q={explore_target};
        for(size_t i=0;i<q.size();i++) for(int v:reverse[q[i]])
            if(explore_distance[v]==INF&&!external_block(v)) {explore_distance[v]=explore_distance[q[i]]+1;q.push_back(v);}
    }
    double attraction(State const& s) const {
        double best=0;
        for(auto const& g:goals) if(!s.eaten[g.pos] && g.distance[s.body[0]]<INF) {
            double penalty=0; // Sharing already discounts the verified food goal.
            int steps=g.distance[s.body[0]];
            int waiting=g.due<0?0:std::max(0,g.due-round-s.turns-steps);
            double value=9*g.reward/(1+.38*steps+.6*waiting+penalty);
            // Occupying a spawn at the coming round boundary prevents its pearl.
            if(g.due==round+s.turns+1&&s.occupied[g.pos])value*=.1;
            best=std::max(best,value);
        }
        if(!explore_distance.empty()&&explore_distance[head]<INF&&explore_distance[s.body[0]]<INF)
            best+=(grower()?1.0:2.0)*std::clamp(explore_distance[head]-explore_distance[s.body[0]],-6,6);
        if(!farm_distance.empty()&&farm_distance[head]<INF&&farm_distance[s.body[0]]<INF)
            best+=2.5*std::clamp(farm_distance[head]-farm_distance[s.body[0]],-8,8);
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
        return {int(q.size()),frontier};
    }
    int mobility(State const& s) const {
        int k=0; for(int v:next[s.body[0]]) if(v>=0&&!s.occupied[v]&&!external_block(v)) k++;return k;
    }
    int attack_distance(State const& s) const {
        int best=99;
        for(int start:threat_sources) {
            Bits seen; seen.set(start); std::vector<std::pair<int,int>> q{{start,0}};
            for(size_t i=0;i<q.size();i++) {
                auto [p,depth]=q[i];if(depth>=3) continue;
                for(int v:next[p]) {
                    if(v<0||seen[v]) continue;
                    if(v==s.body[0]) {best=std::min(best,depth+1);continue;}
                    if(s.occupied[v]||external_block(v)) continue;
                    seen.set(v);q.push_back({v,depth+1});
                }
            }
        }
        return best;
    }
    double danger(int d) const {
        bool worker=!grower()&&length<=6;
        return d==1?policy::danger_one:d==2?(worker?14:policy::danger_two):d==3?(worker?2:policy::danger_three):0;
    }
    double root_score(Candidate& c) const {
        auto [area,frontier]=space(c.state,std::min(400,std::max(80,length*2+20)));c.area=area;c.exits=mobility(c.state);c.threat=attack_distance(c.state);
        double score=c.state.reward+attraction(c.state)+2*std::log(1+area)-danger(c.threat);
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
        State part=s;part.reward=0;part.eaten.reset();
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
    int rescue_split(State const& s) const {
        if(!complete||length<4||count>=limit)return 0;
        int horizon=std::min(6,500-round);
        int best=0,best_length=0;double best_quality=-1e9;
        std::vector<int> sizes{length-2,2,length-3,3,length-4,4,length/2};
        bool repeated=rescue_chain>0&&round<350&&length>=8;
        if(repeated){sizes.insert(sizes.begin(),(length+1)/2);sizes.insert(sizes.begin(),length/2);}
        std::set<int> tried;
        for(int child_size:sizes) {
            if(child_size<2||child_size>length-2||!tried.insert(child_size).second||expired())continue;
            auto child=split_part(s,child_size,true);
            auto rear=escape(child,horizon);
            bool child_safe=rear.verified&&rear.depth==horizon&&attack_distance(child)>1;
            if(child_safe&&child_size==length-2)return child_size;
            auto parent=split_part(s,child_size,false);
            int parent_horizon=std::min(6,499-round); // The parent waits on its split turn.
            auto front=escape(parent,parent_horizon);
            bool parent_safe=front.verified&&front.depth==parent_horizon&&attack_distance(parent)>1;
            if(repeated&&std::abs(child_size-(length-child_size))<=1&&(child_safe||parent_safe))return child_size;
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
        if(!emergency&&(grower()||(count>=8&&(nearby_heads>=3||(nearby_heads>=2&&local_food<2)))||length<policy::split_length||length>6||count>=policy::max_team||round>=policy::split_before||round-last_split<policy::split_cooldown||attack_distance(s)<=1)) return false;
        if(!emergency) {
            auto parent=split_part(s,2,false);
            auto route=escape(parent,2);
            if(!route.verified||route.depth<2)return false;
        }
        return true;
    }
    bool opening_seed(State const& s) const {
        if(round!=0||birth_round!=0||complete||rescue_chain||length<8||count>8||count>=limit||s.body.size()<3)return false;
        int productive=0;
        for(auto const& c:cells)if(c.seen==round&&c.spawn>=round&&c.spawn<=round+6)productive++;
        if(productive<3||attack_distance(s)<=1)return false;
        State parent=s;parent.length=length-length/2;
        parent.body.resize(std::min(int(parent.body.size()),parent.length));
        // The tail is unknown: only certify the parent's visible continuation.
        // Child survival is deliberately not claimed for this opening gamble.
        for(int d=0;d<4;d++){State one;int hit;
            if(!step(parent,d,false,one,hit,true)||attack_distance(one)<=1)continue;
            for(int e=0;e<4;e++){State two;
                if(step(one,e,false,two,hit,true)&&attack_distance(two)>1)return true;
            }
        }
        return false;
    }
    Decision split_decision(State s,int child_size,bool rescue=false,bool seed=false) {
        bool release=seed||(rescue&&rescue_chain>0&&round<350&&child_size<=6);
        s.length=length-child_size;
        s.body.resize(std::min(int(s.body.size()),s.length));
        s.occupied.reset();for(int p:s.body)s.occupied.set(p);
        if(rescue&&length>=7&&s.length>=4)rescued_grower=true;
        if(rescue)rescue_chain=std::min(7,rescue_chain+1);
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
    int donation_move(State const& s) const {
        // Concentrate length only late, with a substantial surviving workforce.
        // The recipient must act later this round with exact length and a visible neck.
        if(feeding_simulation||round<350||count<(round<425?8:4)||length>(round<425?5:6)||
           (round<425&&grower())||!complete||(round<425&&round-last_food<4)||expired(.044))return -1;
        if(threat_distance[head]<=3)return -1;
        auto recipients=allies;
        std::stable_sort(recipients.begin(),recipients.end(),[&](auto a,auto b) {
            if(round<425&&(a.len<10)!=(b.len<10))return a.len<10;
            return a.len>b.len;
        });
        for(auto const& a:recipients) {
            if(expired(.044))return -1;
            if(!a.exact||a.id<=id||a.len<4||a.len<length+2||cells[a.pos].seen!=round||
               cells[a.pos].id!=a.id||!cells[a.pos].head)continue;
            int approach=-1;
            for(int d=0;d<4;d++)if(cells[a.pos].kind[d]==0&&next[a.pos][d]==head)approach=d;
            if(approach<0||threat_distance[a.pos]<=3)continue;
            // Do not add another corpse when the recipient already has nearby
            // food. This also stops several workers feeding it in the same round.
            bool fed=false;
            for(int p=0;p<n&&!fed;p++)if(cells[p].seen==round&&cells[p].pearl&&ally_route(a.pos,p,1))fed=true;
            if(fed)continue;
            std::vector<int> body{a.pos};Bits used;used.set(a.pos);
            while(int(body.size())<a.len) {
                int follower=-1;
                for(int p=0;p<n;p++)if(cells[p].seen==round&&cells[p].id==a.id&&!used[p]&&
                    next[p][cells[p].facing]==body.back()) {follower=p;break;}
                if(follower<0)break;
                body.push_back(follower);used.set(follower);
            }
            if(body.size()<2)continue;
            Brain recipient=*this;recipient.id=a.id;recipient.length=a.len;
            recipient.head=a.pos;recipient.predicted=body;recipient.complete=int(body.size())==a.len;
            recipient.heading=cells[a.pos].facing;recipient.rescued_grower=true;
            recipient.feeding_simulation=true;recipient.count=count-1;recipient.last_food=round;
            recipient.explore_distance.clear();recipient.farm_distance.clear();
            for(size_t k=0;k<s.body.size();k++) {
                auto& c=recipient.cells[s.body[k]];c.id=-1;c.team=-1;c.head=false;
                c.pearl=k%2==0;c.pearl_round=round;c.pearl_since=c.pearl?round:-1;
            }
            // An unseen tail is allowed, but its cells are never guessed empty:
            // restrict this donation check to currently observed destinations.
            for(auto& incoming:recipient.reverse)incoming.clear();
            for(int p=0;p<n;p++)for(int d=0;d<4;d++) {
                int& v=recipient.next[p][d];
                if(cells[p].seen!=round||v<0||cells[v].seen!=round)v=-1;
                else recipient.reverse[v].push_back(p);
            }
            recipient.prepare_goals();
            State meal;int hit;
            if(!recipient.step(recipient.initial(),approach,false,meal,hit,true))continue;
            int horizon=std::min(3,499-round);
            auto escape=recipient.escape(meal,horizon);
            if(!escape.verified||escape.depth!=horizon)continue;
            // A safe collection opportunity alone was insufficient in v4.
            // Require our recipient model to choose it and gain net length.
            auto chosen=recipient.choose();
            if(chosen.dying||chosen.split||chosen.path.empty()||
               recipient.next[a.pos][chosen.path[0]]!=head||chosen.after.length<=a.len)continue;
            // A one-step collision with our own verified neck kills only us,
            // leaving the head pearl exactly where the recipient can take it.
            for(int d=0;d<4;d++)if(next[head][d]==s.body[1])return d;
        }
        return -1;
    }
    Decision choose() {
        State s=initial();std::vector<Candidate> all,attacks;std::vector<int> path;
        enumerate(s,path,all,attacks);
        for(auto& c:all) c.base=c.score=root_score(c);
        bool sheltered=std::any_of(all.begin(),all.end(),[](auto const& c){return c.threat>1&&c.exits>0&&c.area>=6;});
        if(sheltered) all.erase(std::remove_if(all.begin(),all.end(),[](auto const& c){return c.threat<=1;}),all.end());
        std::sort(all.begin(),all.end(),[](auto const& a,auto const& b){return a.base>b.base;});
        if(all.size()>policy::candidate_limit) all.resize(policy::candidate_limit);
        for(auto& c:all) {
            if(expired(.036)) break;
            c.score+=lookahead(c);
            // Root attraction is a shortlist heuristic. Once a continuation is
            // scored, its terminal attraction replaces it. Adding both rewards
            // walking near a pearl now and eating it later over eating it now.
            if(c.depth>0)c.score-=attraction(c.state);
        }
        // Food must not outscore a route that survives when another branch has
        // actually exhausted all continuations inside the search horizon.
        if(std::any_of(all.begin(),all.end(),[](auto const& c){return !c.forced_dead&&c.depth>0;}))
            all.erase(std::remove_if(all.begin(),all.end(),[](auto const& c){return c.forced_dead;}),all.end());
        std::sort(all.begin(),all.end(),[](auto const& a,auto const& b){return a.score>b.score;});
        bool trapped=all.empty() || all[0].forced_dead || (all[0].exits==0 && all[0].area<=2) || all[0].threat==1;
        // Compare targets by visible lower bounds, preferring the largest trade.
        Candidate const* trade=nullptr;
        for(auto const& a:attacks) if(valuable_trade(enemy_size[a.kill])&&
            (!trade||enemy_size[a.kill]>enemy_size[trade->kill])) trade=&a;
        if(trade) return {trade->path,0,s,false,true};
        if(opening_seed(s))return split_decision(s,length/2,true,true);
        if(trapped) {
            int child=rescue_split(s);
            if(child) return split_decision(s,child,true);
        } else if(safe_split(s,false)) return split_decision(s,2);
        // No automatic opening cut: fresh children with useful moves must not
        // repeatedly reverse and shed two more segments in this same round.
        if(!complete&&length>=4&&count<limit&&s.body.size()>=2 &&
           (all.empty()||(!all.empty()&&all[0].forced_dead))) {
            int child=rescue_chain>0&&round<350&&length>=8?length/2:length-2;
            return split_decision(s,child,true);
        }
        int donation=donation_move(s);
        if(donation>=0)return {{donation},0,s,false,true};
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
            if(d.protect_child_length) {
                int role=d.protect_child_length|(std::min(7,rescue_chain)<<12)|(d.release_child?32768:0);
                auto msg=packet(predicted[0],1,role);
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
                if(probe>=0) {
                    probe_portal=cells[predicted[0]].portal[probe];probe_round=round;
                    ct.send_sonar(unswbc::Direction(LETTERS[probe]),packet(predicted[0],0,probe_portal));
                } else for(int k=0;k<4;k++) {
                    auto value=(busy>=0&&(k&1)==((round+id)&1))?packet(predicted[0],0,busy):msg;
                    ct.send_sonar(unswbc::Direction(LETTERS[k]),value);
                }
            }
        } else predicted.clear();
    }
};
}
