"""Offline construction/qualification of the future single passive capture wrapper.

No SSH or target contact occurs here. The reviewed caller must pin the root
capture program, get explicit operator readiness and impose remaining deadline.
An unprivileged identity prefix is audit evidence, not first-owner qualification.
"""
import math
import os

def require_ready(record):
 if record.get('local_sudo_succeeded') is not True or record.get('normal_userspace') is not True:
  raise ValueError('explicit current-boot userspace and local sudo witness required')
 elapsed=record.get('elapsed_seconds')
 if type(elapsed) not in (int,float) or not math.isfinite(elapsed) or not 0<=elapsed<120:
  raise ValueError('handoff clock must leave a positive capture budget')
 return min(40,120-elapsed)

def wrapper_source(root_script):
 """Return code only; caller provides exactly its reviewed, hash-pinned capture."""
 if not isinstance(root_script,str) or not root_script.strip():raise ValueError('missing reviewed root capture')
 compile(root_script,'reviewed-root-capture','exec')
 return '''import os,sys,json,subprocess,math
try:
 with open('/proc/sys/kernel/random/boot_id','r') as f:boot_id=f.read().strip()
 if not boot_id:raise ValueError('empty boot ID')
 with open('/proc/uptime','r') as f:uptime_start=float(f.read().split()[0])
 u=os.uname()
 print(json.dumps({'phase':'unprivileged-boot-identity','boot_id':boot_id,'kernel':u.release,'architecture':u.machine,'uptime_start':uptime_start}),flush=True)
 if not math.isfinite(uptime_start) or not 0<=uptime_start<120:raise ValueError('kernel clock leaves no capture budget')
except Exception as exc:
 print(json.dumps({'phase':'STOP','identity_error':str(exc)}),flush=True)
 sys.exit(1)
try:
 cp=subprocess.run(['sudo','-n','-p','','python3','-I','-B','-S','-c',ROOT_SCRIPT],stdin=subprocess.DEVNULL,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,timeout=min(40,120-uptime_start))
except subprocess.TimeoutExpired as exc:
 for stream,data in [(sys.stdout,exc.stdout),(sys.stderr,exc.stderr)]:
  if data:stream.write(data.decode(errors='replace') if isinstance(data,bytes) else data);stream.flush()
 print(json.dumps({'phase':'STOP','error':'passive capture deadline exceeded','boot_id':boot_id}),flush=True)
 sys.exit(1)
sys.stdout.write(cp.stdout);sys.stdout.flush();sys.stderr.write(cp.stderr);sys.stderr.flush()
if cp.returncode:sys.exit(cp.returncode)
try:
 with open('/proc/sys/kernel/random/boot_id','r') as f:boot_id_end=f.read().strip()
 with open('/proc/uptime','r') as f:uptime_end=float(f.read().split()[0])
 passed=boot_id_end==boot_id and math.isfinite(uptime_end) and uptime_start<=uptime_end<=120
 print(json.dumps({'phase':'passive-capture-end','boot_id':boot_id_end,'uptime_end':uptime_end,'same_boot_deadline_pass':passed}),flush=True)
 sys.exit(0 if passed else 1)
except Exception as exc:
 print(json.dumps({'phase':'STOP','end_identity_error':str(exc)}),flush=True)
 sys.exit(1)
'''.replace('ROOT_SCRIPT',repr(root_script))
