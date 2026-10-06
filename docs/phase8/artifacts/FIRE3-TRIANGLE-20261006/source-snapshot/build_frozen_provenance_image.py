"""Rebuild the existing qualified module pair with a correctly bound first-load hook."""
from pathlib import Path
import sys,json,hashlib,gzip,zlib,stat,subprocess,argparse
r=Path(__file__).resolve().parents[2]
p=argparse.ArgumentParser(description='Offline provenance image rebuild; no target operations')
p.add_argument('--qualified-root',type=Path,required=True)
p.add_argument('--output',type=Path,required=True)
a=p.parse_args();q=a.qualified_root.resolve();e=a.output.resolve()
sys.path.insert(0,str(r/'tools/psb-dri-re'))
import frozen_first_load_image as f
identity=json.loads((q/'qualified-build-01/identity.json').read_text())
old=Path('/home/gama/sgx535-offline/request-byte-decoding-20261002T222753Z/image/build-01')
old_info=json.loads((old/'identity.json').read_text())
data=(old/'initrd.img-sgx535-firstload-diagnostic-01').read_bytes()
assert len(data)==old_info['image_bytes'] and f.sha(data)==old_info['image_sha256']
prefix=data[:old_info['early_prefix_bytes']];raw=gzip.decompress(data[len(prefix):]);rows,end=f.scan(raw)
assert not any(raw[end:]);lookup={row['name']:row for row in rows}
driver_path=f.PAYLOAD;observer_path='usr/lib/sgx535-first-load/sgx535_provenance.ko'
assert observer_path not in lookup
payloads={driver_path:(q/'qualified-build-01/gma500_gfx.ko').read_bytes(),observer_path:(q/'qualified-build-01/sgx535_provenance.ko').read_bytes()}
hook=lookup[f.HOOK]['data'].decode()
assert hook.count(old_info['derivative_sha256'])==1 and hook.count(old_info['loaded_note_sha256'])==1
hook=hook.replace(old_info['derivative_sha256'],identity['driver']['sha256']).replace(old_info['loaded_note_sha256'],identity['driver']['loaded_note_sha256'])
line='    for name in drm syscopyarea sysfillrect sysimgblt fb_sys_fops cec drm_kms_helper video i2c_algo_bit gma500_gfx; do'
assert hook.count(line)==1
hook=hook.replace(line,line.replace('gma500_gfx;','sgx535_provenance gma500_gfx;'))
check='    sgx_hash_ok /'+driver_path
assert hook.count(check)==1
hook=hook.replace(check,'    sgx_hash_ok /'+observer_path+' '+identity['observer']['sha256']+' || { sgx_refuse payload-sgx535_provenance; return 1; }\n'+check)
insert='    sgx_insert_file /'+driver_path
assert hook.count(insert)==1
new='    sgx_insert_file /'+observer_path+' || { sgx_refuse insertion-sgx535_provenance; return 1; }\n'
new+='    state=$(sgx_read_file /sys/module/sgx535_provenance/initstate) || return 1\n'
new+='    [ "$state" = live ] || { sgx_refuse provenance-not-live; return 1; }\n'
new+='    sgx_hash_ok /sys/module/sgx535_provenance/notes/.note.gnu.build-id '+identity['observer']['loaded_note_sha256']+' || { sgx_refuse provenance-identity; return 1; }\n'
hook=hook.replace(insert,new+insert);payloads[f.HOOK]=hook.encode()
# Bind the enclosing integrity gate to these exact final hook bytes.
payloads['init']=f.bind_init_hook(lookup['init']['data'],payloads[f.HOOK])
for key,path in [('driver',driver_path),('observer',observer_path)]:
 assert len(payloads[path])==identity[key]['bytes'] and f.sha(payloads[path])==identity[key]['sha256']
rebuilt=b''.join(f.record(row['name'],payloads[row['name']],row['fields']) if row['name'] in payloads else row['raw'] for row in rows)
fields=[max(row['fields'][0] for row in rows)+1,stat.S_IFREG|0o644,0,0,1,0,0,*lookup['init']['fields'][7:11],0,0]
rebuilt+=f.record(observer_path,payloads[observer_path],fields)+f.record('TRAILER!!!',b'')
rebuilt+=b'\0'*((-len(rebuilt))%512)
outputs=[]
for n in (1,2):
 output=prefix+gzip.compress(rebuilt,compresslevel=9,mtime=0)
 target=e/f'image-build-0{n}';target.mkdir()
 image=target/('initrd.img-sgx535-provenance-'+identity['driver']['build_id'])
 image.write_bytes(output);(target/'sgx535-first-load.sh').write_text(hook)
 saved=image.read_bytes();decoded=gzip.decompress(saved[len(prefix):]);newrows,stop=f.scan(decoded);newlookup={row['name']:row for row in newrows}
 assert saved[:len(prefix)]==prefix and not any(decoded[stop:])
 assert set(newlookup)==set(lookup)|{observer_path}
 for name,row in lookup.items():
  assert newlookup[name]['data']==payloads[name] if name in payloads else newlookup[name]['raw']==row['raw']
 assert newlookup[observer_path]['data']==payloads[observer_path]
 packaged_hook_hash=f.verify_init_hook(newlookup['init']['data'],newlookup[f.HOOK]['data'])
 (target/'modified-init').write_bytes(newlookup['init']['data'])
 assert hook.index('sgx_insert_file /'+observer_path)<hook.index('sgx_insert_file /'+driver_path)
 assert subprocess.run(['/bin/sh','-n',str(target/'sgx535-first-load.sh')]).returncode==0
 info={'path':str(image),'bytes':len(saved),'sha256':f.sha(saved),'early_prefix_bytes':len(prefix),'early_prefix_sha256':f.sha(prefix),'base_image_sha256':old_info['image_sha256'],'driver':identity['driver'],'observer':identity['observer'],'changed_members':['init',driver_path,f.HOOK],'packaged_hook_sha256':packaged_hook_hash,'init_sha256':f.sha(newlookup['init']['data']),'init_hook_binding':'PASS FROM FINISHED IMAGE','added_members':[observer_path],'all_other_member_raw_bytes_unchanged':True,'driver_after_observer':True,'hook_shell_syntax':'PASS','loaded_on_target':False,'boot_id':None,'sgx_execution_authorized':False}
 (target/'identity.json').write_text(json.dumps(info,indent=2)+'\n');outputs.append(saved)
 print(n,len(saved),f.sha(saved),flush=True)
assert outputs[0]==outputs[1]
(e/'image-reproducibility.json').write_text(json.dumps({'byte_identical':True,'sha256':f.sha(outputs[0]),'scope':'offline image construction/unpack/order/syntax only; not live first-owner validation'},indent=2)+'\n')
