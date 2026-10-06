"""Cycle04 supplied-record gates. No remote actions; never grants boot/SGX permission.

Uses v1's identity/stock rules unchanged, with a distinct automatic timing receipt.
No fabricated elapsed value is passed into the v1 deadline validator.
"""
import hashlib,json
from pathlib import Path
import frozen_first_load_procedure as v1
import frozen_first_load_capture_v2 as timing

ROOT=Path(__file__).resolve().parents[2]
PLAN=ROOT/'docs/phase8/cycle04-first-load-procedure.json'
POLICY={'max_experimental_boots':1,'sgx_actions':0,'automatic_retry':False,
        'hot_restoration':False,'automatic_reset':False,'restaging':False,
        'change_default':False,'boot_watch_seconds':600,
        'capture_max_kernel_boot_seconds':1200,'connection_seconds':40,
        'child_seconds':35,'menu_photo_before_selection':True,
        'sgx_gate_b':'BLOCKED','sgx_whitelist':[],
        'live_execution_authorized':False}
TRACE=['SGX535-FIRSTLOAD BEGIN','SGX535-FIRSTLOAD FILES-VERIFIED; INSERTION-POSSIBLE',
       'SGX535-FIRSTLOAD PASS: derivative first owner; boot may continue']


def load_plan():return json.loads(PLAN.read_text())


def validate_plan(p):
    v1.require(p.get('schema')==2 and p.get('policy')==POLICY,'v2 scope/policy drift')
    reference=ROOT/v1.PLAN
    v1.require(p.get('reference_plan')==v1.PLAN
               and p.get('reference_plan_sha256')==hashlib.sha256(reference.read_bytes()).hexdigest(),
               'v1 identity-plan provenance drift')
    v1.validate_plan(v1.load_plan(ROOT))
    return {'classification':'PASS OFFLINE V2 PLAN','boot_authorized':False,'sgx_authorized':False}


def _timing(f,mode):
    v1.require(f.get('userspace_seen_before_boot_watch') is True,'boot-watch observation missing/failed')
    keys=['capture_records','capture_status','capture_stderr','host_duration_seconds']
    v1.require(all(k in f for k in keys),'missing automatic timing evidence')
    v1.require(isinstance(f['capture_stderr'],(str,bytes)),'invalid stderr record')
    r=timing.validate_receipt(f['capture_records'],f['capture_status'],f['capture_stderr'],
                              f['host_duration_seconds'],mode)
    v1.require(r['boot_id']==f.get('boot_id'),'supplied boot differs from raw capture')


def validate_first_owner(p,f):
    validate_plan(load_plan());v1.validate_plan(p);v1.bound_identity(p,f,v1.DERIVATIVE_NOTE)
    _timing(f,'experimental')
    for k in ['selection_photo','stock_entry_visible_before_selection','experimental_selected_once',
              'userspace_reached','display_normal','slimski_running','xorg_running']:
        v1.require(f.get(k) is True,'first-load guard '+k)
    v1.require(all(isinstance(f.get(k),str) and f[k] for k in ['boot_id','prior_boot_id'])
               and f['boot_id']!=f['prior_boot_id'] and f.get('experimental_boots')==1,
               'experimental attribution/bound')
    v1.require(f.get('log','').splitlines()==TRACE,'missing/duplicate/reordered/error hook trace')
    return {'first_owner':'PASS PROVIDED RECORD','first_load':'PASS PROVIDED RECORD',
            'raw_provenance':'INDEPENDENT VERIFICATION REQUIRED',
            'boot_authorized':False,'sgx_authorized':False}


def validate_recovery(p,f):
    validate_plan(load_plan());v1.validate_plan(p);v1.bound_identity(p,f,v1.ORIGINAL_NOTE)
    _timing(f,'stock_recovery')
    for k in ['selection_photo','stock_selected','stock_files_exact','userspace_reached',
              'display_normal','ssh_available','slimski_running','xorg_running']:
        v1.require(f.get(k) is True,'stock-recovery guard '+k)
    v1.require(all(isinstance(f.get(k),str) and f[k] for k in ['prior_boot_id','experimental_boot_id','boot_id'])
               and len({f['prior_boot_id'],f['experimental_boot_id'],f['boot_id']})==3,
               'missing/contradictory three-boot boundary')
    v1.require(f.get('grub_env')=={'saved_entry':v1.STOCK_ID},'stock saved/default state')
    v1.require(f.get('framebuffer_dimensions')=='1280x800' and f.get('vtcon0')==0
               and f.get('vtcon1')==1,'stock framebuffer/VT state')
    return {'recovery':'PASS PROVIDED RECORD','raw_provenance':'INDEPENDENT VERIFICATION REQUIRED',
            'boot_authorized':False,'sgx_authorized':False}
