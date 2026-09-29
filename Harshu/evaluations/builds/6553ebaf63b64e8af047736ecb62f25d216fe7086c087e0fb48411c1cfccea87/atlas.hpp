#pragma once
#include "map_catalog.hpp"
#include <algorithm>
namespace abyss::atlas {
// Predictions never enter the collision map. A rejection is irreversible for
// this process and is inherited by children, including newly introduced maps.
struct Knowledge {
    int candidate=0,mode=0; // 0 unknown, 1..10 checked catalogue, 15 general lock
    bool initialized=false,educated=false;
    const Map* map() const {return mode>=1&&mode<=int(catalog.size())?catalog[mode-1]:nullptr;}
    void reject(){mode=15;candidate=0;}
    void receive(int code,int rev,bool birth) {
        if(rev!=revision||code<0||(code>int(catalog.size())&&code!=15))return;
        if(birth)educated=true;
        if(code==15){reject();return;}
        if(mode==15||code==0)return;
        if(candidate&&candidate!=code){reject();return;}
        candidate=code;
    }
    template<class B> bool compatible(B const& b,Map const& m,int& evidence,int& landmarks) const {
        if(b.w!=m.w||b.h!=m.h)return false;
        evidence=landmarks=0;
        for(int p=0;p<b.n;p++) {
            auto const& c=b.cells[p];
            for(int d:{0,3})if(c.kind[d]>=0) {
                int expected=m.edges[2*p+(d==3)];
                int actual=c.kind[d]==2?c.portal[d]+2:c.kind[d];
                if(actual!=expected)return false;
                evidence++;landmarks+=expected!=0;
            }
            if(c.seen>=0) {
                if((c.spawn>=0)!=(m.hi[p]>0))return false;
                if(c.spawn>=0&&c.spawn-c.seen>m.hi[p])return false;
                landmarks+=m.hi[p]==0;
            }
        }
        return true;
    }
    template<class B> void update(B const& b) {
        if(mode==15)return;
        if(!initialized) {
            initialized=true;
            if(!educated&&b.birth_round==0&&b.round==0) {
                int matches=0;
                for(int code=1;code<=int(catalog.size());code++) {
                    auto const& m=*catalog[code-1];
                    if(b.w!=m.w||b.h!=m.h||b.id>=int(m.starts.size()))continue;
                    auto s=m.starts[b.id];
                    if(s.length!=b.length||s.heading!=b.heading||m.body[s.offset]!=b.head)continue;
                    bool own_ok=true;
                    for(int p=0;p<b.n;p++)if(b.cells[p].seen==0&&b.cells[p].id==b.id)
                        own_ok&=std::find(m.body.begin()+s.offset,m.body.begin()+s.offset+s.length,p)!=m.body.begin()+s.offset+s.length;
                    for(int j=0;j<s.length;j++) {int p=m.body[s.offset+j];
                        if(b.cells[p].seen==0)own_ok&=b.cells[p].id==b.id;
                    }
                    int evidence=0,landmarks=0;
                    if(own_ok&&compatible(b,m,evidence,landmarks)){candidate=code;matches++;}
                }
                if(matches!=1){reject();return;}
                // Spawn position uniquely identified the map - trust it for navigation immediately
                mode=candidate;
            } else if(!candidate) {
                // A child with no received birth packet must never identify a
                // map from its incidental head location or dimensions.
                return;
            }
        }
        if(!candidate)return;
        int evidence=0,landmarks=0;
        if(!compatible(b,*catalog[candidate-1],evidence,landmarks)){reject();return;}
        if(evidence>=64&&landmarks>=8)mode=candidate;
    }
};
}
