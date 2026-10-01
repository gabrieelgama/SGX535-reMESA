from pathlib import Path
import subprocess,json,hashlib,os,sys,datetime
r=Path('/home/gama/sgx535-gfx');w=Path(__file__).parent
checks=[]
def run(name,args):
 cp=subprocess.run(args,cwd=r,capture_output=True,env=dict(os.environ,PYTHONDONTWRITEBYTECODE='1'))
 (w/(name+'.stdout')).write_bytes(cp.stdout);(w/(name+'.stderr')).write_bytes(cp.stderr)
 checks.append({'name':name,'argv':args,'start_result_recorded_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'exit_code':cp.returncode});(w/'build-commands.json').write_text(json.dumps(checks,indent=2)+'\n')
 assert cp.returncode==0,(name,cp.stderr.decode())
 return cp
c=r/'docs/hardware-evidence/MINI12-20261001T030754Z-BOOT-PROVENANCE-READONLY-02'
inputs=[]
for rel in [c/'decoded-files/release_initrd',c/'decoded-files/_boot_grub_grub_cfg',c/'decoded-files/_boot_grub_grubenv',r/'docs/hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifacts/build_Module_symvers',r/'docs/hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifacts/boot-config',r/'docs/hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifacts/build__config',r/'docs/phase8/artifacts/candidate-01-build-20260930/candidate-02-lifecycle/gma500_gfx.ko',r/'docs/phase8/artifacts/candidate-01-build-20260930/candidate-01/gma500_gfx.ko',r/'tools/psb-dri-re/frozen_first_load_image.py',r/'kernel/sgx535_frozen/first_load/preload.sh.in']:
 data=rel.read_bytes();inputs.append({'path':str(rel.relative_to(r)),'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest()})
(w/'inputs.json').write_text(json.dumps({'files':inputs,'kernel_image_hash_only':'cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438','kernel_image_source':'capture02 transcript KERNEL_IMAGE_METADATA; binary was not captured','framework':'captured initramfs-tools0.148.3 /init retained except one guarded insertion','stock_members':2138,'target_contact':False},indent=2)+'\n')
for n in [1,2]:
 cp=run('build-'+str(n),['python3','tools/psb-dri-re/frozen_first_load_image.py','--output',str(w/f'build-0{n}')])
 print(cp.stdout.decode())
a=(w/'build-01/initrd.img-sgx535-firstload-01').read_bytes();b=(w/'build-02/initrd.img-sgx535-firstload-01').read_bytes()
assert a==b
(w/'repeat-build.json').write_text(json.dumps({'result':'BYTE-IDENTICAL','build1':str(w/'build-01'),'build2':str(w/'build-02'),'bytes':len(a),'sha256':hashlib.sha256(a).hexdigest()},indent=2)+'\n')
run('hook-shell-syntax',['/bin/sh','-n',str(w/'build-01/sgx535-first-load.sh')])
run('init-shell-syntax',['/bin/sh','-n',str(w/'build-01/modified-init')])
print('Two isolated offline image builds byte-identical; shell syntax PASS.')
