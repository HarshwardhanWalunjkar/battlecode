#!/usr/bin/env python3
"""Create frozen evaluation opponents. Never overwrite the submission bot."""
from pathlib import Path
import shutil
ROOT=Path(__file__).resolve().parents[1]
variants={'no_split':{'max_team':1},'early_split':{'split_length':6,'max_team':3},'late_split':{'split_length':20},'wide_search':{'beam_width':14,'lookahead':14},'cautious':{'danger_two':90.0,'danger_three':25.0},'bold':{'danger_two':15.0,'danger_three':2.0}}
for name,changes in variants.items():
    out=ROOT/'baselines'/name;out.mkdir(exist_ok=True)
    for f in ('main.cpp','helper.hpp','strategy.hpp','config.hpp','bot.toml'):shutil.copyfile(ROOT/'bot'/f,out/f)
    import re
    cfg=(out/'config.hpp').read_text()
    for key,value in changes.items():cfg=re.sub(rf'({key} = )[^;]+;',rf'\g<1>{value};',cfg)
    (out/'config.hpp').write_text(cfg)
# Greedy and hunter share only observations/legality with the candidate, not search.
for mode in ('greedy','hunter'):
    out=ROOT/'baselines'/mode;out.mkdir(exist_ok=True)
    for f in ('helper.hpp','strategy.hpp','config.hpp','bot.toml'):shutil.copyfile(ROOT/'bot'/f,out/f)
    (out/'main.cpp').write_text('''#include "strategy.hpp"
int main(){try{auto [ct,g]=unswbc::init();abyss::Brain b(g.width,g.height,ct.get_id(),ct.get_team().value=='A'?0:1);
while(unswbc::update(ct,g)){b.observe(ct,g);auto s=b.initial();std::vector<abyss::Candidate> c,a;std::vector<int> p;b.enumerate(s,p,c,a);
abyss::Decision decision;double best=-1e20;
for(auto &v:c){double score=v.state.reward+b.attraction(v.state);auto [room,frontier]=b.space(v.state,100);score+=2*std::log(1+room);if(b.mobility(v.state)==0)score-=200;
''' + ('score-=b.danger(b.attack_distance(v.state))*.4;\n' if mode=='greedy' else 'for(int h:b.enemy_heads)score+=12.0/(1+b.distance(v.state.body[0],h));\n') + '''if(score>best){best=score;decision={v.path,0,v.state,b.complete,false};}}
''' + ('''if(!a.empty()&&(b.count>1||c.empty()))decision={a[0].path,0,s,false,true};
if(b.count<4&&b.length>=6&&b.safe_split(s,true)){s.body.resize(s.body.size()-2);decision={{},2,s,true,false};}
''' if mode=='hunter' else '') + '''if(decision.path.empty()&&!decision.split)decision={{b.heading},0,s,false,true};b.commit(decision,ct);unswbc::end_turn();}
}catch(std::exception const&){return std::cin.eof()?0:1;}}
''')
print('Created',len(variants)+2,'frozen opponents')
