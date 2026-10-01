from pathlib import Path
import json,subprocess,hashlib,os,sys,struct,importlib.util
r=Path('/home/gama/sgx535-gfx');w=Path('/home/gama/sgx535-offline/candidate-01-build-20260930');label=sys.argv[1];ko=Path(sys.argv[2]);d=w/label;d.mkdir(exist_ok=True);env=json.loads((w/'build-environment.json').read_text());table=r/'docs/hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifacts/build_Module_symvers'
def h(p):return hashlib.sha256(p.read_bytes()).hexdigest()
assert h(table)=='faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca'
b=ko.read_bytes();assert b[:7]==b'\x7fELF\x01\x01\x01';assert struct.unpack_from('<HH',b,16)==(1,3)
shutil=None
(d/'gma500_gfx.ko').write_bytes(b)
outputs={}
for name,args in [('elf-header',[env['CROSS_COMPILE']+'readelf','-h',str(ko)]),('elf-sections',[env['CROSS_COMPILE']+'readelf','-SW',str(ko)]),('elf-notes',[env['CROSS_COMPILE']+'readelf','-n',str(ko)]),('elf-symbols',[env['CROSS_COMPILE']+'readelf','-sW',str(ko)]),('undefined-symbols',[env['CROSS_COMPILE']+'nm','-u',str(ko)]),('modinfo',['/usr/sbin/modinfo',str(ko)]),('vermagic',['/usr/sbin/modinfo','-F','vermagic',str(ko)]),('dependencies',['/usr/sbin/modinfo','-F','depends',str(ko)]),('target-table-check',['python3',str(r/'tools/psb-dri-re/frozen_module_versions.py'),'--check-symvers',str(ko),str(table)])]:
 p=subprocess.run(args,env=env,capture_output=True,text=True);(d/(name+'.stdout')).write_text(p.stdout);(d/(name+'.stderr')).write_text(p.stderr);outputs[name]={'argv':args,'exit_code':p.returncode};assert p.returncode==0,(name,p.stdout,p.stderr)
(d/'commands.json').write_text(json.dumps(outputs,indent=2)+'\n')
check=json.loads((d/'target-table-check.stdout').read_text());assert check['mismatched_count']==0 and not check['missing_imports'];assert check['module_layout']['reference']=='0xb84efb99'
vermagic=(d/'vermagic.stdout').read_text().rstrip('\n');assert vermagic.rstrip()=='5.10.240-antix.1-486-smp SMP mod_unload modversions 486'
spec=importlib.util.spec_from_file_location('versions',r/'tools/psb-dri-re/frozen_module_versions.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
versions,_=m.read_versions(ko);original,_=m.read_versions(r/'docs/hardware-evidence/MINI12-20260927-H0/raw/installed-gma500_gfx.ko');(d/'import-crcs.json').write_text(json.dumps({s:f'0x{crc:08x}' for s,crc in sorted(versions.items())},indent=2)+'\n')
import re
bid=re.search(r'Build ID: (\w+)',(d/'elf-notes.stdout').read_text());assert bid
result={'path':str(ko),'preserved_path':str(d/'gma500_gfx.ko'),'size':len(b),'sha256':h(ko),'build_id':bid[1],'elf':'ELF32 little-endian ET_REL EM_386','vermagic':vermagic,'module_layout':'0xb84efb99','imports_total':len(versions),'imports_covered':len(versions),'missing_imports':0,'crc_mismatches':0,'candidate_only_imports':sorted(set(versions)-set(original)),'removed_imports':sorted(set(original)-set(versions)),'dependencies':(d/'dependencies.stdout').read_text().strip(),'module_signature_trailer_present':b.endswith(b'~Module signature appended~\n'),'target_table_sha256':h(table),'abi_import_guard':'PASS','full_qualification':'PENDING provenance/regression/repeat checks'}
(d/'identity.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result,indent=2))
