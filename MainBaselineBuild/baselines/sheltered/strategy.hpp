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
    bool pearl=false;
    int id=-1, team=-1, facing=0;
    bool head=false;
    std::array<int,4> kind{{-1,-1,-1,-1}}, portal{{-1,-1,-1,-1}};
};
struct Goal { int pos; double reward; std::vector<int> distance; };
struct Ally { int id,pos,len,round; };
struct State {
    std::vector<int> body;
    Bits occupied, eaten;
    double reward=0;
    int heading=0;
};
struct Candidate {
    State state;
    std::vector<int> path;
    int kill=-1;
    double base=-1e9, score=-1e9;
    int area=0, exits=0, threat=99, depth=0;
};
struct Decision { std::vector<int> path; int split=0; State after; bool exact=false, dying=false; };

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
    std::map<int,int> enemy_size;
    std::vector<int> enemy_heads, threat_sources;
    std::vector<int> predicted;
    bool complete=false;
    int last_split=-1000;
    std::chrono::steady_clock::time_point started;
    Brain(int width,int height,int dragon,int side):w(width),h(height),n(w*h),id(dragon),team(side),cells(n),next(n),adjacent(n),reverse(n),last_visit(n,-1000),threat_distance(n,99) {
        for(int p=0;p<n;p++) {
            int x=p%w,y=p/w;
            adjacent[p]={{((y+h-1)%h)*w+x,y*w+(x+1)%w,((y+1)%h)*w+x,y*w+(x+w-1)%w}};
        }
    }
    int pos(unswbc::Position p) const { return p.y*w+p.x; }
    int distance(int a,int b) const { int x=std::abs(a%w-b%w),y=std::abs(a/w-b/w); return std::min(x,w-x)+std::min(y,h-y); }
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
    std::uint64_t packet(int p,int len) const {
        std::uint64_t v=std::uint64_t(p%w) | (std::uint64_t(p/w)<<6) | (std::uint64_t(std::min(len,4095))<<12)
            | (std::uint64_t(round)<<24) | (std::uint64_t(id&65535)<<33) | (std::uint64_t(team)<<49);
        return v | (std::uint64_t(checksum(v))<<50);
    }
    void messages(const std::vector<std::uint64_t>& inbox) {
        allies.erase(std::remove_if(allies.begin(),allies.end(),[&](auto a){return round-a.round>2;}),allies.end());
        for(auto m:inbox) {
            auto v=m&((UINT64_C(1)<<50)-1);
            if(checksum(v)!=(m>>50) || int((v>>49)&1)!=team) continue;
            int x=v&63,y=(v>>6)&63,l=(v>>12)&4095,r=(v>>24)&511,i=(v>>33)&65535;
            if(x>=w||y>=h||l<2||i==id||r>round||round-r>2) continue;
            int p=y*w+x;
            if(cells[p].seen==round && (cells[p].id!=i || !cells[p].head || cells[p].team!=team)) continue;
            auto it=std::find_if(allies.begin(),allies.end(),[&](auto a){return a.id==i;});
            Ally a{i,p,l,r}; if(it==allies.end()) allies.push_back(a); else if(r>=it->round) *it=a;
        }
    }
    void observe(unswbc::Controller const& ct,unswbc::Game const& game) {
        started=std::chrono::steady_clock::now();
        round=game.get_round_num(); length=ct.get_length(); count=ct.get_unit_count(); limit=game.get_unit_limit();
        head=pos(ct.get_position()); heading=direction(ct.get_dir().value); enemy_heads.clear(); enemy_size.clear();
        for(auto const& tile:ct.get_tiles()) {
            int p=pos(tile.get_position()); auto& c=cells[p]; c.seen=round; c.pearl=tile.has_pearl(); c.pearl_round=round;
            c.spawn=tile.get_pearl_time()<0?-1:round+tile.get_pearl_time(); c.id=-1; c.head=false; c.team=-1;
            if(auto part=tile.get_dragon()) {
                c.id=part->get_id(); c.team=part->get_team().value=='A'?0:1; c.head=part->is_head(); c.facing=direction(part->get_dir().value);
                if(c.team!=team) { enemy_size[c.id]++; if(c.head) enemy_heads.push_back(p); }
            }
            for(int d=0;d<4;d++) { auto const& e=tile.get_edge(unswbc::Direction(LETTERS[d])); set_edge(p,d,int(e.get_edge_type()),e.get_portal_id()); }
        }
        transitions(); messages(ct.get_sonar_messages());
        threat_sources=enemy_heads;
        // A tail can become a new head and attack in this same round.
        Bits has_follower;
        for(int p=0;p<n;p++) if(cells[p].seen==round&&cells[p].id>=0&&cells[p].team!=team&&!cells[p].head) {
            int q=next[p][cells[p].facing];if(q>=0)has_follower.set(q);
        }
        for(int p=0;p<n;p++) if(cells[p].seen==round&&cells[p].id>=0&&cells[p].team!=team&&!cells[p].head&&!has_follower[p]) {
            if(enemy_size[cells[p].id]>=4) threat_sources.push_back(p);
        }
        complete=int(predicted.size())==length && !predicted.empty() && predicted[0]==head;
        if(complete) {
            for(int p:predicted) if(cells[p].seen==round && cells[p].id!=id) {complete=false;break;}
        }
        if(!complete) {
            predicted={head}; Bits used; used.set(head);
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
        prepare_goals();
    }
    State initial() const {
        State s; s.body=predicted;s.heading=heading;
        for(int p:s.body) s.occupied.set(p);
        if(!complete) for(int p=0;p<n;p++) if(cells[p].seen==round&&cells[p].id==id) s.occupied.set(p);
        return s;
    }
    bool external_block(int p) const {
        return cells[p].id>=0 && cells[p].id!=id && round-cells[p].seen<=1;
    }
    bool step(State const& before,int d,bool extra,State& after,int& hit,bool current=true) const {
        hit=-1;
        int actual_len=complete?int(before.body.size()):length;
        if(extra&&actual_len<=2) return false;
        int dest=next[before.body[0]][d];
        if(dest<0 || (current&&cells[dest].seen!=round)) return false;
        if(before.occupied[dest]) return false;
        if(external_block(dest)) {
            if(cells[dest].seen==round&&cells[dest].head&&cells[dest].team!=team) hit=cells[dest].id;
            return false;
        }
        after=before; after.heading=d;
        bool food=cells[dest].pearl && round-cells[dest].pearl_round<=15 && !before.eaten[dest];
        after.body.insert(after.body.begin(),dest);after.occupied.set(dest);
        if(food) {after.eaten.set(dest);after.reward+=policy::food_reward;}
        else if(complete) {after.occupied.reset(after.body.back());after.body.pop_back();}
        if(extra && complete) {after.occupied.reset(after.body.back());after.body.pop_back();}
        if(extra) after.reward-=(round>=450?22.0:policy::sprint_cost);
        return true;
    }
    void prepare_goals() {
        goals.clear();
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
            std::vector<int> q;q.reserve(n);q.push_back(p);
            for(size_t i=0;i<q.size();i++) for(int v:reverse[q[i]]) if(g.distance[v]==INF) {g.distance[v]=g.distance[q[i]]+1;q.push_back(v);}
            goals.push_back(std::move(g));
        }
    }
    double attraction(State const& s) const {
        double best=0;
        for(auto const& g:goals) if(!s.eaten[g.pos] && g.distance[s.body[0]]<INF) {
            double penalty=0;
            for(auto a:allies) if(a.len>length && distance(a.pos,g.pos)<g.distance[s.body[0]]) penalty=2;
            best=std::max(best,9*g.reward/(1+.38*g.distance[s.body[0]]+penalty));
        }
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
    double danger(int d) const {return d==1?policy::danger_one:d==2?policy::danger_two:d==3?policy::danger_three:0;}
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
    bool expired() const {return std::chrono::duration<double>(std::chrono::steady_clock::now()-started).count()>policy::search_seconds;}
    void enumerate(State const& s,std::vector<int>& path,std::vector<Candidate>& all,std::vector<Candidate>& attacks) const {
        for(int d=0;d<4;d++) {
            State t;int hit;
            bool ok=step(s,d,!path.empty(),t,hit,true);
            path.push_back(d);
            if(ok) {
                Candidate c;c.state=std::move(t);c.path=path;all.push_back(c);
                if(int(path.size())<policy::max_sprint&&complete) enumerate(c.state,path,all,attacks);
            } else if(hit>=0) {Candidate c;c.path=path;c.kill=hit;attacks.push_back(std::move(c));}
            path.pop_back();
        }
    }
    double lookahead(Candidate& c) const {
        std::vector<State> beam{c.state}; double best=-1000;
        int max_depth=std::min(policy::lookahead,499-round);
        for(int depth=1;depth<=max_depth;depth++) {
            if(expired()) break;
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
            if(children.empty()) {c.depth=depth-1;return -200+12*depth;}
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
    bool safe_split(State const& s,bool emergency) const {
        if(!complete||length<4||count>=limit) return false;
        State child;child.body={s.body.back(),s.body[s.body.size()-2]};
        // Retain all existing body segments as obstacles for the newborn's first step.
        child.occupied=s.occupied;
        int options=mobility(child);auto [room,frontier]=space(child,50);
        if(options<1||room<(emergency?4:8)||attack_distance(child)<=1) return false;
        if(!emergency&&(length<policy::split_length||count>=policy::max_team||round>=policy::split_before||round-last_split<policy::split_cooldown||distance(s.body.front(),s.body.back())<2||attack_distance(s)<=1)) return false;
        return true;
    }
    Decision choose() {
        State s=initial();std::vector<Candidate> all,attacks;std::vector<int> path;
        enumerate(s,path,all,attacks);
        for(auto& c:all) c.base=c.score=root_score(c);
        bool sheltered=std::any_of(all.begin(),all.end(),[](auto const& c){return c.threat>3&&c.exits>0&&c.area>=6;});
        if(sheltered) all.erase(std::remove_if(all.begin(),all.end(),[](auto const& c){return c.threat<=3;}),all.end());
        std::sort(all.begin(),all.end(),[](auto const& a,auto const& b){return a.base>b.base;});
        if(all.size()>policy::candidate_limit) all.resize(policy::candidate_limit);
        for(auto& c:all) {
            if(expired()) break;
            c.score+=lookahead(c);
        }
        std::sort(all.begin(),all.end(),[](auto const& a,auto const& b){return a.score>b.score;});
        bool trapped=all.empty() || (all[0].exits==0 && all[0].area<=2) || all[0].threat==1;
        // A guaranteed head trade is reserved for a small expendable dragon.
        for(auto const& a:attacks) {
            auto it=enemy_size.find(a.kill);int enemy=it==enemy_size.end()?0:it->second;
            if((count>1&&length<=5&&enemy>=length)||(trapped&&count==1)) return {a.path,0,s,false,true};
        }
        if(safe_split(s,trapped)) {
            s.body.resize(s.body.size()-2);s.occupied.reset();for(int p:s.body)s.occupied.set(p);
            last_split=round;return {{},2,s,true,false};
        }
        if(!all.empty()) return {all[0].path,0,all[0].state,complete,false};
        // With no known safe option, an unobserved portal exit is preferable to certain collision.
        for(int d=0;d<4;d++) if(cells[head].kind[d]==2) {
            int v=next[head][d];if(v<0||cells[v].seen!=round) return {{d},0,s,false,false};
        }
        return {{heading},0,s,false,true};
    }
    void commit(Decision const& d,unswbc::Controller& ct) {
        if(d.split) ct.do_split(d.split);
        else {std::vector<unswbc::Direction> dirs;for(int v:d.path)dirs.emplace_back(LETTERS[v]);ct.make_moves(dirs);}
        if(d.exact&&!d.dying) {
            predicted=d.after.body;
            if(count>1||d.split) {
                auto msg=packet(predicted[0],int(predicted.size()));
                int first=(round+id)%4;
                ct.send_sonar(unswbc::Direction(LETTERS[first]),msg);
                ct.send_sonar(unswbc::Direction(LETTERS[first^2]),msg);
            }
        } else predicted.clear();
    }
};
}
