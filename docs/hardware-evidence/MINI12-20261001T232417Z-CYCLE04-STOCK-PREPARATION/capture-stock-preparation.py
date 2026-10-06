from pathlib import Path
import sys,subprocess,json,shlex,hashlib,time,datetime
R=Path('/home/gama/sgx535-gfx');E=Path(Path('/tmp/sgx535-cycle04-evidence').read_text());W=Path(Path('/tmp/sgx535-cycle04-work').read_text())
sys.path.insert(0,str(R/'tools/psb-dri-re'))
import frozen_first_load_capture_v2 as v2
import frozen_first_load_procedure as old
ready=json.loads((E/'operator-preparation-readiness.json').read_text());v2.require_ready(ready)
source=(E/'stock-preparation-root.py').read_text();wrapper=(E/'stock-preparation-wrapper.py').read_text();pins=json.loads((E/'source-pins.json').read_text())
assert hashlib.sha256(source.encode()).hexdigest()==pins['new_root_sha256']
assert hashlib.sha256(wrapper.encode()).hexdigest()==pins['wrapper_sha256']
assert wrapper==v2.wrapper_source(source,'stock_preparation')
d=E/'stock-preparation-capture';d.mkdir(exist_ok=False)
argv=json.loads((W/'pinned-ssh-prefix.json').read_text())+['python3 -I -B -S -c '+shlex.quote(wrapper)]
r={'argv':argv,'start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'timeout_seconds':40,'stdin':'DEVNULL','scope':'ONE STOCK-side read-only preparation; no boot/cycle qualification; no retry'}
(d/'command.json').write_text(json.dumps(r,indent=2)+'\n');started=time.monotonic()
try:
 cp=subprocess.run(argv,stdin=subprocess.DEVNULL,capture_output=True,timeout=40);out,err,status=cp.stdout,cp.stderr,cp.returncode
except subprocess.TimeoutExpired as exc:out,err,status=exc.stdout or b'',exc.stderr or b'',124
r.update(host_duration_seconds=time.monotonic()-started,end_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),exit_status=status)
(d/'stdout.txt').write_bytes(out);(d/'stderr.txt').write_bytes(err);(d/'command.json').write_text(json.dumps(r,indent=2)+'\n')
rows=[];s=out.decode();dec=json.JSONDecoder()
try:
 while s.strip():s=s.lstrip();row,n=dec.raw_decode(s);rows.append(row);s=s[n:]
 verdict=v2.validate_receipt(rows,status,err,r['host_duration_seconds'],'stock_preparation')
 root=rows[1];p=old.load_plan(R)
 receipts=json.loads((R/'docs/hardware-evidence/MINI12-20261001T062604Z-FIRSTLOAD-CYCLE-02/staging/decoded-records.json').read_text())
 verdict['existing_stage_receipts']=old.validate_existing_stage_files(p,receipts,root['destinations']);verdict['guards_passed']=len(root['guards'])
 verdict['stock_boot_age_seconds']=rows[0]['uptime_start'];verdict['capture_uptime_end']=rows[-1]['uptime_end'];verdict['host_duration_seconds']=r['host_duration_seconds'];verdict['target_duration_seconds']=rows[-1]['duration_seconds']
 verdict['historical_cycle03_repaired']=False
 (d/'decoded-records.json').write_text(json.dumps(rows,indent=2)+'\n');(d/'verdict.json').write_text(json.dumps(verdict,indent=2)+'\n')
 print('PASS STOCK read-only preparation;',len(root['guards']),'root guards; existing creation receipts PASS.')
 print('Automatic uptime range:',rows[0]['uptime_start'],rows[-1]['uptime_end'],'seconds; capture duration:',round(r['host_duration_seconds'],3),'seconds.')
except Exception as exc:
 (d/'verdict.json').write_text(json.dumps({'classification':'STOP; raw evidence preserved','reason':str(exc),'retry':False,'sgx_authorized':False},indent=2)+'\n')
 print('STOP:',str(exc));sys.exit(1)
