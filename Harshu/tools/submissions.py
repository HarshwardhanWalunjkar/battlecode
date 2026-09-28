#!/usr/bin/env python3
"""Package, track uploads, and distinguish upload time from Elo eligibility."""
import argparse, datetime as dt, hashlib, json, pathlib, sys, zipfile, io, fcntl
from zoneinfo import ZoneInfo
from importlib.metadata import version
ROOT=pathlib.Path(__file__).resolve().parents[1]
HISTORY=ROOT/'submissions/history.json'
UTC=dt.timezone.utc

def now():return dt.datetime.now(UTC)
def parse_time(s):
    d=dt.datetime.fromisoformat(s.replace('Z','+00:00'))
    if d.tzinfo is None:raise ValueError('Include a timezone, e.g. 2026-09-27T18:30:00+05:30')
    return d.astimezone(UTC)
def show(d):return d.astimezone(ZoneInfo('Asia/Kolkata')).strftime('%Y-%m-%d %H:%M:%S IST')+' / '+d.strftime('%Y-%m-%d %H:%M:%S UTC')
def load():return json.loads(HISTORY.read_text())
def save(h):
    temp=HISTORY.with_suffix('.tmp');temp.write_text(json.dumps(h,indent=2)+'\n');temp.replace(HISTORY)
def package():
    from unswbc.project import Project
    p=Project.from_dir(ROOT/'bot');p.collect_sources()
    names=sorted(set(['bot.toml',*p.sources]));digest=hashlib.sha256();out=io.BytesIO()
    with zipfile.ZipFile(out,'w',zipfile.ZIP_DEFLATED) as z:
        for name in names:
            data=(p.path/name).read_bytes();digest.update(name.encode()+b'\0'+data+b'\0')
            info=zipfile.ZipInfo(name,(2026,1,1,0,0,0));info.compress_type=zipfile.ZIP_DEFLATED;z.writestr(info,data)
    blob=out.getvalue()
    if len(blob)>4*1024*1024:raise ValueError('ZIP exceeds 4 MB')
    return blob,digest.hexdigest(),names

def advice(h,identity,at):
    uploads=[u for u in h['uploads'] if u['status']=='accepted']
    recent=[u for u in uploads if at-dt.timedelta(hours=1)<parse_time(u['uploaded_at'])<=at]
    lines=[f'Checked: {show(at)}',f'Accepted uploads recorded in last hour: {len(recent)}/12.']
    if not h.get('history_known_from_account_creation'):lines.append('Account history is incomplete: server eligibility and remaining quota are not confirmed.')
    if len(recent)>=12:
        free=min(parse_time(u['uploaded_at']) for u in recent)+dt.timedelta(hours=1)
        lines.append('Local hourly quota is full. Earliest recorded slot: '+show(free))
    windows=[parse_time(b['played_at']) for b in h['ranked_battles'] if b.get('fresh_window')]
    same=[u for u in uploads if u['fingerprint']==identity]
    counts=[]
    for u in same:
        battles=[b for b in h['ranked_battles'] if str(b['version'])==str(u['version'])]
        if battles:counts.append(max(b['count_after'] for b in battles))
    if counts:lines.append(f'Identical code previously played ranked; highest recorded effective count is {max(counts)}. Re-upload does not reset it.')
    elif windows:
        deadline=max(windows)+dt.timedelta(hours=12)
        if at<deadline:lines.append('Fresh-code high-K window is still occupied until '+show(deadline)+'. A stronger bot may still be worth submitting now, with inherited K.')
        else:lines.append('Recorded 12-hour window has expired. Different code may qualify at its first ranked battle; confirm no unrecorded activity.')
    elif h.get('history_known_from_account_creation') and not uploads and not h['ranked_battles']:lines.append('Confirmed new team: no earlier ranked play. The first eligible bot starts its 12-hour window when it first plays ranked, not on upload.')
    else:lines.append('No confirmed first-ranked fresh-window time. Upload time cannot establish the 12-hour deadline.')
    lines.append('High K magnifies losses as well as wins; use it for a tested improvement, not a cosmetic change.')
    return lines

def main():
    release=json.loads((ROOT/'versions/current.json').read_text())
    p=argparse.ArgumentParser();sub=p.add_subparsers(dest='cmd',required=True)
    sub.add_parser('status');sub.add_parser('package')
    s=sub.add_parser('submit');s.add_argument('--name',default=release['name']);s.add_argument('--description',default=release['description'])
    r=sub.add_parser('record-ranked');r.add_argument('--version',required=True);r.add_argument('--battle-id',required=True);r.add_argument('--time',required=True);r.add_argument('--count-before',type=int,required=True);r.add_argument('--fresh-window',action='store_true')
    sub.add_parser('sync')
    a=p.parse_args();blob,identity,names=package()
    with (ROOT/'submissions/.lock').open('w') as lock:
        fcntl.flock(lock,fcntl.LOCK_EX);h=load()
        if a.cmd=='status':print('\n'.join(advice(h,identity,now())));return
        if a.cmd=='package':
            out=ROOT/'dist'/f"{release['name']}.zip";out.parent.mkdir(exist_ok=True)
            if out.exists() and out.read_bytes()!=blob:
                raise ValueError('A different build already uses this version name. Advance versions/current.json first.')
            out.write_bytes(blob)
            manifest={'release':release['release'],'name':release['name'],'fingerprint':identity,'files':names,'bytes':len(blob),'toolkit':version('unswbc')}
            (ROOT/'dist'/f"{release['name']}.manifest.json").write_text(json.dumps(manifest,indent=2)+'\n')
            (ROOT/'dist/manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
            print(f'{out}\n{len(blob)} bytes; fingerprint {identity}');return
        if a.cmd=='record-ranked':
            if a.count_before<0 or (a.fresh_window and a.count_before!=0):raise ValueError('Fresh-window must start at count zero; counts must be nonnegative')
            if any(str(b['battle_id'])==a.battle_id for b in h['ranked_battles']):raise ValueError('Battle already recorded')
            t=parse_time(a.time)
            if t>now():raise ValueError('A played battle cannot be in the future')
            h['ranked_battles'].append({'version':a.version,'battle_id':a.battle_id,'played_at':t.isoformat(),'count_before':a.count_before,'count_after':a.count_before+1,'fresh_window':a.fresh_window})
            save(h);print('\n'.join(advice(h,identity,now())));return
        from unswbc import api,auth
        key=auth.key()
        if not key:raise ValueError('No API key configured. Run .venv/bin/unswbc auth set YOUR_KEY locally.')
        if a.cmd=='sync':
            snapshot={'fetched_at':now().isoformat(),'submissions':api.request('submissions',key=key),'battles':api.request('battles?limit=200',key=key)}
            (ROOT/'submissions/server-snapshot.json').write_text(json.dumps(snapshot,indent=2)+'\n')
            print('Saved server records. First-ranked count/window still needs verification; raw fields are not guessed.');return
        print('\n'.join(advice(h,identity,now())))
        recent=[u for u in h['uploads'] if u['status']=='accepted' and parse_time(u['uploaded_at'])>now()-dt.timedelta(hours=1)]
        if len(recent)>=12:raise ValueError('Local hourly upload quota is full')
        # Freeze the exact uploaded bytes before any network request.
        stamp=now();release=ROOT/'submissions'/f'{stamp:%Y%m%dT%H%M%SZ}-{identity[:12]}.zip';release.write_bytes(blob)
        record={'uploaded_at':stamp.isoformat(),'fingerprint':identity,'name':a.name,'version':None,'status':'attempting','archive':release.name}
        h['uploads'].append(record);save(h)
        from unswbc.submit import _multipart
        body,content_type=_multipart({'name':a.name,'language':'cpp','description':a.description},release.name,blob)
        try:
            response=api.request('submissions',key=key,method='POST',body=body,content_type=content_type)
        except api.ApiError as error:
            record['status']='rejected' if error.status else 'unknown';record['error']=str(error);save(h)
            raise ValueError('Upload did not return success. Check server before retrying; timing record retained.') from error
        record.update(status='accepted',version=response.get('version'),server_response=response);save(h)
        print(f"Submitted version {record['version']}; upload recorded at {show(stamp)}. First ranked play is not yet known.")
if __name__=='__main__':
    try:main()
    except (ValueError,OSError) as e:print(str(e),file=sys.stderr);sys.exit(1)
