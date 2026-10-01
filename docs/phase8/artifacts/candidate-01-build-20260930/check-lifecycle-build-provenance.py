from pathlib import Path
import json,hashlib,subprocess,sys
r=Path('/home/gama/sgx535-gfx');w=Path('/home/gama/sgx535-offline/candidate-01-build-20260930');o=Path('/home/gama/sgx535-offline/antix-kbuild-successor-20260930/output');label=sys.argv[1]
def h(p):return hashlib.sha256(p.read_bytes()).hexdigest()
state=json.loads((w/'prepared-state-before.json').read_text());changed={rel:{'before':sha,'after':h(o/rel)} for rel,sha in state.items() if h(o/rel)!=sha};assert not changed,changed
new={}
for base in ('include/config','include/generated','arch/x86/include/generated'):
 for p in (o/base).rglob('*'):
  if p.is_file() and str(p.relative_to(o)) not in state:new[str(p.relative_to(o))]=h(p)
assert not new,new
assert h(o/'Module.symvers')=='faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca'
log=(w/(label+'.stdout')).read_text();err=(w/(label+'.stderr')).read_text();assert not err,err
assert ' -m32 ' in log and ' -march=i486 ' in log and 'i686-linux-gnu-gcc-14 --sysroot=' in log
assert 'i686-linux-gnu-ld.bfd -m elf_i386' in log
assert 'scripts/mod/modpost -m' in log and '-e -i Module.symvers' in log
assert 'vmlinux.symvers' not in log and 'clang' not in log.lower() and 'WARNING:' not in log and 'warning:' not in log
env=json.loads((w/'build-environment.json').read_text());helpers=[]
for rel in ('scripts/mod/modpost','scripts/basic/fixdep','scripts/genksyms/genksyms'):
 p=o/rel;data=p.read_bytes();assert data[:7]==b'\x7fELF\x02\x01\x01' and int.from_bytes(data[18:20],'little')==183
 helpers.append({'path':str(p),'sha256':h(p),'elf':'ELF64 AArch64'})
manifest=json.loads((w/'integrated-source-manifest-lifecycle-final.json').read_text())
for rel,sha in manifest.items():assert h(w/'module'/rel)==sha,rel
result={'build':label,'config_generated_state_unchanged':len(state),'new_config_generated_files':new,'target_table_sha256':h(o/'Module.symvers'),'native_host_helpers':helpers,'target_gcc_m32_i486':True,'target_gnu_bfd_elf_i386':True,'modpost_external_e_i_target_table':True,'old_vmlinux_table_not_consumed':True,'no_build_warnings':True,'integrated_source_files_match':len(manifest)}
(w/(label+'-provenance.json')).write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
