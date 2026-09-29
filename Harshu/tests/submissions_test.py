import importlib.util, datetime as dt
from pathlib import Path
spec=importlib.util.spec_from_file_location('tracker',Path(__file__).parents[1]/'tools/submissions.py')
t=importlib.util.module_from_spec(spec);spec.loader.exec_module(t)
now=t.parse_time('2026-09-27T12:00:00Z')
h={'uploads':[],'ranked_battles':[],'history_known_from_account_creation':False}
assert 'incomplete' in '\n'.join(t.advice(h,'new',now))
h['uploads']=[{'status':'accepted','uploaded_at':'2026-09-27T11:59:00Z','fingerprint':'old','version':'1'}]
assert 'No confirmed first-ranked' in '\n'.join(t.advice(h,'new',now))
h['ranked_battles']=[{'version':'1','played_at':'2026-09-27T08:00:00Z','fresh_window':True,'count_after':4}]
a='\n'.join(t.advice(h,'new',now));assert '20:00:00 UTC' in a
assert 'highest recorded effective count is 4' in '\n'.join(t.advice(h,'old',now))
assert 'expired' in '\n'.join(t.advice(h,'new',now+dt.timedelta(hours=9)))
h['fresh_window']={'started_at':'2026-09-27T11:00:00Z'}
assert '23:00:00 UTC' in '\n'.join(t.advice(h,'new',now+dt.timedelta(hours=9)))
assert 'expired' in '\n'.join(t.advice(h,'new',now+dt.timedelta(hours=12)))
h['uploads']*=12
assert 'quota is full' in '\n'.join(t.advice(h,'new',now))
assert t.parse_time('2026-09-27T17:30:00+05:30')==now
try:t.parse_time('2026-09-27T17:30:00')
except ValueError:pass
else:raise AssertionError('Naive timestamp accepted')
ranked_before=list(h['ranked_battles'])
t.sync_markers(h,{'fetched_at':'2026-09-27T12:00:00Z','submissions':[{'id':42,'version':2,'name':'new','status':'active','kStart':0,'kFreshAt':'2026-09-27T11:30:00Z'}]})
assert h['fresh_window']['started_at']=='2026-09-27T11:30:00Z' and h['latest_submission_verification']['active_submission_id']==42
assert h['ranked_battles']==ranked_before
print('12 submission timing checks passed')
