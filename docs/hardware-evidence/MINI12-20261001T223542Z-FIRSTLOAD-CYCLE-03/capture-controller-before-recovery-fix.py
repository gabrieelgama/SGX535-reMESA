from pathlib import Path
import sys, os, json, subprocess, hashlib, datetime, shlex
R=Path('/home/gama/sgx535-gfx')
W=Path('/tmp/sgx535-cycle03-work').read_text();W=Path(W)
E=Path('/tmp/sgx535-cycle03-evidence').read_text();E=Path(E)
sys.path.insert(0,str(R/'tools/psb-dri-re'))
import frozen_first_load_capture as timing
import frozen_first_load_procedure as procedure
OLD=R/'docs/hardware-evidence/MINI12-20261001T062604Z-FIRSTLOAD-CYCLE-02'

def capture(stage, source, timeout):
 d=E/stage
 d.mkdir(exist_ok=False)
 wrapper=timing.wrapper_source(source) if stage!='preflight' else None
 remote='python3 -I -B -S -c '+shlex.quote(wrapper) if wrapper else 'sudo -n -p '+shlex.quote('')+' python3 -I -B -S -c '+shlex.quote(source)
 argv=json.loads((W/'pinned-ssh-prefix.json').read_text())+[remote]
 (d/'root-source.py').write_text(source)
 if wrapper:(d/'wrapper-source.py').write_text(wrapper)
 command={'argv':argv,'start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'timeout_seconds':timeout,'scope':'ONE read-only capture; stdin DEVNULL; no credential input; no retry','root_source_sha256':hashlib.sha256(source.encode()).hexdigest()}
 (d/'command.json').write_text(json.dumps(command,indent=2)+'\n')
 try:
  cp=subprocess.run(argv,stdin=subprocess.DEVNULL,capture_output=True,timeout=timeout)
  out,err,status=cp.stdout,cp.stderr,cp.returncode
 except subprocess.TimeoutExpired as exc:
  out,err,status=exc.stdout or b'',exc.stderr or b'',124
 (d/'stdout.txt').write_bytes(out);(d/'stderr.txt').write_bytes(err)
 command.update(end_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),exit_status=status,stdout_sha256=hashlib.sha256(out).hexdigest(),stderr_sha256=hashlib.sha256(err).hexdigest())
 (d/'command.json').write_text(json.dumps(command,indent=2)+'\n')
 if status or err:raise RuntimeError('STOP: capture failed; preserved evidence; no retry')
 records=[];s=out.decode();decoder=json.JSONDecoder()
 while s.strip():
  s=s.lstrip();value,end=decoder.raw_decode(s);records.append(value);s=s[end:]
 root=next(x for x in records if 'guards' in x)
 if not root['guards'] or not all(row['pass'] for row in root['guards']):raise RuntimeError('STOP: incomplete root guards')
 if wrapper:
  prefix=records[0];end=records[-1]
  if prefix.get('phase')!='unprivileged-boot-identity' or prefix['boot_id']!=root['boot_id'] or not end.get('same_boot_deadline_pass') or end['boot_id']!=root['boot_id']:raise RuntimeError('HOLD: boot/timing receipt mismatch')
 (d/'decoded-records.json').write_text(json.dumps(records,indent=2)+'\n')
 return root

def main(stage):
 p=procedure.load_plan(R);procedure.validate_plan(p,R)
 if stage=='preflight':
  witness=json.loads((E/'operator-preflight-readiness.json').read_text())
  for key in ['physically_present','stock_display_normal','grub_controls_available','manual_power_controls_available','data_loss_risk_understood','local_sudo_succeeded']:
   if witness.get(key) is not True:raise ValueError('STOP: operator prerequisite '+key)
  root=capture(stage,(E/'stock-existing-preflight.py').read_text(),60)
  receipt=procedure.validate_existing_stage_files(p,json.loads((OLD/'staging/decoded-records.json').read_text()),root['destinations'])
  prior=json.loads((OLD/'stock-recovery/stdout.txt').read_text())
  if root['stock']!=prior['stock']:raise ValueError('STOP: stock file identities changed from retained recovery')
  if not root.get('boot_id'):raise ValueError('STOP: missing stock boot ID')
  (E/'preflight-decision.json').write_text(json.dumps({'classification':'PASS','stock_boot_id':root['boot_id'],'guards_passed':len(root['guards']),'existing_creation_receipts':receipt,'sgx_authorized':False},indent=2)+'\n')
 elif stage in ['experimental','stock-recovery']:
  before=json.loads((E/'preflight-decision.json').read_text());assert before['classification']=='PASS'
  ready=json.loads((E/('operator-'+stage+'-readiness.json')).read_text());timeout=timing.require_ready(ready)
  if stage=='experimental':
   for key in ['menu_photo_captured','stock_still_default','stock_entry_visible','experimental_selected_exactly_once','physical_display_normal']:
    if ready.get(key) is not True:raise ValueError('HOLD: experimental selection/operator prerequisite '+key)
   source=(E/'experimental-source-before.py').read_text().replace("'8ae37532-19d4-4ec7-9c29-a791acfd889f'",repr(before['stock_boot_id']))
  else:
   if ready.get('stock_manually_selected') is not True or ready.get('physical_display_normal') is not True:raise ValueError('HOLD: stock operator prerequisite')
   exp=json.loads((E/'experimental/decoded-records.json').read_text());expid=next(x for x in exp if 'guards' in x)['boot_id']
   source=(E/'stock-existing-preflight.py').read_text()
   anchor=" check(txt('/proc/sys/kernel/random/boot_id')==result['boot_id'],'same boot at capture end')"
   assert source.count(anchor)==1
   source=source.replace(anchor,anchor+"\n check(result['boot_id'] not in "+repr([before['stock_boot_id'],expid])+",'distinct recovery boot')")
  root=capture(stage,source,timeout)
  procedure.validate_existing_stage_files(p,json.loads((OLD/'staging/decoded-records.json').read_text()),root['destinations'])
  (E/(stage+'-root-pass.json')).write_text(json.dumps({'classification':'PASS ROOT CAPTURE; operator completion/record qualification pending','boot_id':root['boot_id'],'guards_passed':len(root['guards']),'sgx_authorized':False},indent=2)+'\n')
 else:raise ValueError('no such reviewed stage')
if __name__=='__main__':
 if len(sys.argv)!=2:raise ValueError('one explicit stage required')
 main(sys.argv[1])
