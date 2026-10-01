from pathlib import Path
import hashlib,json,os,subprocess,sys,zlib
from read_initramfs import read_image
r=Path('/home/gama/sgx535-gfx'); w=Path(__file__).parent
meta=json.loads(Path('/tmp/sgx535-current-boot-capture-paths.json').read_text()); e=Path(meta['evidence'])
image=e/'decoded-files/release_initrd'; data=image.read_bytes()
records,containers=read_image(data)
# Independent GNU cpio listing of both actually observed archive members.
streams=[('early',data[:containers[0]['bytes']])]
streams.append(('main',zlib.decompress(data[containers[1]['offset']:],31)))
validation=[]
cpio=Path('/usr/bin/cpio')
for name,payload in streams:
 argv=[str(cpio),'-it','--quiet']
 cp=subprocess.run(argv,input=payload,capture_output=True)
 (w/(name+'-cpio-list.stdout')).write_bytes(cp.stdout);(w/(name+'-cpio-list.stderr')).write_bytes(cp.stderr)
 assert cp.returncode==0,cp.stderr.decode()
 # Catalog order must exactly match the independent GNU parser.
 native=cp.stdout.decode().splitlines()
 expected=[p['name'] for p in records[:5]] if name=='early' else [p['name'] for p in records[5:]]
 assert native==expected,(name,'native parser disagrees')
 validation.append({'archive':name,'argv':argv,'exit_code':cp.returncode,'members':len(native),'exact_catalog_agreement':True})
(w/'native-cpio-verification.json').write_text(json.dumps({'cpio_sha256':hashlib.sha256(cpio.read_bytes()).hexdigest(),'cpio_version':subprocess.check_output([str(cpio),'--version'],text=True).splitlines()[0],'checks':validation},indent=2)+'\n')
# Preserve the complete captured stock gma500 module dependency closure, not a
# guessed or rebuilt replacement for it. All files come from the hash-verified
# initramfs. Original itself is absent, as separately recorded.
root='usr/lib/modules/5.10.240-antix.1-486-smp/'
line=next(p for p in (e/'decoded-files/modules_modules_dep').read_text().splitlines() if p.startswith('kernel/drivers/gpu/drm/gma500/gma500_gfx.ko:'))
needed=line.split(':',1)[1].split()
lookup={p['name']:p for p in records}
table=r/'docs/hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifacts/build_Module_symvers'
assert hashlib.sha256(table.read_bytes()).hexdigest()=='faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca'
results=[]
for relative in needed:
 entry=lookup[root+relative]
 target=w/'dependency-modules'/relative
 assert not target.exists()
 target.parent.mkdir(parents=True,exist_ok=True); target.write_bytes(entry['data'])
 args=['python3',str(r/'tools/psb-dri-re/frozen_module_versions.py'),'--check-symvers',str(target),str(table)]
 cp=subprocess.run(args,capture_output=True,env=dict(os.environ,PYTHONDONTWRITEBYTECODE='1'))
 label=Path(relative).stem
 (w/(label+'-crc-check.stdout')).write_bytes(cp.stdout);(w/(label+'-crc-check.stderr')).write_bytes(cp.stderr)
 assert cp.returncode==0,cp.stderr.decode()
 report=json.loads(cp.stdout)
 assert report['module_layout']['reference']=='0xb84efb99' and not report['missing_imports'] and report['mismatched_count']==0
 results.append({'initramfs_member':root+relative,'sha256':entry['sha256'],'bytes':entry['bytes'],'checker_argv':args,'checker_exit_code':cp.returncode,'versioned_imports':report['reference_import_count'],'module_layout':'0xb84efb99','missing':0,'crc_mismatches':0})
(w/'dependency-coverage.json').write_text(json.dumps({'original_modules_dep_line':line,'count':len(needed),'target_table_sha256':hashlib.sha256(table.read_bytes()).hexdigest(),'result':'PASS FOR COMPLETE STOCK DEPENDENCY CLOSURE','modules':results},indent=2)+'\n')
print('Independent GNU cpio agrees with all 2138 catalog members.')
print('Captured initramfs contains all',len(needed),'stock gma500 dependency modules; every versioned import matches target table.')
print([(Path(p['initramfs_member']).name,p['versioned_imports']) for p in results])
