#!/usr/bin/env python3
"""Qualify new CPU cube reference only; does not rerun square qualification."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import cube_reference as c

def main():
    ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--workspace',type=Path,required=True)
    w=ap.parse_args().workspace.resolve();out=w/'CPU-tests';out.mkdir()
    repo=Path(__file__).resolve().parents[2]
    root=Path('/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root')
    qemu=Path('/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/tests/qemu-root/usr/bin/qemu-i386')
    env={**os.environ,'LD_LIBRARY_PATH':str(root/'usr/lib/aarch64-linux-gnu'),'PYTHONDONTWRITEBYTECODE':'1','LC_ALL':'C'}
    def run(name,cmd,input=None,success=True):
        p=subprocess.run(list(map(str,cmd)),input=input,capture_output=True,env=env)
        (out/(name+'.stdout')).write_bytes(p.stdout);(out/(name+'.stderr')).write_bytes(p.stderr)
        (out/(name+'.command.json')).write_text(json.dumps({'argv':list(map(str,cmd)),'returncode':p.returncode,'success_expected':success},indent=2)+'\n')
        if (p.returncode==0)!=success:raise RuntimeError(name+': '+p.stderr.decode(errors='replace'))
        return p.stdout
    run('new-python-tests',[sys.executable,'-B','-m','unittest','discover','-s',repo/'tools/sgx535_demo/cube_cpu/tests','-p','test_cube_reference.py','-v'])
    fixture=['5'];expected=b''
    for angle in (0,15,30,45,60):
        f=c.frame(angle)
        for m in (f['model'],f['view'],f['projection']):
            fixture += [format(x,'.17g') for row in m for x in row]
        for v in c.VERTICES:fixture += [format(x,'.17g') for x in (*v,1)]
        expected+=c.pack_vertices(f['vertices'])
    raw=('\n'.join(fixture)+'\n').encode();(out/'matrix-input.txt').write_bytes(raw)
    (out/'expected-screen-vertices.bin').write_bytes(expected)
    results={}
    for name,compiler,flags in [('native',['cc'],['-O2']),('UBSan',['cc'],['-O1','-fsanitize=undefined','-fno-sanitize-recover=all']),
                             ('i386',[root/'usr/bin/i686-linux-gnu-gcc-14','--sysroot='+str(root)],['-O2','-static','-march=i486'])]:
        exe=out/('projection-'+name)
        run('compile-'+name,compiler+flags+['-std=c11','-Wall','-Wextra','-Werror','-ffp-contract=off',repo/'tools/sgx535_demo/cube_projection_oracle.c','-o',exe,'-lm'])
        cmd=[qemu,exe] if name=='i386' else [exe]
        got=run('projection-'+name,cmd,raw)
        assert got==expected,name+' differs from CPU float32 reference'
        for case,data in [('wrong-count',b'6\n'),('truncated',b'5\n'),('nonfinite',b'5\nnan\n')]:run(name+'-'+case,cmd,data,False)
        results[name]={'float32_vertices':40,'bytes':len(got),'sha256':hashlib.sha256(got).hexdigest(),'negative_cases':3,'result':'PASS'}
    manifest=c.generate(w/'CPU-reference')
    report={'scope':'CPU math/clipping/packing/reference only; not SGX qualification','new_python_tests':20,'projection_and_packing':results,
            'reference_frames':5,'reference_frame_hashes':[f['ARGB_sha256'] for f in manifest['frames']],
            'GPU_candidate_produced':False,'SGX_invocations':0,'hardware_interactions':0,'completed_square_qualification_repeated':False}
    (w/'CPU-qualification.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))

if __name__=='__main__':main()
