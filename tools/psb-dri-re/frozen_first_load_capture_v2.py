"""Cycle04 passive capture tools. No boot, staging, or SGX actions.

V1 remains immutable. Proposed cycle limits require separate boot authorization.
STOCK preparation has no boot-age limit and cannot qualify a boot/recovery cycle.
"""
import math

BOOT_WATCH_SECONDS = 600
CYCLE_CAPTURE_MAX_BOOT_SECONDS = 1200
CONNECTION_SECONDS = 40
CHILD_SECONDS = 35
MODES = ('stock_preparation', 'experimental', 'stock_recovery')


def require_ready(record):
    for key in ('local_sudo_succeeded', 'normal_userspace', 'physical_display_normal'):
        if record.get(key) is not True:
            raise ValueError('explicit current-boot readiness required: '+key)
    return CONNECTION_SECONDS


def process_start_record(text, hz):
    """Parse real /proc/PID/stat without confusing comm parentheses with fields."""
    try:
        pid = int(text.split('(', 1)[0].strip())
        _, separator, tail = text.rpartition(') ')
        fields = tail.split()
        ticks = int(fields[19])  # field22; tail starts at field3 (state)
        if not separator or pid <= 0 or ticks < 0 or type(hz) is not int or hz <= 0:
            raise ValueError('invalid process clock')
    except (IndexError, TypeError, AttributeError, ValueError) as exc:
        raise ValueError('invalid /proc process start record') from exc
    return {'pid':pid, 'start_ticks':ticks, 'clock_ticks_per_second':hz,
            'process_start_since_boot_seconds':ticks/hz,
            'meaning':'process start, not userspace readiness'}


def wrapper_source(root_script, mode):
    if mode not in MODES or not isinstance(root_script, str) or not root_script.strip():
        raise ValueError('reviewed source and explicit capture mode required')
    compile(root_script, 'reviewed-root-capture', 'exec')
    limit = None if mode == 'stock_preparation' else CYCLE_CAPTURE_MAX_BOOT_SECONDS
    return '''import os,sys,json,subprocess,math,time,datetime
MODE = MODE_VALUE
LIMIT = LIMIT_VALUE
try:
 with open('/proc/sys/kernel/random/boot_id') as f:boot_id=f.read().strip()
 with open('/proc/uptime') as f:uptime_start=float(f.read().split()[0])
 if not boot_id:raise ValueError('empty boot ID')
 u=os.uname();started=time.monotonic()
 print(json.dumps({'phase':'unprivileged-boot-identity','mode':MODE,'boot_id':boot_id,'kernel':u.release,'architecture':u.machine,'uptime_start':uptime_start,'utc_start':datetime.datetime.now(datetime.timezone.utc).isoformat()}),flush=True)
 if not math.isfinite(uptime_start) or uptime_start<0:raise ValueError('invalid kernel clock')
 if LIMIT is not None and uptime_start>=LIMIT:raise ValueError('cycle evidence window expired')
 budget=35 if LIMIT is None else min(35,LIMIT-uptime_start)
except Exception as exc:
 print(json.dumps({'phase':'STOP','identity_error':str(exc)}),flush=True);sys.exit(1)
try:
 cp=subprocess.run(['sudo','-n','-p','','python3','-I','-B','-S','-c',ROOT_SCRIPT],stdin=subprocess.DEVNULL,stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True,timeout=budget)
except subprocess.TimeoutExpired as exc:
 for stream,data in [(sys.stdout,exc.stdout),(sys.stderr,exc.stderr)]:
  if data:stream.write(data.decode(errors='replace') if isinstance(data,bytes) else data);stream.flush()
 print(json.dumps({'phase':'STOP','error':'passive capture timeout','boot_id':boot_id}),flush=True);sys.exit(1)
sys.stdout.write(cp.stdout);sys.stdout.flush();sys.stderr.write(cp.stderr);sys.stderr.flush()
if cp.returncode:sys.exit(cp.returncode)
try:
 with open('/proc/sys/kernel/random/boot_id') as f:boot_id_end=f.read().strip()
 with open('/proc/uptime') as f:uptime_end=float(f.read().split()[0])
 duration=time.monotonic()-started
 passed=boot_id_end==boot_id and math.isfinite(uptime_end) and uptime_start<=uptime_end and math.isfinite(duration) and 0<=duration<=35 and abs((uptime_end-uptime_start)-duration)<=1 and (LIMIT is None or uptime_end<=LIMIT)
 print(json.dumps({'phase':'passive-capture-end','mode':MODE,'boot_id':boot_id_end,'uptime_end':uptime_end,'duration_seconds':duration,'same_boot_timing_pass':passed,'utc_end':datetime.datetime.now(datetime.timezone.utc).isoformat()}),flush=True)
 sys.exit(0 if passed else 1)
except Exception as exc:
 print(json.dumps({'phase':'STOP','end_identity_error':str(exc)}),flush=True);sys.exit(1)
'''.replace('MODE_VALUE', repr(mode)).replace('LIMIT_VALUE', repr(limit)).replace('ROOT_SCRIPT', repr(root_script))


def validate_receipt(records, status, stderr, host_duration, mode):
    def need(condition, why):
        if not condition:
            raise ValueError(why)
    need(mode in MODES, 'unknown capture mode')
    need(status == 0 and not stderr, 'failed capture or unexpected stderr')
    need(type(host_duration) in (int, float) and math.isfinite(host_duration)
         and 0 <= host_duration <= CONNECTION_SECONDS, 'host capture deadline')
    need(isinstance(records, list) and len(records) == 3
         and all(isinstance(x, dict) for x in records), 'incomplete/extra capture records')
    start, root, end = records
    need(start.get('phase') == 'unprivileged-boot-identity'
         and end.get('phase') == 'passive-capture-end'
         and start.get('mode') == end.get('mode') == mode, 'capture phase/mode')
    # Retained root scripts use either spelling; every supplied value must agree.
    kernels = [root[key] for key in ('kernel', 'kernel_release') if key in root]
    need(isinstance(start.get('kernel'), str) and bool(start['kernel'])
         and bool(kernels) and all(value == start['kernel'] for value in kernels),
         'capture kernel drift')
    need(isinstance(start.get('architecture'), str) and bool(start['architecture'])
         and start['architecture'] == root.get('architecture'),
         'capture architecture drift')
    ids = [x.get('boot_id') for x in records]
    need(all(isinstance(x, str) and x for x in ids) and len(set(ids)) == 1,
         'capture boot identity')
    need(isinstance(root.get('guards'), list) and bool(root['guards'])
         and all(isinstance(x, dict) and x.get('pass') is True for x in root['guards']),
         'failed/absent root guards')
    a, b, duration = start.get('uptime_start'), end.get('uptime_end'), end.get('duration_seconds')
    need(all(type(x) in (int, float) and math.isfinite(x) for x in [a, b, duration])
         and 0 <= a <= b and 0 <= duration <= CHILD_SECONDS
         and abs((b-a)-duration) <= 1,
         'invalid/regressing capture clocks')
    need(duration <= host_duration, 'target capture exceeds enclosing host interval')
    if mode != 'stock_preparation':
        need(a < CYCLE_CAPTURE_MAX_BOOT_SECONDS and b <= CYCLE_CAPTURE_MAX_BOOT_SECONDS,
             'cycle evidence deadline')
    need(end.get('same_boot_timing_pass') is True, 'target timing receipt rejected')
    return {'classification':'PASS RAW CAPTURE ONLY', 'boot_id':ids[0],
            'mode':mode, 'sgx_authorized':False, 'boot_authorized':False,
            'first_owner':'NOT QUALIFIED BY TIMING ALONE',
            'full_recovery':'NOT QUALIFIED BY TIMING ALONE'}


def capture_once(evidence_path, reviewed_ssh_prefix, root_script, readiness, mode):
    """One reviewed read-only connection. Caller supplies scope/provenance/authorization.

    Exclusive local directory consumes this attempt before any connection. This
    function grants no authorization and never retries. No CLI or boot action.
    """
    from pathlib import Path
    import subprocess, shlex, time, datetime, json, hashlib
    require_ready(readiness)
    if not isinstance(reviewed_ssh_prefix, list) or not reviewed_ssh_prefix:
        raise ValueError('reviewed pinned transport missing')
    wrapper = wrapper_source(root_script, mode)
    directory = Path(evidence_path)
    directory.mkdir(exist_ok=False)
    (directory/'root-source.py').write_text(root_script)
    (directory/'wrapper-source.py').write_text(wrapper)
    argv = reviewed_ssh_prefix + ['python3 -I -B -S -c '+shlex.quote(wrapper)]
    record = {'argv':argv, 'mode':mode, 'timeout_seconds':CONNECTION_SECONDS,
              'stdin':'DEVNULL', 'automatic_retry':False,
              'root_source_sha256':hashlib.sha256(root_script.encode()).hexdigest(),
              'start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat()}
    (directory/'command.json').write_text(json.dumps(record,indent=2)+'\n')
    started = time.monotonic()
    try:
        child = subprocess.run(argv,stdin=subprocess.DEVNULL,capture_output=True,
                               timeout=CONNECTION_SECONDS)
        stdout, stderr, status = child.stdout, child.stderr, child.returncode
    except subprocess.TimeoutExpired as exc:
        stdout, stderr, status = exc.stdout or b'', exc.stderr or b'', 124
    duration = time.monotonic()-started
    (directory/'stdout.txt').write_bytes(stdout)
    (directory/'stderr.txt').write_bytes(stderr)
    record.update(exit_status=status, host_duration_seconds=duration,
                  end_utc=datetime.datetime.now(datetime.timezone.utc).isoformat())
    (directory/'command.json').write_text(json.dumps(record,indent=2)+'\n')
    try:
        records=[]; remaining=stdout.decode(); decoder=json.JSONDecoder()
        while remaining.strip():
            remaining=remaining.lstrip(); value,end=decoder.raw_decode(remaining)
            records.append(value); remaining=remaining[end:]
        verdict=validate_receipt(records,status,stderr,duration,mode)
    except (ValueError, UnicodeError) as exc:
        (directory/'verdict.json').write_text(json.dumps({'classification':'STOP',
             'reason':str(exc),'retry':False,'sgx_authorized':False},indent=2)+'\n')
        raise ValueError('STOP: capture refused; evidence retained; no retry') from exc
    (directory/'decoded-records.json').write_text(json.dumps(records,indent=2)+'\n')
    (directory/'verdict.json').write_text(json.dumps(verdict,indent=2)+'\n')
    return records,verdict
