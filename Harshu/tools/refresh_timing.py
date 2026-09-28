"""Reconcile ranked-series counts from a saved API snapshot and archived details."""
from collections import Counter
import datetime as dt
import json
from pathlib import Path
from zoneinfo import ZoneInfo

root = Path(__file__).resolve().parents[1]
audit = root / 'evaluations/recent-20260928'
snapshot = json.loads((root/'submissions/server-snapshot.json').read_text())
history = json.loads((root/'submissions/history.json').read_text())
submissions = {s['id']:s for s in snapshot['submissions']}
counts = {s['version']:s['kStart'] for s in submissions.values() if s['kStart'] is not None}
ranked = []
pending = []
for battle in sorted(snapshot['battles'], key=lambda b:b['at']):
    if not battle['ranked']:
        continue
    details = json.loads((audit/f"{battle['id']}.json").read_text())
    match = details['match'];side = 'A' if match['teamAId']==422 else 'B'
    submission = submissions[match['submission'+side+'Id']]
    version = submission['version']
    if any(r == 'pending' for r in battle['results']) or battle['outcome'] in ('queued','live'):
        pending.append({'battle_id':battle['id'],'version':version,'opponent':battle['opponent']})
        continue
    if version not in counts:
        raise RuntimeError('Completed ranked version has no server starting count')
    count = counts[version];counts[version] += 1
    fresh = count == 0 and submission['kFreshAt'] is not None
    starts = []
    for game in details['games']:
        path = audit/f"{game['id']}.json"
        if path.exists():
            start = json.loads(path.read_text())['match'].get('startedAt')
            if start:starts.append(start)
    ranked.append({'version':version,'submission_id':submission['id'],'battle_id':battle['id'],
                   'series_id':match['seriesId'], 'played_at':submission['kFreshAt'] if fresh else battle['at'],
                   'series_completed_at':battle['at'],'count_before':count,'count_after':count+1,
                   'fresh_window':fresh,'opponent':battle['opponent'], 'wins':battle['wins'],
                   'losses':battle['losses'],'draws':battle['results'].count('draw'),
                   'elo_change':battle['eloChange'],'game_ids':[g['id'] for g in details['games']],
                   'first_game_started_at':min(starts) if starts else None,
                   'evidence':f"evaluations/recent-20260928/{battle['id']}.json"})
history['ranked_battles'] = ranked
active = next(s for s in submissions.values() if s['status']=='active')
history['latest_server_verification'] = {'at':snapshot['fetched_at'], 'active_submission_id':active['id'],
    'active_version':active['version'],'kStart':active['kStart'],'kFreshAt':active['kFreshAt'],
    'completed_ranked_series':len(ranked),'queued_ranked_series':pending,
    'effective_counts':{str(k):v for k,v in counts.items()},
    'effective_count_basis':'Server kStart plus completed ranked series; live/queued series excluded.',
    'snapshot':'evaluations/recent-20260928/snapshot.json'}
for upload in history['uploads']:
    s = next((s for s in submissions.values() if s['version']==upload['version']), None)
    if s:upload['latest_server_state'] = {k:s[k] for k in ('id','version','status','kStart','kFreshAt')}
history['notes'] = 'Complete account history confirmed by user; uploads and ranked series reconciled from API. v3 is a local candidate, not uploaded.'
(root/'submissions/history.json').write_text(json.dumps(history,indent=2)+'\n')
fresh_at = max(s['kFreshAt'] for s in submissions.values() if s['kFreshAt'])
deadline = dt.datetime.fromisoformat(fresh_at.replace('Z','+00:00'))+dt.timedelta(hours=12)
rows = []
for version,count in sorted(counts.items()):
    group = [r for r in ranked if r['version']==version]
    rows.append(f"| v{version} | {len(group)} | {sum(r['wins'] for r in group)}–{sum(r['draws'] for r in group)}–{sum(r['losses'] for r in group)} | {count} | {max(24,96-7.2*count):g} |")
text = f'''# Latest ranked timing

Snapshot: **{snapshot['fetched_at']}**. Active submission: **{active['name']} / {active['id']}**.

| Version | Completed ranked series | Games W–D–L | Effective count | Next K |
|---|---:|---|---:|---:|
'''+'\n'.join(rows)+f'''

Fresh-window deadline: **{deadline.astimezone(ZoneInfo('Asia/Kolkata')).strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]} IST**
or **{deadline.strftime('%Y-%m-%d %H:%M:%S.%f')[:-3]} UTC**. This uses the server's
`kFreshAt` ({fresh_at}), not upload or individual-game start time. Fresh code before
that deadline inherits the last ranked bot's count capped at ten; identical code
retains its own progress. A stronger candidate can still be worth submitting earlier.

{len(pending)} live/queued ranked series are excluded from completed counts:
{', '.join(str(b['battle_id'])+' ('+b['opponent']+')' for b in pending) or 'none'}.
Some games can finish while replays download; this timing ledger consistently uses
the listing's snapshot cutoff. Refresh before later submission advice.

Original upload receipts remain in [history.json](../../submissions/history.json).
The earlier [timing audit](TIMING.md) explains the first-ranked timestamp discrepancy.
v3 is packaged locally and has not been uploaded.
'''
(audit/'TIMING-LATEST.md').write_text(text)
print(text)
