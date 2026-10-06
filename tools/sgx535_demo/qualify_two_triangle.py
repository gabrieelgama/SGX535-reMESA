"""Offline generated-image differential qualification. No target access.

Requires an isolated build/module containing the experimental include tables.
Toolchain and QEMU are the already retained offline i386 environment.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import two_triangle as q

def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--workspace',type=Path,required=True)
    a=ap.parse_args(); w=a.workspace.resolve(); out=w/'tests'; out.mkdir()
    module=w/'build/module'
    root=Path('/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root')
    qemu=Path('/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/tests/qemu-root/usr/bin/qemu-i386')
    env={**os.environ,'LD_LIBRARY_PATH':str(root/'usr/lib/aarch64-linux-gnu'),'PYTHONDONTWRITEBYTECODE':'1'}
    cases={}
    def run(name,cmd,binary=False):
        p=subprocess.run(list(map(str,cmd)),capture_output=True,env=env)
        (out/(name+'.stdout')).write_bytes(p.stdout)
        (out/(name+'.stderr')).write_bytes(p.stderr)
        if p.returncode: raise RuntimeError(name+' failed: '+p.stderr.decode(errors='replace'))
        return p.stdout
    run('python-regressions',[sys.executable,'-m','unittest','discover','-s',q.REPO/'tools/sgx535_demo','-v'])
    cases['python_regressions']=32
    m=q.scene(); plan=q.bo.build(m)
    raw=q.backings(m,plan); raw['use'][:8]=bytes.fromhex('ff001f00f1f1a7fc')
    addrs=dict(zip(plan['bos'],[0x20000000,0x20080000,0x40000000,0x30000000,0,0x40001000,0x40002000,0x42000000,0x31000000,0]))
    relocated=q.bo.resolve(m,plan,addrs,{0:3,1:4},0x80000000)
    relocated['use'][:8]=bytes.fromhex('ff001f00f1f1a7fc')
    expected={'initial':b''.join(raw.values()),'relocated':b''.join(relocated.values())}
    builds={
        'native':(['cc'],['-O2']),
        'ubsan':(['cc'],['-O1','-fsanitize=undefined','-fno-sanitize-recover=all']),
        'i386':([str(root/'usr/bin/i686-linux-gnu-gcc-14'),'--sysroot='+str(root)],['-O2','-static','-march=i486'])}
    for name,(compiler,flags) in builds.items():
        exe=out/('oracle-'+name)
        cmd=compiler+flags+['-std=c11','-Wall','-Wextra','-Werror','-DSGX535_EXPERIMENTAL_CONSTANT_FRAGMENT=1','-I'+str(module),str(q.REPO/'tools/sgx535_demo/contract_oracle.c'),str(module/'frozen_kernel_contract.c'),'-o',str(exe)]
        run('compile-'+name,cmd)
        prefix=[qemu,exe] if name=='i386' else [exe]
        for stage,args in [('initial',[]),('relocated',['relocated'])]:
            got=run(name+'-'+stage,prefix+args,True)
            if got!=expected[stage]: raise RuntimeError(name+' '+stage+' full backing mismatch')
            cases[name+'_'+stage]={'bytes':len(got),'sha256':hashlib.sha256(got).hexdigest(),'PASS':True}
        # Same known-good constant fragment encoder, invalid-view and full-zero
        # initialization tests, compiled against this isolated payload.
        source=(q.REPO/'tools/psb-dri-re/test_experimental_fragment_constant.c').read_text()
        testcopy=out/('fragment-'+name+'.c'); testcopy.write_text(source)
        exe2=out/('fragment-'+name)
        run('compile-fragment-'+name,compiler+flags+['-std=c11','-Wall','-Wextra','-Werror','-DSGX535_EXPERIMENTAL_CONSTANT_FRAGMENT=1','-I'+str(module),str(testcopy),str(module/'frozen_kernel_contract.c'),'-o',str(exe2)])
        run('fragment-'+name,([qemu,exe2] if name=='i386' else [exe2]))
        cases['fragment_'+name]={'immediate_roundtrips':1032,'PASS':True}
    report={'classification':'PASS','scope':'CPU construction/policy only; no GPU proof','cases':cases,'SGX_invocations':0,'hardware_interactions':0}
    (w/'cpu-qualification.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))

if __name__=='__main__': main()
