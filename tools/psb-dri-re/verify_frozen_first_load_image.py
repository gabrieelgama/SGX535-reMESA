"""Independent offline inspection of the constructed image, using GNU cpio too."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import stat
import subprocess
import zlib
import frozen_first_load_image as construction


def validate_layout(before,after):
 # Only init contents may change; its type, executable mode and inode identity
 # must survive. New records have deterministic, disjoint inode identities.
 row=after['init']
 if row['raw']!=construction.record('init',row['data'],before['init']['fields']):
  raise ValueError('retained init metadata changed')
 inode=max(x['fields'][0] for x in before.values())+1
 for name,mode in [('usr/lib/sgx535-first-load',0o40755),(construction.PAYLOAD,0o100644),(construction.HOOK,0o100755)]:
  row=after[name]
  if stat.S_ISDIR(mode) and row['data']:raise ValueError('new directory contains data')
  fields=[inode,mode,0,0,2 if stat.S_ISDIR(mode) else 1,0,0,*before['init']['fields'][7:11],0,0]
  if row['raw']!=construction.record(name,row['data'],fields):
   raise ValueError('new record inode/type/mode/lifetime metadata changed: '+name)
  inode+=1


def verify(repo,image,proof):
 if proof.exists():raise ValueError('preserve previous verification')
 inputs=construction.load_inputs(repo)
 data=image.read_bytes();prefix=inputs['prefix']
 if data[:len(prefix)]!=prefix:raise ValueError('early archive/microcode changed')
 main=zlib.decompress(data[len(prefix):],31)
 old,old_end=construction.scan(inputs['main']);new,new_end=construction.scan(main)
 before={x['name']:x for x in old};after={x['name']:x for x in new}
 added=set(after)-set(before);removed=set(before)-set(after)
 modified={n for n in before.keys()&after.keys() if before[n]['raw']!=after[n]['raw']}
 if added!={'usr/lib/sgx535-first-load',construction.PAYLOAD,construction.HOOK} or removed or modified!={'init'}:
  raise ValueError('unexpected image delta')
 validate_layout(before,after)
 if any(main[new_end:]):raise ValueError('nonzero main tail')
 hook=construction.render_hook(inputs).encode()
 if after[construction.HOOK]['data']!=hook:raise ValueError('actual hook differs')
 if after['init']['data']!=construction.patched_init(before['init']['data'],hook):raise ValueError('init gate differs')
 if after[construction.PAYLOAD]['data']!=inputs['payloads'][-1]['data']:raise ValueError('derivative content drift')
 if sum('gma500_gfx' in n for n in after)!=1:raise ValueError('unexpected second/original module')
 if any(n.endswith('frozen-triangle-one-shot-i386') for n in after):raise ValueError('client in initrd')
 for p in inputs['payloads']:
  if construction.sha(after[p['path'][1:]]['data'])!=p['sha256']:raise ValueError('payload mismatch')
 for n in ['usr/lib','scripts']:
  if not stat.S_ISDIR(after[n]['fields'][1]):raise ValueError('unexpected parent path')
 for row in after.values():
  if row['name'] in added and row['fields'][2:4]!=[0,0]:raise ValueError('new owner metadata')
 init=after['init']['data'].decode()
 if init.count('/bin/sh /scripts/sgx535-first-load')!=1 or init.index('/bin/sh /scripts/sgx535-first-load')>init.index('run_scripts /scripts/init-top'):
  raise ValueError('pre-udev ordering')
 for forbidden in ['rmmod','unbind','/dev/dri','ioctl','reboot']:
  if forbidden in hook.decode():raise ValueError('prohibited hook action')
 # Preserved independently tested parser; do not modify or execute target text.
 reader_path=repo/construction.CAPTURE/'offline-analysis/read_initramfs.py'
 spec=importlib.util.spec_from_file_location('preserved_image_reader',reader_path)
 reader=importlib.util.module_from_spec(spec);spec.loader.exec_module(reader)
 independent,containers=reader.read_image(data)
 if len(independent)!=2141:raise ValueError('independent member count')
 proof.mkdir(parents=True)
 commands=[]
 def cpio(label,payload,args):
  argv=['/usr/bin/cpio',*args];cp=subprocess.run(argv,input=payload,cwd=proof,capture_output=True)
  (proof/(label+'.stdout')).write_bytes(cp.stdout);(proof/(label+'.stderr')).write_bytes(cp.stderr)
  commands.append({'label':label,'argv':argv,'exit_code':cp.returncode})
  if cp.returncode or cp.stderr:raise ValueError('GNU cpio inspection failed '+label)
  return cp.stdout
 native=cpio('native-main-list',main,['-it','--quiet']).decode().splitlines()
 if native!=[x['name'] for x in new]:raise ValueError('independent cpio catalog disagrees')
 early=cpio('native-early-list',prefix,['-it','--quiet']).decode().splitlines()
 if early!=[x['name'] for x in inputs['early_rows']]:raise ValueError('early catalog disagrees')
 for label,name in [('native-derivative',construction.PAYLOAD),('native-hook',construction.HOOK),('native-init','init')]:
  extracted=cpio(label,main,['-i','--to-stdout','--quiet',name])
  if extracted!=after[name]['data']:raise ValueError('native extraction disagrees '+name)
 (proof/'inspection-commands.json').write_text(json.dumps(commands,indent=2)+'\n')
 (proof/'full-image-manifest.json').write_text(json.dumps([{k:v for k,v in x.items() if k!='data'} for x in independent],indent=2)+'\n')
 (proof/'stock-versus-experimental-delta.json').write_text(json.dumps({'stock_members':2138,'experimental_members':2141,'added':sorted(added),'modified':sorted(modified),'removed':[],'unchanged_main_raw_records':len(old)-1,'early_prefix_byte_exact':True,'all_unchanged_records_preserve_headers_payloads_padding_and_hardlink_ids':True,'dependency_indices':'stock indices untouched; private payload loaded by explicit fixed-path insmod','full_stock_framework_preserved_except_guarded_init_insertion':True},indent=2)+'\n')
 identity={'image_path':str(image),'image_bytes':len(data),'image_sha256':construction.sha(data),'derivative_in_image_sha256':construction.sha(after[construction.PAYLOAD]['data']),'hook_in_image_sha256':construction.sha(hook),'early_prefix_sha256':construction.sha(prefix),'native_cpio_sha256':construction.sha(Path('/usr/bin/cpio').read_bytes()),'native_cpio_version':subprocess.check_output(['/usr/bin/cpio','--version'],text=True).splitlines()[0],'independent_parser_path':str(reader_path),'independent_parser_sha256':construction.sha(reader_path.read_bytes()),'result':'PASS OFFLINE','live_first_owner':'UNKNOWN','live_display_ssh_recovery':'UNKNOWN','gate_b':'BLOCKED','whitelist':[]}
 (proof/'qualification.json').write_text(json.dumps(identity,indent=2)+'\n')
 return identity

def main():
 p=argparse.ArgumentParser();p.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[2]);p.add_argument('--image',type=Path,required=True);p.add_argument('--proof',type=Path,required=True);a=p.parse_args()
 print(json.dumps(verify(a.repo,a.image,a.proof),indent=2))
if __name__=='__main__':main()
