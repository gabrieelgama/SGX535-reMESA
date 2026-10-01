"""Construct only the pinned offline first-load image; never insert modules."""
import argparse
import gzip
import hashlib
import json
from pathlib import Path, PurePosixPath
import re
import stat
import struct
import zlib
import frozen_module_versions as versions

CAPTURE=Path('docs/hardware-evidence/MINI12-20261001T030754Z-BOOT-PROVENANCE-READONLY-02')
DERIVATIVE=Path('docs/phase8/artifacts/candidate-01-build-20260930/candidate-02-lifecycle/gma500_gfx.ko')
TABLE=Path('docs/hardware-evidence/MINI12-20260930-POSTRESET-ABI-READONLY-08/artifacts/build_Module_symvers')
RELEASE='5.10.240-antix.1-486-smp'
DERIVATIVE_HASH='91a6040e743d9c6222cb1307067db6e29fe92558576716a33d9fe0f4f5a87d74'
STOCK_HASH='f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341'
TABLE_HASH='faab2fae02fec696f2901790feba0e81776a0b66e74e14bbce0c72c570039dca'
GRUB_HASH='396ac7dff0bdea48393fd0a25a5fc633d40e2368caf14872b855eba03e96089f'
EXPERIMENTAL_INITRD='/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-01'
PAYLOAD='usr/lib/sgx535-first-load/gma500_gfx.ko'
HOOK='scripts/sgx535-first-load'
ROOT_MODULES=f'usr/lib/modules/{RELEASE}/'


def sha(data):return hashlib.sha256(data).hexdigest()

def path_ok(name):
 if not name or '\0' in name or name.startswith('/') or '..' in PurePosixPath(name).parts:
  raise ValueError('unsafe archive path')

def record(name,data,fields=None):
 path_ok(name);n=name.encode()+b'\0'
 f=list(fields) if fields else [1,stat.S_IFREG|0o644,0,0,1,0,0,0,0,0,0,0,0]
 f[6]=len(data);f[11]=len(n);f[12]=0
 if len(f)!=13 or any(x<0 or x>0xffffffff for x in f):raise ValueError('bad newc fields')
 h=b'070701'+b''.join(f'{x:08x}'.encode() for x in f)+n
 return h+b'\0'*((-len(h))%4)+data+b'\0'*((-len(data))%4)

def scan(data,start=0):
 pos=start;rows=[];seen=set()
 while True:
  begin=pos
  if pos+110>len(data) or data[pos:pos+6]!=b'070701':raise ValueError('invalid/truncated newc')
  try:f=[int(data[pos+6+i*8:pos+14+i*8],16) for i in range(13)]
  except ValueError as exc:raise ValueError('bad newc field') from exc
  ns=pos+110;namesize=f[11]
  if namesize<1 or ns+namesize>len(data) or data[ns+namesize-1]!=0:raise ValueError('bad newc name')
  name=data[ns:ns+namesize-1].decode();path_ok(name)
  ds=(ns+namesize+3)&~3;end=ds+f[6];pos=(end+3)&~3
  if end>len(data) or pos>len(data):raise ValueError('truncated newc payload')
  if name in seen:raise ValueError('duplicate newc path')
  seen.add(name)
  if name=='TRAILER!!!':
   if f[6]!=0:raise ValueError('bad trailer')
   return rows,pos
  rows.append({'name':name,'fields':f,'data':data[ds:end],'raw':data[begin:pos]})

def splice(main,new_init,additions):
 rows,end=scan(main);lookup={x['name']:x for x in rows}
 if 'init' not in lookup or lookup['init']['fields'][4]!=1:raise ValueError('missing/hardlinked init')
 if any(main[end:]):raise ValueError('nonzero archive tail')
 names=[x[0] for x in additions]
 if len(set(names))!=len(names) or set(names)&lookup.keys():raise ValueError('duplicate additions')
 out=b''.join(record('init',new_init,x['fields']) if x['name']=='init' else x['raw'] for x in rows)
 inode=max(x['fields'][0] for x in rows)+1
 for name,payload,mode in additions:
  path_ok(name)
  f=[inode,mode,0,0,2 if stat.S_ISDIR(mode) else 1,0,0,*lookup['init']['fields'][7:11],0,0]
  out+=record(name,payload,f);inode+=1
 out+=record('TRAILER!!!',b'')
 return out+b'\0'*((-len(out))%512)

def order_dependencies(roots,graph):
 result=[];active=set();done=set()
 def visit(n):
  if n in done:return
  if n in active or n not in graph:raise ValueError('cyclic/missing dependency '+n)
  active.add(n)
  for dep in graph[n]:visit(dep)
  active.remove(n);done.add(n);result.append(n)
 for n in roots:visit(n)
 return result

def elf_info(data):
 if data[:7]!=b'\x7fELF\x01\x01\x01':raise ValueError('not ELF32 little endian')
 h=struct.unpack_from('<16sHHIIIIIHHHHHH',data)
 if h[2]!=3 or h[11]!=40:raise ValueError('not i386 ELF sections')
 sections=[struct.unpack_from('<IIIIIIIIII',data,h[6]+i*40) for i in range(h[12])]
 def payload(s):
  if s[4]+s[5]>len(data):raise ValueError('ELF section bounds')
  return data[s[4]:s[4]+s[5]]
 names=payload(sections[h[13]]);byname={}
 for s in sections:byname[names[s[0]:].split(b'\0',1)[0].decode()]=payload(s) if s[1]!=8 else b''
 info={}
 for value in byname.get('.modinfo',b'').split(b'\0'):
  if b'=' in value:
   k,v=value.decode().split('=',1);info.setdefault(k,[]).append(v)
 undefined=set()
 for s in sections:
  if s[1]!=2:continue
  symbols=payload(s);strings=payload(sections[s[6]])
  if s[9]!=16:raise ValueError('ELF symbol entries')
  for pos in range(0,len(symbols),16):
   n,_,_,kind,_,index=struct.unpack_from('<IIIBBH',symbols,pos)
   if n and index==0 and kind>>4:
    undefined.add(strings[n:].split(b'\0',1)[0].decode())
 return {'sections':byname,'modinfo':info,'undefined':undefined}

def load_inputs(repo):
 c=repo/CAPTURE;stock=(c/'decoded-files/release_initrd').read_bytes()
 if sha(stock)!=STOCK_HASH:raise ValueError('stock image identity')
 first,end=scan(stock)
 offset=end
 while offset<len(stock) and stock[offset]==0:offset+=1
 if stock[offset:offset+2]!=b'\x1f\x8b':raise ValueError('stock compression')
 stream=zlib.decompressobj(31);main=stream.decompress(stock[offset:])+stream.flush()
 if not stream.eof or stream.unused_data:raise ValueError('stock gzip tail')
 rows,main_end=scan(main);lookup={x['name']:x for x in rows}
 if any('gma500' in x['name'] for x in first+rows):raise ValueError('unexpected original in stock image')
 if len(first)!=5 or len(rows)!=2133:raise ValueError('stock member count')
 derivative=(repo/DERIVATIVE).read_bytes()
 if len(derivative)!=242724 or sha(derivative)!=DERIVATIVE_HASH:raise ValueError('derivative identity')
 table=versions.read_symvers(repo/TABLE)
 if table[1]!=TABLE_HASH:raise ValueError('target table identity')
 info=elf_info(derivative)
 if info['modinfo'].get('name')!=['gma500_gfx'] or info['modinfo'].get('vermagic')!=[RELEASE+' SMP mod_unload modversions 486 ']:raise ValueError('derivative metadata')
 if info['modinfo'].get('depends')!=['drm,drm_kms_helper,video,i2c-algo-bit']:raise ValueError('dependency metadata drift')
 if info['modinfo'].get('firmware') or info['modinfo'].get('softdep'):raise ValueError('new firmware/softdep requirement')
 aliases=info['modinfo']['alias']
 matching_ids=[]
 for alias in aliases:
  match=re.fullmatch(r'pci:v00008086d0000([0-9A-F]{4})sv\*sd\*bc\*sc\*i\*',alias)
  if not match:raise ValueError('unexpected driver matching scope')
  matching_ids.append('0x8086:0x'+match[1].lower())
 if 'pci:v00008086d00008108sv*sd*bc*sc*i*' not in aliases:raise ValueError('Poulsbo alias absent')
 graph={}
 for line in lookup[ROOT_MODULES+'modules.dep']['data'].decode().splitlines():
  name,deps=line.split(':',1);graph[name]=deps.split()
 # Actual modinfo names select the captured stock paths; all indirect edges
 # are read from the image's unmodified modules.dep.
 roots=[]
 for dep in info['modinfo']['depends'][0].split(','):
  candidates=[n for n in graph if Path(n).stem.replace('-','_')==dep.replace('-','_')]
  if len(candidates)!=1:raise ValueError('ambiguous dependency '+dep)
  roots.append(candidates[0])
 order=order_dependencies(roots,graph)
 expected={'video','drm_kms_helper','cec','drm','fb_sys_fops','syscopyarea','sysfillrect','sysimgblt','i2c-algo-bit'}
 if {Path(x).stem for x in order}!=expected or len(order)!=9:raise ValueError('dependency closure drift')
 prior=json.loads((c/'offline-analysis/dependency-coverage.json').read_text())
 pinned={x['initramfs_member']:x['sha256'] for x in prior['modules']}
 payloads=[]
 for relative in order:
  row=lookup[ROOT_MODULES+relative]
  if sha(row['data'])!=pinned.get(row['name']):raise ValueError('dependency bytes drift')
  mod=elf_info(row['data'])
  if mod['modinfo'].get('firmware') or mod['modinfo'].get('softdep'):raise ValueError('dependency firmware/softdep')
  # Metadata must introduce no edge outside the selected fixed closure.
  for name in mod['modinfo'].get('depends',[''])[0].split(','):
   if name and name.replace('-','_') not in {n.replace('-','_') for n in expected}:raise ValueError('dependency outside closure')
  payloads.append({'path':'/'+row['name'],'sha256':sha(row['data']),'name':Path(relative).stem.replace('-','_'),'source_member':row['name'],'data':row['data']})
 payloads.append({'path':'/'+PAYLOAD,'sha256':DERIVATIVE_HASH,'name':'gma500_gfx','data':derivative})
 qualification=[]
 for item in payloads:
  path=(repo/DERIVATIVE if item['name']=='gma500_gfx' else c/'offline-analysis/dependency-modules'/item['source_member'][len(ROOT_MODULES):])
  imported=versions.read_versions(path)
  if imported[1]!=item['sha256']:raise ValueError('checker source identity')
  q=versions.compare_symvers(path,repo/TABLE)
  actual=elf_info(item['data'])
  if actual['undefined']!=imported[0].keys()-{'module_layout'}:raise ValueError('undefined/version-set mismatch '+item['name'])
  if q['classification']!='PASS' or q['module_layout']['reference']!='0xb84efb99':raise ValueError('ABI reject '+item['name'])
  if actual['modinfo'].get('vermagic')!=[RELEASE+' SMP mod_unload modversions 486 ']:raise ValueError('dependency vermagic')
  qualification.append({'name':item['name'],'sha256':item['sha256'],'versioned_imports':len(imported[0]),'undefined_symbols':len(actual['undefined']),'missing':0,'crc_mismatches':0,'module_layout':'0xb84efb99','modinfo':actual['modinfo'],'imports':{name:{'crc':f'0x{crc:08x}','target_crc':f'0x{table[0][name]:08x}','undefined':name in actual['undefined']} for name,crc in sorted(imported[0].items())}})
 # Resolve applet hardlinks, including the nonzero archived BusyBox payload.
 utilities={}
 for name in ['usr/bin/sh','usr/bin/sha256sum','usr/bin/uname','usr/bin/readlink','usr/bin/grep','usr/bin/cat','usr/bin/sleep','usr/sbin/insmod']:
  x=lookup[name]
  peers=[p for p in rows if p['fields'][0]==x['fields'][0] and p['fields'][7:9]==x['fields'][7:9] and p['data']]
  if len(peers)!=1 or peers[0]['data'][:7]!=b'\x7fELF\x01\x01\x01':raise ValueError('applet hardlink identity')
  utility_elf=elf_info(peers[0]['data'])
  utilities[name]={'payload_member':peers[0]['name'],'sha256':sha(peers[0]['data']),'inode':x['fields'][0],'elf':'ELF32 little-endian i386'}
 if sha((c/'decoded-files/_boot_grub_grub_cfg').read_bytes())!=GRUB_HASH:raise ValueError('GRUB source drift')
 return {'stock':stock,'prefix':stock[:offset],'main':main,'rows':rows,'early_rows':first,'payloads':payloads,'qualification':qualification,'utilities':utilities,'matching_ids':sorted(set(matching_ids)),'note_hash':sha(info['sections']['.note.gnu.build-id']),'build_id':info['sections']['.note.gnu.build-id'][16:].hex(),'repo':repo}

def render_hook(inputs):
 template=(inputs['repo']/'kernel/sgx535_frozen/first_load/preload.sh.in').read_text()
 names=' '.join(p['name'] for p in inputs['payloads'])
 checks='\n'.join(f"    sgx_hash_ok {p['path']} {p['sha256']} || {{ sgx_refuse payload-{p['name']}; return 1; }}" for p in inputs['payloads'])
 insert='\n'.join(f"    sgx_insert_file {p['path']} || {{ sgx_refuse insertion-{p['name']}; return 1; }}" for p in inputs['payloads'])
 out=template.replace('@MODULE_NAMES@',names).replace('@PAYLOAD_CHECKS@',checks).replace('@INSERTIONS@',insert).replace('@NOTE_HASH@',inputs['note_hash']).replace('@MATCHING_IDS@','|'.join(inputs['matching_ids']))
 if '@' in out:raise ValueError('unrendered hook')
 return out

def patched_init(old,hook):
 marker=b'\nrun_scripts /scripts/init-top\n'
 if old.count(marker)!=1:raise ValueError('init ordering anchor drift')
 digest=sha(hook)
 block=f'''\n# SGX535-FIRSTLOAD-BEGIN: precedes init-top/udev; failure never falls through.
firstload_hash=$(/bin/sha256sum /scripts/sgx535-first-load)
firstload_status=$?
if [ "$firstload_status" != 0 ] || [ "${{firstload_hash%% *}}" != {digest} ] || ! /bin/sh /scripts/sgx535-first-load; then
    echo 'SGX535 FIRST-LOAD HOLD: no continuation or second load' >/dev/console
    while :; do /bin/sleep 3600; done
fi
# SGX535-FIRSTLOAD-END
'''.encode()
 return old.replace(marker,block+marker)

def grub_entry(cfg):
 start=cfg.index("menuentry 'antiX-26 Stephen Kapos, 5.10.240-antix.1-486-smp'")
 end=cfg.index('\nmenuentry ',start+1)
 block=cfg[start:end]
 if block.count('\tsavedefault\n')!=1:raise ValueError('stock entry saving anchor')
 lines=block.splitlines();lines[0]="menuentry 'EXPERIMENTAL SGX535 rev121 FIRST-LOAD ONLY (no triangle)' --class gnu-linux --class os --id 'sgx535-rev121-firstload-01' {"
 block='\n'.join(l for l in lines if l!='\tsavedefault')+'\n'
 block=block.replace('/boot/initrd.img-'+RELEASE,EXPERIMENTAL_INITRD)
 if any(t in block for t in ['savedefault','save_env','set default','next_entry','blacklist','nomodeset']):raise ValueError('persistent/changed entry policy')
 if block.count(EXPERIMENTAL_INITRD)!=1:raise ValueError('initrd entry path')
 return '# OFFLINE PROPOSAL ONLY. Manual selection; leave the saved stock entry unchanged.\n'+block

def build(repo,out):
 if out.exists():raise ValueError('do not overwrite previous artifacts')
 inputs=load_inputs(repo);hook=render_hook(inputs).encode();old_init=next(x['data'] for x in inputs['rows'] if x['name']=='init')
 new_init=patched_init(old_init,hook)
 additions=[('usr/lib/sgx535-first-load',b'',stat.S_IFDIR|0o755),(PAYLOAD,inputs['payloads'][-1]['data'],stat.S_IFREG|0o644),(HOOK,hook,stat.S_IFREG|0o755)]
 main=splice(inputs['main'],new_init,additions)
 output=inputs['prefix']+gzip.compress(main,compresslevel=9,mtime=0)
 out.mkdir(parents=True)
 (out/'initrd.img-sgx535-firstload-01').write_bytes(output)
 (out/'sgx535-first-load.sh').write_bytes(hook);(out/'proposed-custom.cfg').write_text(grub_entry((repo/CAPTURE/'decoded-files/_boot_grub_grub_cfg').read_text()))
 (out/'modified-init').write_bytes(new_init)
 (out/'payloads.json').write_text(json.dumps([{k:v for k,v in p.items() if k!='data'} for p in inputs['payloads']],indent=2)+'\n')
 (out/'module-qualification.json').write_text(json.dumps(inputs['qualification'],indent=2)+'\n')
 (out/'utilities.json').write_text(json.dumps(inputs['utilities'],indent=2)+'\n')
 (out/'identity.json').write_text(json.dumps({'stock_sha256':STOCK_HASH,'image_bytes':len(output),'image_sha256':sha(output),'early_prefix_bytes':len(inputs['prefix']),'early_prefix_sha256':sha(inputs['prefix']),'derivative_sha256':DERIVATIVE_HASH,'hook_sha256':sha(hook),'loaded_note_sha256':inputs['note_hash'],'build_id':inputs['build_id'],'target_table_sha256':TABLE_HASH,'new_members':len(inputs['early_rows'])+len(inputs['rows'])+len(additions),'compression':'gzip level9 mtime0; unchanged raw early archive/padding; stock main records preserved except init','gate_b':'BLOCKED','whitelist':[]},indent=2)+'\n')
 return json.loads((out/'identity.json').read_text())

def main():
 p=argparse.ArgumentParser();p.add_argument('--repo',type=Path,default=Path(__file__).resolve().parents[2]);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 print(json.dumps(build(a.repo,a.output),indent=2))
if __name__=='__main__':main()
