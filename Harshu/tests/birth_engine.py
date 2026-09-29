"""Verify actual split-sonar delivery, including a case no ray can reach.
The fixture emits the release birth envelope; C++ tests validate its decoding.
"""
import json,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from audit_coordination import checksum
from unswbc.engine import EngineModule
ROOT=Path(__file__).resolve().parents[1]
class Done(Exception):pass

def run(body,edges=()):
 def coords(xs):return ' '.join(f'{p%11} {p//11}' for p in xs)
 raw=('MAP 11 11\nTILE_COUNT 0\nEDGE_COUNT '+str(len(edges))+'\n'+''.join(f'EDGE {i} 2 17\n' for i in edges)+f'DRAGON_COUNT 2\nDRAGON 0 {len(body)} {coords(body)}\nDRAGON 1 3 2 1 1 1 0 1\n').encode()
 # Explicit fallback mode, revision matching this release, role not protected.
 revision=143
 value=2|(15<<12)|(revision<<30);packet=value|((checksum(value)^0x1a97)<<50)
 child=None
 def reply(i,data):
  nonlocal child
  r=int(data.splitlines()[0].split()[1])
  if r>0:raise Done()
  if i==0:return ('SPLIT 2\n'+''.join(f'SONAR {d} {packet}\n' for d in 'NESW')+'PROTOCOL 3\nENDTURN\n').encode()
  if i==2:child=data.decode()
  return b'MOVE S\nPROTOCOL 3\nENDTURN\n'
 try:EngineModule().run(raw,reply,lambda *_:None,seed=4,on_notice=lambda _:None)
 except Done:pass
 assert child is not None,'Child must act this same round'
 return {'body':body,'portal_edges':edges,'received':str(packet) in child,'same_round':child.startswith('ROUND 0\n')}
if __name__=='__main__':
 rows=[run([60,59,58,57]),run([60,59,48,47]),run([60,59,48,49]),run([60,59,90,89],[136,207])]
 assert [x['received'] for x in rows]==[True,False,True,True],rows
 (ROOT/'evaluations/v8-review/birth-engine.json').write_text(json.dumps(rows,indent=2)+'\n')
 print('Straight, bent-unreachable, coiled and portal birth delivery contracts passed')
