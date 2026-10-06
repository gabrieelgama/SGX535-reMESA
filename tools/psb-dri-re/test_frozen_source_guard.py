"""Offline tests of policy and actual extracted native code; no device access."""
import os
from pathlib import Path
import re
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[2]
BACKEND = ROOT / 'kernel/sgx535_frozen/gma500_fixed_backend.c'
OBSERVER = ROOT / 'kernel/sgx535_frozen/gma500_capsule_observer.c'
SNAPSHOT = ROOT / 'kernel/sgx535_frozen/gma500_source_snapshot.h'
BASE = Path('/home/gama/sgx535-offline/phase8-provenance-implementation-20261005T005304Z/build/module')


def function(source, name):
    match = re.search(r'\b' + re.escape(name) + r'\s*\([^;{}]*\)\s*\{', source)
    if not match:
        raise AssertionError('function missing: ' + name)
    start = source.index('{', match.start())
    depth = 1
    end = start + 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[match.start():end]


def compile_run(text):
    with tempfile.TemporaryDirectory() as directory:
        src = Path(directory) / 'adapter.c'
        src.write_text(text)
        exe = Path(directory) / 'adapter'
        build=subprocess.run([os.environ.get('CC', 'gcc'), '-std=c11', '-Wall',
                        '-Wextra', '-Werror', '-I', str(ROOT/'tools/psb-dri-re'),
                        str(src), str(ROOT/'tools/psb-dri-re/frozen_source_guard.c'),
                        '-o', str(exe)], capture_output=True)
        if build.returncode:
            raise AssertionError(build.stderr.decode())
        return subprocess.run([str(exe)], check=True, capture_output=True).stdout


PREFIX = r'''
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "frozen_source_guard.h"
typedef unsigned u32;
#define true 1
#define false 0
#define PSB_CR_EVENT_STATUS 0x12c
#define PSB_CR_EVENT_STATUS2 0x118
#define PSB_CR_2D_SOCIF 0xe18
#define PSB_CR_2D_BLIT_STATUS 0xe04
#define PSB_CR_SOFT_RESET 0x80
#define PSB_CR_BIF_INT_STAT 0xc04
#define _PSB_C2_SOCIF_EMPTY 0x80
#define _PSB_C2B_STATUS_BUSY (1U<<24)
struct drm_device { void *dev_private; };
struct drm_psb_private { void *sgx_reg; struct drm_device *dev; struct { unsigned gatt_start; } gtt; };
struct session { unsigned phase; };
struct service { struct session *session; const void *observer; void *observer_context; };
struct sgx535_gma500_owner {
    unsigned initialized, device_claimed, power_held;
    struct session session; struct service fixed_service; struct drm_device *dev;
};
struct sgx535_gma500_fixed_backend {
    struct sgx535_gma500_owner *owner;
    unsigned active, power_held, last_stage, pending_status1, pending_status2, fire_possible;
};
static unsigned reads, hardware_pending, hardware_busy, hardware_reset, hardware_reads, hardware_fault, hardware_timer, hardware_kick;
static struct sgx535_source_guard guard;
static const struct sgx535_source_facts clean={1,0,0,0,0,0,1,0};
static int gma_power_is_on(struct drm_device *d) { return d!=NULL; }
static unsigned passive_read(struct drm_psb_private *p, unsigned offset) {
    if (!p->sgx_reg) return ~0U;
    reads++;
    if (offset==PSB_CR_EVENT_STATUS) return hardware_pending;
    if (offset==PSB_CR_2D_SOCIF) return _PSB_C2_SOCIF_EMPTY;
    if (offset==PSB_CR_2D_BLIT_STATUS) return hardware_busy;
    if (offset==PSB_CR_SOFT_RESET) return hardware_reset;
    if (offset==SGX535_SOURCE_BIF_READS_REG) return hardware_reads;
    if (offset==PSB_CR_BIF_INT_STAT) return hardware_fault;
    if (offset==SGX535_SOURCE_EVENT_TIMER_REG) return hardware_timer;
    if (offset==SGX535_SOURCE_EVENT_KICK_REG) return hardware_kick;
    return 0;
}
#define PSB_RSGX32(offset) passive_read(dev_priv,offset)
'''


class GuardAdapterTests(unittest.TestCase):
    def patched(self):
        temp = tempfile.TemporaryDirectory()
        directory = Path(temp.name)
        for name in ['psb_drv.c', 'accel_2d.c', 'power.c']:
            (directory/name).write_bytes((BASE/name).read_bytes())
        subprocess.run(['patch', '-p1', '--batch', '--forward', '-i',
                        str(ROOT/'kernel/sgx535_frozen/patches/antix-source-guard.patch')],
                       cwd=directory, check=True, capture_output=True)
        self.addCleanup(temp.cleanup)
        return directory

    def test_actual_backend_begin_consumes_lifecycle_not_clean_status(self):
        snapshot = 'static void ' + function(SNAPSHOT.read_text(), 'sgx535_gma500_source_snapshot')
        body = 'int ' + function(BACKEND.read_text(), 'sgx535_gma500_fixed_backend_begin')
        helpers = r'''
static struct sgx535_gma500_fixed_backend *fixed_irq_owner;
static int capsule_service_observer;
static unsigned admissions, covered;
static unsigned long sgx535_gma500_fixed_irq_lock(void) { return 0; }
static void sgx535_gma500_fixed_irq_unlock(unsigned long f) { (void)f; }
static int sgx535_provenance_source_begin(struct sgx535_gma500_fixed_backend *b,
    struct drm_device *d) { (void)d; return sgx535_source_begin(&guard,b); }
static int sgx535_provenance_source_boundary(struct sgx535_gma500_fixed_backend *b,
    const struct sgx535_source_facts *f) {
    struct sgx535_source_facts observed=*f; observed.coverage_complete=covered;
    return sgx535_source_boundary(&guard,b,&observed);
}
static void sgx535_provenance_admit(struct sgx535_gma500_fixed_backend *b) {
    if (sgx535_source_valid(&guard,b)) admissions++;
}
static void seed(void) {
    sgx535_source_boot_start(&guard,&clean); sgx535_source_producer(&guard,1);
    sgx535_source_boot_assert(&guard,SGX535_SOURCE_RESET_MASK);
    sgx535_source_boot_finish(&guard,&clean); sgx535_source_producer(&guard,0);
    sgx535_source_probe(&guard,&clean);
}
'''
        suffix = r'''
int main(void) {
    struct drm_psb_private priv={0}; struct drm_device dev={&priv};
    struct sgx535_gma500_owner owner={0};
    struct sgx535_gma500_fixed_backend b={0}; unsigned mode;
    priv.sgx_reg=&priv; priv.dev=&dev;
    owner.initialized=owner.device_claimed=owner.power_held=1;
    owner.session.phase=SGX535_PHASE_HOST_IMAGE_FINAL;
    owner.fixed_service.session=&owner.session; owner.dev=&dev;
    for (mode=0; mode<17; mode++) {
        memset(&guard,0,sizeof(guard)); memset(&b,0,sizeof(b));
        fixed_irq_owner=NULL; reads=admissions=0; covered=1;
        hardware_pending=hardware_busy=hardware_reset=hardware_reads=hardware_fault=hardware_timer=hardware_kick=0;
        priv.sgx_reg=&priv;
        if (mode!=1) seed();
        if (mode==2) hardware_pending=1U<<13;
        if (mode==3) hardware_pending=1U;
        if (mode==4) hardware_busy=_PSB_C2B_STATUS_BUSY;
        if (mode==5) priv.sgx_reg=NULL;
        if (mode==6) hardware_reads=1;
        if (mode==7) hardware_fault=1;
        if (mode==8) hardware_reset=1;
        if (mode==9) sgx535_source_pending(&guard,1U<<13); /* late prior IRQ */
        if (mode==10) sgx535_source_producer(&guard,1);
        if (mode==11) sgx535_source_lifecycle_lost(&guard);
        if (mode==12) b.pending_status2=16; /* delayed software source */
        if (mode==13) covered=0;
        if (mode==14) sgx535_source_lost(&guard);
        if (mode==15) hardware_timer=SGX535_SOURCE_EVENT_TIMER_ENABLE;
        if (mode==16) hardware_kick=SGX535_SOURCE_EVENT_KICK_NOW;
        if (sgx535_gma500_fixed_backend_begin(&b,&owner)!=(mode==0 ? 0 : -EAGAIN)) return 1;
        if (reads!=(mode==5 ? 0U : 9U)) return 2;
        if (mode==0) {
            if (!b.active || !b.owner || fixed_irq_owner!=&b || admissions!=1) return 3;
            sgx535_source_producer(&guard,1); sgx535_source_producer(&guard,0);
            if (sgx535_source_valid(&guard,&b)) return 4;
        } else {
            if (b.active || b.owner || b.power_held || fixed_irq_owner || admissions) return 5;
            if (sgx535_gma500_fixed_backend_begin(&b,&owner)!=-EBUSY) return 6;
        }
    }
    puts("17 actual backend cases PASS; no write/ACK/reset/submission APIs");
    return 0;
}
'''
        self.assertIn(b'17 actual backend cases PASS', compile_run(PREFIX + snapshot + helpers + body + suffix))

    def test_actual_existing_reset_and_delayed_work_model(self):
        source = (self.patched()/'accel_2d.c').read_text()
        snapshot = 'static void ' + function(SNAPSHOT.read_text(), 'sgx535_gma500_source_snapshot')
        spank = 'void ' + function(source, 'psb_spank')
        helpers = r'''
#define PSB_CR_BIF_CTRL 0xc00
#define PSB_CR_BIF_TWOD_REQ_BASE 0xcc4
#define _PSB_CB_CTRL_CLEAR_FAULT (1U<<4)
#define _PSB_CS_RESET_BIF_RESET 1U
#define _PSB_CS_RESET_TWOD_RESET 2U
#define _PSB_CS_RESET_DPM_RESET 4U
#define _PSB_CS_RESET_TA_RESET 8U
#define _PSB_CS_RESET_USE_RESET 16U
#define _PSB_CS_RESET_ISP_RESET 32U
#define _PSB_CS_RESET_TSP_RESET 64U
static unsigned queued_old, reset_writes, sleeps, reset_values[2];
static void fake_write(unsigned value,unsigned offset) {
    if (offset==PSB_CR_SOFT_RESET) {
        if (reset_writes<2) reset_values[reset_writes]=value;
        reset_writes++; hardware_reset=value;
        if (value==SGX535_SOURCE_RESET_MASK) queued_old=0;
    }
    /* No event register write is allowed, even in the reset harness. */
    if (offset==0x134 || offset==0x114 || offset==0xac8) queued_old=99;
}
#define PSB_WSGX32(value,offset) fake_write(value,offset)
static void wmb(void) { }
static void msleep(unsigned n) { if(n==1) sleeps++; }
static void sgx535_provenance_source_reset_begin(struct drm_device *d) {
    (void)d; if(guard.boot!=SGX535_BOOT_RESET_EXPECTED) sgx535_source_lifecycle_lost(&guard);
    sgx535_source_producer(&guard,1);
}
static void sgx535_provenance_source_reset_assert(struct drm_device *d,unsigned r) {
    (void)d; sgx535_source_boot_assert(&guard,r);
}
static void sgx535_provenance_source_reset_end(struct drm_device *d,const struct sgx535_source_facts *f) {
    (void)d; sgx535_source_boot_finish(&guard,f); sgx535_source_producer(&guard,0);
}
'''
        suffix = r'''
int main(void) {
    struct drm_psb_private priv={0}; struct drm_device dev={&priv};
    struct sgx535_source_facts before; unsigned mode;
    priv.sgx_reg=&priv; priv.dev=&dev;
    for(mode=0;mode<4;mode++) {
        memset(&guard,0,sizeof(guard)); reset_writes=sleeps=0; queued_old=1;
        hardware_pending=hardware_reads=hardware_fault=hardware_busy=hardware_reset=0;
        if(mode==1) hardware_pending=1U<<13;
        if(mode==2) hardware_pending=1U;
        if(mode==3) hardware_reads=1;
        sgx535_gma500_source_snapshot(&priv,&before);
        /* Apparently clean immediate observation cannot establish a seed. */
        if (sgx535_source_boot_valid(&guard)) return 1;
        sgx535_source_boot_start(&guard,&before);
        psb_spank(&priv); /* existing normal startup operation, modeled */
        if(queued_old || reset_writes!=2 || sleeps!=2 ||
           reset_values[0]!=0x7f || reset_values[1]!=0) return 2;
        if(sgx535_source_boot_valid(&guard)!=(mode==0)) return 3;
        if(mode && !guard.reasons) return 4; /* no reset-to-clean evidence */
    }
    (void)clean;
    puts("4 actual initialization-reset model cases PASS; SYNTHETIC ONLY");
    return 0;
}
'''
        self.assertIn(b'4 actual initialization-reset', compile_run(PREFIX + snapshot + helpers + spank + suffix))

    def test_no_new_hardware_writes_or_reset_calls_in_patch(self):
        patched = self.patched()
        def calls(source, token):
            result=[]
            for m in re.finditer(r'\b'+token+r'\s*\(', source):
                start=m.end(); end=start; depth=1
                while depth:
                    depth += (source[end]=='(')-(source[end]==')');end+=1
                result.append(re.sub(r'\s+','',source[start:end-1]))
            return result
        for name in ['psb_drv.c','accel_2d.c','power.c']:
            for token in ['PSB_WSGX32', 'psb_spank', 'msleep']:
                self.assertEqual(calls((BASE/name).read_text(),token), calls((patched/name).read_text(),token))
        self.assertLess((patched/'psb_drv.c').read_text().index('source_startup(dev'),
                        (patched/'psb_drv.c').read_text().index('psb_spank(dev_priv)'))
        power=(patched/'power.c').read_text()
        self.assertIn('sgx535_provenance_source_lifecycle(dev)',function(power,'gma_power_suspend'))
        self.assertIn('sgx535_provenance_source_lifecycle(dev)',function(power,'gma_power_resume'))

    def test_no_mmio_or_operation_when_reading_retained_guard(self):
        for name in ['source_open','source_read','sgx535_provenance_source_producer']:
            body=function(OBSERVER.read_text(),name)
            for forbidden in ['PSB_RSGX','PSB_WSGX','fixed_actions','psb_spank',
                              'sgx535_gma500_fixed_backend_begin','sgx535_capsule_issued']:
                self.assertNotIn(forbidden,body)
        self.assertIn('proc_create("sgx535_source_guard", 0400',OBSERVER.read_text())
        self.assertIn('startup_pending=%u',OBSERVER.read_text())

    def test_native_proof_is_retained_lifecycle_not_injected_boolean(self):
        core=(ROOT/'tools/psb-dri-re/frozen_source_guard.c').read_text()
        self.assertIn('sgx535_source_boot_valid(g)',function(core,'sgx535_source_boundary'))
        self.assertNotIn('delayed_excluded',(ROOT/'tools/psb-dri-re/frozen_source_guard.h').read_text())
        self.assertIn('capsule.state != SGX535_CAP_UNUSED',function(OBSERVER.read_text(),'sgx535_provenance_source_startup'))
        self.assertIn('sgx535_provenance_source_pending(dev',function(BACKEND.read_text(),'sgx535_gma500_fixed_irq_capture_locked'))
        write=function(BACKEND.read_text(),'fixed_write')
        self.assertLess(write.index('sgx535_provenance_source_write'),write.index('PSB_WSGX32'))

    def test_actual_preparation_reader_pins_and_serializes_without_admission(self):
        o=OBSERVER.read_text(); b=BACKEND.read_text()
        parts=r'''
#include <stdlib.h>
#include <stdarg.h>
#define SGX535_CAP_UNUSED 0
#define SGX535_SOURCE_UNUSED 0
#define SGX535_CAPSULE_BYTES 4152
#define GFP_KERNEL 0
struct module { int live; };
struct inode { int unused; };
struct file { void *private_data; };
typedef void (*sgx535_source_reader)(struct drm_device *);
typedef void (*sgx535_source_snapshot_fn)(struct drm_device *,struct sgx535_source_facts *);
static struct { unsigned state; } capsule;
static void *call_task;
static struct drm_device *source_device;
static struct module *source_module;
static sgx535_source_reader source_reader;
#define source_guard guard
static int capsule_lock, irq_lock, pin_count, pin_allowed;
#define spin_lock_irqsave(lock,flags) do { if(*(lock)) abort(); *(lock)=1; flags=0; } while(0)
#define spin_unlock_irqrestore(lock,flags) do { *(lock)=0; (void)(flags); } while(0)
static unsigned long sgx535_gma500_fixed_irq_lock(void) {
    if(capsule_lock || irq_lock) abort();
    irq_lock=1; return 0;
}
static void sgx535_gma500_fixed_irq_unlock(unsigned long f) { (void)f; irq_lock=0; }
static int try_module_get(struct module *m) { if(!m || !m->live || !pin_allowed) return 0; pin_count++; return 1; }
static void module_put(struct module *m) { (void)m; pin_count--; }
static void *kzalloc(unsigned n,int f) { (void)f; return calloc(1,n); }
static void kfree(void *p) { free(p); }
static int scnprintf(char *p,unsigned n,const char *fmt,...) {
    va_list a; int r; va_start(a,fmt); r=vsnprintf(p,n,fmt,a); va_end(a); return r;
}
'''
        code=PREFIX+parts+'static void '+function(SNAPSHOT.read_text(),'sgx535_gma500_source_snapshot')
        code+='static void '+function(b,'source_snapshot_locked')
        code+='void '+function(o,'sgx535_provenance_source_observe')
        code+='void '+function(b,'sgx535_gma500_source_observe_passive')
        code+='static int '+function(o,'source_open')
        code+=r'''
int main(void) {
    struct drm_psb_private priv={0}; struct drm_device dev={&priv};
    struct module module={1}; struct inode node={0}; struct file file={0};
    priv.dev=&dev; priv.sgx_reg=&priv;
    source_device=&dev;source_module=&module;source_reader=sgx535_gma500_source_observe_passive;
    sgx535_source_boot_start(&guard,&clean);sgx535_source_producer(&guard,1);
    sgx535_source_boot_assert(&guard,127);sgx535_source_boot_finish(&guard,&clean);sgx535_source_producer(&guard,0);
    pin_allowed=0;
    if(source_open(&node,&file)!=-EAGAIN || reads || guard.prepared || pin_count) return 1;
    pin_allowed=1;
    if(source_open(&node,&file) || reads!=9 || !guard.prepared || pin_count || irq_lock || capsule_lock) return 2;
    if(!strstr(file.private_data,"prepared=1") || !strstr(file.private_data,"STARTUP_LIFECYCLE")) return 3;
    free(file.private_data);
    /* Interference after preparation poisons the interval; later clean reads
     * cannot rearm it. Reading retained failed evidence cannot dereference it. */
    sgx535_source_producer(&guard,1);sgx535_source_producer(&guard,0);
    priv.sgx_reg=NULL;
    if(source_open(&node,&file) || reads!=9 || !strstr(file.private_data,"UNPROVEN")) return 4;
    free(file.private_data);
    puts("actual preparation reader PASS; no admission, writes, ACK or wake");
    return 0;
}
'''
        self.assertIn(b'actual preparation reader PASS',compile_run(code))

    def test_interval_extends_from_boundary_through_public_call_close(self):
        o=OBSERVER.read_text()
        self.assertIn('sgx535_source_valid(&source_guard, b)',function(o,'sgx535_provenance_admit'))
        self.assertIn('capsule.invalid = 1',function(o,'sgx535_provenance_source_producer'))
        close=function(o,'syscall_exit')
        self.assertLess(close.index('sgx535_source_close'),close.index('sgx535_capsule_close'))

    def test_actual_write_gate_rejects_wrong_producer_or_lost_interval(self):
        code=PREFIX+r'''
static int capsule_lock;
static void *call_task, *current;
static struct { int invalid; void *backend, *owner; } capsule;
#define source_guard guard
#define spin_lock_irqsave(lock,flags) do { *(lock)=1; flags=0; } while(0)
#define spin_unlock_irqrestore(lock,flags) do { *(lock)=0; (void)(flags); } while(0)
'''
        code+='int '+function(OBSERVER.read_text(),'sgx535_provenance_source_write')
        code+=r'''
int main(void) {
    struct sgx535_gma500_fixed_backend b={0}; int owner,other; unsigned mode;
    struct drm_psb_private priv={0};
    for(mode=0;mode<6;mode++) {
        memset(&guard,0,sizeof(guard));
        sgx535_source_boot_start(&guard,&clean);sgx535_source_producer(&guard,1);
        sgx535_source_boot_assert(&guard,127);sgx535_source_boot_finish(&guard,&clean);sgx535_source_producer(&guard,0);
        sgx535_source_probe(&guard,&clean);sgx535_source_begin(&guard,&b);sgx535_source_boundary(&guard,&b,&clean);
        b.owner=(void*)&owner;capsule.owner=&owner;capsule.backend=&b;capsule.invalid=0;
        current=call_task=&owner;
        if(mode==1) capsule.invalid=1;
        if(mode==2) capsule.owner=&other;
        if(mode==3) capsule.backend=&other;
        if(mode==4) current=&other;
        if(mode==5) sgx535_source_producer(&guard,1);
        if(sgx535_provenance_source_write(&b)!=(mode==0)) return 1;
        if(mode && !capsule.invalid) return 2;
    }
    (void)passive_read(&priv,0);
    (void)gma_power_is_on(NULL);
    puts("6 actual write-gate cases PASS; no hardware writes");return 0;
}
'''
        self.assertIn(b'6 actual write-gate cases PASS',compile_run(code))


if __name__=='__main__':
    unittest.main(verbosity=2)
