#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Compile an actual Mesa NIR-builder client against the qualified i386 cache."""
import argparse,json,os,subprocess
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('--cache',type=Path,required=True);p.add_argument('--test',choices=['test_vertex_deref_builder.c','test_vertex_builder.c','test_nir_builder.c'],default='test_vertex_deref_builder.c');p.add_argument('--output',type=Path,required=True);a=p.parse_args();a.output.mkdir(parents=True,exist_ok=True)
repo=Path(__file__).resolve().parents[3];m=a.cache;b=m/'build-i386';env=dict(os.environ);env.update(json.loads((m/'build-environment.json').read_text()))
commands=json.loads((b/'compile_commands.json').read_text());import shlex
record=next(x for x in commands if x['file'].endswith('/remesa_nir.c'));args=record.get('arguments') or shlex.split(record['command']);filtered=[];skip=False
for arg in args:
 if skip:skip=False;continue
 if arg in ('-o','-MF','-MQ'):skip=True;continue
 if arg in ('-c','-MD') or arg==record['file']:continue
 filtered.append(arg)
filtered.insert(1,'-I'+str(repo/'mesa/sgx535'))
filtered.append('-UNDEBUG')
exe=a.output/'nir-builder-i386';cmd=filtered+[str(repo/'mesa/sgx535/tests'/a.test),str(repo/'mesa/sgx535/remesa_nir.c'),'-I'+str(repo/'mesa/sgx535'),'-Wl,--start-group']+[str(b/x) for x in ['src/compiler/nir/libnir.a','src/compiler/libcompiler.a','src/util/libmesa_util.a']]+['-Wl,--end-group',str(m/'build-tools/native/usr/i686-linux-gnu/lib/libstdc++.so.6.0.33'),'-lm','-ldl','-latomic','-pthread','-o',str(exe)]
r=subprocess.run(cmd,cwd=b,env=env,capture_output=True,text=True);(a.output/'compile.log').write_text(r.stdout+r.stderr)
if r.returncode:raise SystemExit('Actual builder compile failed')
q='/home/gama/sgx535-offline/phase8-single-color-hypothesis-20261006T034310Z/tests/qemu-root/usr/bin/qemu-i386';r=subprocess.run([q,'-L',str(m/'runtime'),str(exe)],env=env,capture_output=True,text=True);(a.output/'run.log').write_text(r.stdout+r.stderr)
(a.output/'result.json').write_text(json.dumps({'PASS':r.returncode==0,'command':cmd,'output':r.stdout+r.stderr,'scope':'actual Mesa builder; no GPU'},indent=2)+'\n')
print(r.stdout+r.stderr);raise SystemExit(r.returncode)
