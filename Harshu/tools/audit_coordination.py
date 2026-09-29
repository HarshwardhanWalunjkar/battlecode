"""Link actual v7 acknowledgements to donor deaths and recipient pearl pickups.

Only sonar rays that hit the intended donor count as delivered agreements.
Other corpse meals are not labelled deliberate. Also runs the independent
body/snapshot and phase audit. Usage: audit_coordination.py results.json
"""
import json
import sys
from pathlib import Path
from reassess_v6 import audit, Board

def checksum(value):
    value ^= value >> 23
    value = (value * 0x9e3779b97f4a7c15) & ((1 << 64)-1)
    value ^= value >> 31
    return value & 16383

def inspect(path, side):
    audited=audit(path)
    replay=json.loads(path.read_text());board=Board(replay['map'])
    heads={i:body[0] for i,body in board.bodies.items()}
    acknowledgements={};deaths=[];round=-1
    for e in replay['events']:
        kind=e['type']
        if kind=='roundStart': round=e['round']
        elif kind=='dragonUpdate': heads[e['id']]=board.pos(e['head'])
        elif kind=='dragonSplit':
            heads[e['parentId']]=board.pos(e['parentBody'][0]);heads[e['childId']]=board.pos(e['childBody'][0])
        elif kind=='sonarPing':
            value=int(e['value']);payload=value & ((1<<50)-1)
            if checksum(payload) ^ (value>>50) != 0x2b19: continue
            donor=(payload>>33)&65535
            if e.get('hitId')!=donor or not e['hitKind'].startswith('ally'):continue
            team='AB'[(payload>>49)&1]
            if team!=side:continue
            food=(payload & 63)+((payload>>6)&63)*board.w
            key=(round,e['senderId'],donor,food)
            acknowledgements[key]={'round':round,'collector':e['senderId'],'donor':donor,'food':food}
        elif kind=='dragonDeath':
            deaths.append({'id':e['id'],'head':heads[e['id']],'round':round,'reason':e['reason']})
            del heads[e['id']]
    links=[]
    for ack in acknowledgements.values():
        death=next((d for d in deaths if d['id']==ack['donor'] and d['head']==ack['food'] and
            d['reason']=='S' and ack['round']<=d['round']<=ack['round']+1),None)
        meals=[]
        if death:
            meals=[m for m in audited['corpse_meals'] if m['donor']==ack['donor'] and
                m['death_r']==death['round'] and m['recipient']==ack['collector'] and
                m['cell']==ack['food'] and m['r']<=ack['round']+2]
        links.append({**ack,'donation_death':death,'actual_head_pickups':meals})
    return {'replay':path.name,'side':side,'snapshot_checks':audited['checks'],
        'result':audited['result'],'phase_metrics':audited['phase_metrics'],
        'checkpoints':audited['checkpoints'],'delivered_agreements':links,
        'delivered_count':len(links),'donation_count':sum(x['donation_death'] is not None for x in links),
        'completed_count':sum(bool(x['actual_head_pickups']) for x in links),
        'deaths':audited['deaths']}

def main():
    results=Path(sys.argv[1]);data=json.loads(results.read_text());out=[]
    folder=results.parent/(results.stem+'-replays')
    for i,record in enumerate(data['records']):
        row=inspect(folder/f"{i:04d}-{record['map']}.events.json",record['bot_side'])
        out.append(row)
        print(record['map'],record['bot_side'],'agreed',row['delivered_count'],
            'donated',row['donation_count'],'collected',row['completed_count'],flush=True)
    target=results.with_name(results.stem+'-audit.json')
    target.write_text(json.dumps({'fingerprints':data['fingerprints'],'records':out},indent=2)+'\n')

if __name__=='__main__': main()
