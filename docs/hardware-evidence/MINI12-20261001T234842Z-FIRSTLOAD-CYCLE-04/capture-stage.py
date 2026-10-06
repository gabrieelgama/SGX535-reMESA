from pathlib import Path
import sys,json,hashlib,os
R=Path('/home/gama/sgx535-gfx')
E=Path(Path('/tmp/sgx535-cycle04-live-evidence').read_text())
sys.path.insert(0,str(R/'tools/psb-dri-re'))
import frozen_first_load_capture_v2 as timing
import frozen_first_load_procedure as base
import frozen_first_load_procedure_v2 as gates
stage=sys.argv[1]
if stage not in ('experimental','stock-recovery'):raise ValueError('explicit reviewed stage required')
for pin in json.loads((E/'tooling-pins.json').read_text()):
 assert hashlib.sha256((R/pin['path']).read_bytes()).hexdigest()==pin['sha256'],'tooling drift'
gates.validate_plan(gates.load_plan());p=base.load_plan(R);base.validate_plan(p,R)
ready=json.loads((E/('operator-'+stage+'-readiness.json')).read_text());timing.require_ready(ready)
prior=json.loads((E/'preflight-decision.json').read_text());assert prior['classification']=='PASS'
if stage=='experimental':
 menu=json.loads((E/'menu-review.json').read_text())
 assert menu['classification']=='PASS' and menu['photo_supplied'] is True and menu['stock_still_default'] is True and menu['both_titles_visible'] is True
 assert all(ready.get(k) is True for k in ('experimental_selected_exactly_once','userspace_seen_before_boot_watch'))
 source=(E/'experimental-root.py').read_text();mode='experimental'
 provenance=json.loads((E/'future-capture-source-provenance.json').read_text())
 assert hashlib.sha256(source.encode()).hexdigest()==provenance['experimental_root_sha256'],'experimental source drift'
else:
 assert ready.get('stock_manually_selected') is True and ready.get('userspace_seen_before_boot_watch') is True
 expid=None
 f=E/'experimental/decoded-records.json'
 if f.exists():expid=json.loads(f.read_text())[0]['boot_id']
 # Missing experimental data stays missing. A fresh stock observation is still
 # allowed for recovery; it must never be promoted to a full ledger PASS.
 ids=[prior['stock_boot_id']]+([expid] if expid else [])
 (E/'recovery-prior-identities.json').write_text(json.dumps({'stock_before':prior['stock_boot_id'],'experimental':expid,'full_ledger_possible':bool(expid)},indent=2)+'\n')
 source=(E/'stock-root-template.py').read_text()
 provenance=json.loads((E/'future-capture-source-provenance.json').read_text())
 assert hashlib.sha256(source.encode()).hexdigest()==provenance['stock_template_sha256'],'stock template drift'
 anchor=" check(txt('/proc/sys/kernel/random/boot_id')==result['boot_id'],'same boot at capture end')"
 assert source.count(anchor)==1
 source=source.replace(anchor,anchor+"\n check(result['boot_id'] not in "+repr(ids)+",'distinct recovery boot')")
 mode='stock_recovery'
rows,verdict=timing.capture_once(E/stage,json.loads((E/'pinned-ssh-prefix.json').read_text()),source,ready,mode)
root=rows[1]
creation=json.loads((R/'docs/hardware-evidence/MINI12-20261001T062604Z-FIRSTLOAD-CYCLE-02/staging/decoded-records.json').read_text())
verdict['existing_stage_receipts']=base.validate_existing_stage_files(p,creation,root['destinations'])
assert root['stock']==json.loads((E/'preflight/decoded-records.json').read_text())[1]['stock'],'stock bytes/provenance drift'
verdict['guards_passed']=len(root['guards']);verdict['qualification']='PASS PASSIVE RAW OBSERVATION; full record/operator review required'
(E/(stage+'-raw-pass.json')).write_text(json.dumps(verdict,indent=2)+'\n')
print(stage,'PASS:',len(root['guards']),'guards; boot ID',root['boot_id'])
print('Automatic target uptime:',rows[0]['uptime_start'],'to',rows[-1]['uptime_end'])
print('No SGX operation, hot unload or replacement.')
