"""CPU-only explicit binding/transport preparation. Import never contacts hardware."""
import ast,json,hashlib,uuid,shlex,datetime,time,subprocess,os,sys
from pathlib import Path
sys.dont_write_bytecode=True
ROOT=Path(__file__).parent
B=json.loads((ROOT/'authoritative-bindings.json').read_text())
ORIGIN=json.loads((ROOT/'origins.json').read_text())
from witness_binding import bind_predicate,validate as validate_witness
WITNESS=json.loads((ROOT/'witness-authority.json').read_text())
OLD=Path(ORIGIN['old']);PRIOR=Path(ORIGIN['prior2'])
OLD_DRIVER='9d0b5b2fdb7d9881f2828f43fb89253176c38817'
OLD_NOTE='f91c232ec42c521dd961c33ea1f985cb3fc921714768052199d5129671e0e13d'
OLD_BOOT='a76f3b25-31ae-4507-891b-474e912dd356'
OLD_SOURCE='2967348bcf2f2e85ff1bbd56848283df9a187839acf43a5c4a48c6fb4b76f555'
OLD_BASE='/root/sgx535-frozen-seq1-'+OLD_DRIVER+'-ef7e01cb'

def sha(d):return hashlib.sha256(d).hexdigest()
def edits(source, changes):
 lines=source.splitlines(keepends=True);off=[0]
 for line in lines:off.append(off[-1]+len(line))
 for node,replacement in sorted(changes,key=lambda x:(x[0].lineno,x[0].col_offset),reverse=True):
  a=off[node.lineno-1]+node.col_offset;b=off[node.end_lineno-1]+node.end_col_offset
  source=source[:a]+replacement+source[b:]
 compile(source,'bound-controller','exec');return source

def remove_argv_overrides(source):
 changes=[]
 for n in ast.walk(ast.parse(source)):
  if isinstance(n,ast.Assign) and any(isinstance(x,ast.Attribute) and isinstance(x.value,ast.Name) and x.value.id=='sys' and x.attr=='argv' for x in n.targets):changes.append((n,'pass # historical sys.argv override removed; explicit dispatched arguments are authoritative'))
 out=edits(source,changes)
 # Complete AST equality apart from these controller-input replacements.
 class Normalize(ast.NodeTransformer):
  def visit_Assign(self,n):
   if any(isinstance(x,ast.Attribute) and isinstance(x.value,ast.Name) and x.value.id=='sys' and x.attr=='argv' for x in n.targets):return ast.Pass()
   return self.generic_visit(n)
 assert ast.dump(Normalize().visit(ast.parse(source)),include_attributes=False)==ast.dump(ast.parse(out),include_attributes=False)
 return out,len(changes)

def assignments(source,values):
 found={};changes=[]
 for n in ast.parse(source).body:
  if isinstance(n,ast.Assign) and len(n.targets)==1 and isinstance(n.targets[0],ast.Name) and n.targets[0].id in values:
   key=n.targets[0].id
   if key in found:raise ValueError('duplicate binding '+key)
   found[key]=True;changes.append((n.value,repr(values[key])))
 if set(found)!=set(values):raise ValueError('missing binding '+repr(set(values)-set(found)))
 return edits(source,changes)

def replace_literal(source,before,after,expected=None):
 nodes=[n for n in ast.walk(ast.parse(source)) if isinstance(n,ast.Constant) and n.value==before]
 if expected is not None and len(nodes)!=expected:raise ValueError('literal binding count '+repr(before))
 if not nodes:raise ValueError('missing literal binding '+repr(before))
 return edits(source,[(n,repr(after)) for n in nodes])

def read_source(phase):
 paths={'incoming':PRIOR/'incoming.proposed.py','stage':PRIOR/'stage.proposed.py','first-owner':PRIOR/'first-owner.proposed.py','protected':OLD/'protected-preparation/source.py','final':OLD/'independent-final-guards/source.py','precheck':OLD/'fire-one-authorized-call/precheck-source.py','fire':OLD/'fire-one-authorized-call/authorized-ordinary-call-source.py','retrieve':OLD/'fire-one-authorized-call/retrieve-source.py'}
 return paths[phase].read_text()

def early(phase):
 source,_=remove_argv_overrides(read_source(phase))
 if phase=='stage':
  nodes=[n.args[0] for n in ast.walk(ast.parse(source)) if isinstance(n,ast.Call) and isinstance(n.func,ast.Name) and n.func.id=='exec' and n.args and isinstance(n.args[0],ast.Constant) and isinstance(n.args[0].value,str)]
  if len(nodes)!=1:raise ValueError('one embedded STOCK guard expected')
  embedded=(PRIOR/'stock-preflight.corrected.py').read_text()
  p=B['plan'];a=repr([p[k] for k in ['image','incoming','backup','pending']]);b=repr([p[k] for k in ['image','backup','pending']])
  if embedded.count(a)!=1:raise ValueError('incoming phase binding count')
  embedded=embedded.replace(a,b,1)
  source=edits(source,[(nodes[0],repr(embedded))])
 return source

def dispatch_args(phase,args):
 expected={'incoming':['--prepare','--stock-boot',B['stock_boot']],'stage':['--apply','--stock-boot',B['stock_boot']],'first-owner':['--prior-boot',B['stock_boot']]}.get(phase,[])
 if list(args)!=expected:raise ValueError('noncanonical/duplicate/stale/positional arguments refused')
 return list(args)

def candidate_context(boot,first,source_sha):
 if str(uuid.UUID(boot))!=boot or boot in [B['stock_boot'],OLD_BOOT]:raise ValueError('fresh candidate boot required')
 if len(source_sha)!=64 or any(c not in '0123456789abcdef' for c in source_sha):raise ValueError('fresh source hash required')
 return {'boot_id':boot,'module_build_id':B['candidate']['driver']['build_id'],'observer_build_id':B['candidate']['observer']['build_id'],'image_sha256':B['candidate']['image']['sha256'],'action':B['action'],'first_owner_guards':first,'capsule':'UNUSED','source_boundary':source_sha,'sgx_execution_authorized':False,'sgx_invocations_this_boot':0,'historical_client_invocations':2,'triangle_this_boot':'NOT ATTEMPTED','hypothesis':B['hypothesis'],'experimental_words':B['experimental_words'],'diagnostic_argb':B['diagnostic_argb']}

def late(phase,*,context,expected=None,card=None,pin=None):
 required={'boot_id','module_build_id','observer_build_id','image_sha256','source_boundary'}
 if not required<=set(context):raise ValueError('missing fresh live binding')
 c=candidate_context(context['boot_id'],context['first_owner_guards'],context['source_boundary'])
 if context!=c:raise ValueError('context does not match authoritative binding table')
 boot=context['boot_id'];base=B['protected_base'];evidence=base+'/evidence-'+boot
 source=read_source(phase)
 values={'BOOT':boot,'BASE':base,'EVIDENCE':evidence,'CONTEXT':context}
 if phase=='retrieve':
  if pin is None or pin['path']!=evidence:raise ValueError('current protected destination receipt required')
  source=assignments(source,{'EVIDENCE':evidence,'PIN':pin})
 else:
  if phase in ['final','precheck','fire']:
   if expected is None or expected['evidence']['path']!=evidence or expected['base']['path']!=base or expected['boot_id']!=boot:raise ValueError('current preparation receipts required')
   values['EXPECTED']=expected;values['STAGED']={'merged_config':B['merged_config']}
  if phase=='fire':
   if card is None or card['boot_id']!=boot or card['driver']['build_id']!=context['module_build_id'] or card['image']['sha256']!=context['image_sha256'] or card['observer']['build_id']!=context['observer_build_id']:raise ValueError('current reviewed execution card required')
   values['CARD']=card;values['CARD_SHA']=sha((json.dumps(card,indent=2)+'\n').encode())
  source=assignments(source,values)
  source=replace_literal(source,OLD_NOTE,B['candidate']['driver']['loaded_note_sha256'],expected=1)
  source=bind_predicate(source,context['source_boundary'],edits)
  validate_witness(source,WITNESS,phase)
 # No historical action/boot/inode receipt may remain active in late phases.
 for n in ast.walk(ast.parse(source)):
  if isinstance(n,ast.Constant) and isinstance(n.value,str) and any(x in n.value for x in [OLD_BOOT,OLD_BASE,OLD_NOTE]):raise ValueError('stale active late-phase binding')
 compile(source,'late-phase','exec');return source

def prefix():
 t=B['transport']
 return ['ssh','-T','-p',str(t['port']),'-o','BatchMode=yes','-o','ConnectTimeout=8','-o','ConnectionAttempts=1','-o','StrictHostKeyChecking=yes','-o','UserKnownHostsFile='+t['known_hosts'],'-o','IdentitiesOnly=yes','-o','PasswordAuthentication=no','-o','KbdInteractiveAuthentication=no','-i',t['identity'],t['user']+'@'+t['host']]

def once(name,source,*,sudo=False,args=(),payload=None,timeout=40,phase=None):
 if not (ROOT/'controller-audit-PASS.json').is_file():raise ValueError('offline audit gate absent')
 if phase is not None:dispatch_args(phase,args)
 if '__UNBOUND_' in source:raise ValueError('unresolved future binding')
 d=ROOT/name;d.mkdir(exist_ok=False);(d/'source.py').write_text(source)
 remote=shlex.join((['sudo','-n','-p',''] if sudo else [])+['python3','-I','-B','-S','-c',source]+list(args));argv=prefix()+[remote]
 rec={'argv':argv,'source_sha256':sha(source.encode()),'start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'automatic_retry':False,'stdin':'PAYLOAD ONLY' if payload is not None else 'DEVNULL'}
 (d/'command.json').write_text(json.dumps(rec,indent=2)+'\n');start=time.monotonic()
 try:
  cp=subprocess.run(argv,input=payload if payload is not None else b'',capture_output=True,timeout=timeout);out,err,status=cp.stdout,cp.stderr,cp.returncode
 except subprocess.TimeoutExpired as e:out,err,status=e.stdout or b'',e.stderr or b'',124
 (d/'stdout.original.txt').write_bytes(out);(d/'stderr.original.txt').write_bytes(err)
 rec.update(exit_code=status,host_duration=time.monotonic()-start,end_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),stdout_sha256=sha(out),stderr_sha256=sha(err))
 (d/'receipt.json').write_text(json.dumps(rec,indent=2)+'\n')
 objects=[];rem=out.decode();dec=json.JSONDecoder()
 try:
  while rem.strip():rem=rem.lstrip();v,n=dec.raw_decode(rem);objects.append(v);rem=rem[n:]
 except Exception as e:(d/'decode-error.json').write_text(json.dumps({'error':repr(e)}));raise
 (d/'decoded.json').write_text(json.dumps(objects,indent=2)+'\n')
 if status or err:raise RuntimeError('STOP; receipt retained: '+str(d))
 return objects,rec

def health_source():
 s=(Path('/home/gama/sgx535-gfx/tools/psb-dri-re/frozen_kernel_health.py')).read_text()
 # Same classifier source is embedded in the retained qualified first-owner profile.
 if sha(s.encode())!='5abe09ed15609cc2ad409d909d4785953275656690632700c869fc5a48d3b7ee':raise ValueError('health classifier changed')
 return s

def freshness_source():
 const={'boot':B['stock_boot'],'kernel':B['kernel'],'arch':B['architecture'],'machine':B['machine'],'note':B['stock_note_sha256'],'cfg':B['stock_custom_cfg'],'plan':B['plan']}
 return 'import os,stat,json,hashlib,subprocess,datetime\nfrom pathlib import Path\nK='+repr(const)+'\n'+health_source()+'''\nr={'scope':'minimum passive SAME STOCK freshness; no mutation','guards':[],'start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat()}
def need(v,n):
 r['guards'].append({'name':n,'pass':bool(v)})
 if not v:raise ValueError(n)
try:
 need(os.geteuid()==0,'root passive privilege')
 boot=Path('/proc/sys/kernel/random/boot_id').read_text().strip();r['boot_id']=boot
 need(boot==K['boot'],'same established STOCK boot')
 need(os.uname().release==K['kernel'] and os.uname().machine==K['arch'],'same kernel/architecture')
 need(Path('/sys/class/dmi/id/product_name').read_text().strip()==K['machine'],'same Mini12')
 need(Path('/sys/module/gma500_gfx/initstate').read_text().strip()=='live','stock module Live')
 need(hashlib.sha256(Path('/sys/module/gma500_gfx/notes/.note.gnu.build-id').read_bytes()).hexdigest()==K['note'],'same original STOCK loaded bytes')
 need(os.path.realpath('/sys/bus/pci/devices/0000:00:02.0/driver')=='/sys/bus/pci/drivers/gma500','same PCI owner')
 for path in ['/proc/sgx535_source_guard','/proc/sgx535_current_operation','/sys/module/sgx535_provenance','/run/initramfs/sgx535-first-load.log']:need(not os.path.lexists(path),'stock derivative absence '+path)
 path='/boot/grub/custom.cfg';st=os.lstat(path);x=K['cfg'];data=Path(path).read_bytes()
 need(stat.S_ISREG(st.st_mode) and st.st_uid==x['uid'] and st.st_gid==x['gid'] and oct(stat.S_IMODE(st.st_mode))==x['mode'] and st.st_nlink==x['nlink'] and st.st_dev==x['dev'] and st.st_ino==x['ino'] and len(data)==x['size'] and hashlib.sha256(data).hexdigest()==x['sha256'],'same authoritative config inode/bytes')
 p=subprocess.run(['grub-editenv','/boot/grub/grubenv','list'],capture_output=True,timeout=10)
 need(p.returncode==0 and not p.stderr and p.stdout.strip()==b'saved_entry=gnulinux-5.10.240-antix.1-486-smp-advanced-6da9b4a7-ede2-4e27-bbfc-b537f568eaf1','STOCK remains saved/default recovery')
 p=subprocess.run(['dmesg'],capture_output=True,timeout=10)
 need(p.returncode==0 and not p.stderr,'complete current kernel log')
 r['kernel_log']=p.stdout.decode(errors='replace');r['health']=classify_kernel_log(r['kernel_log'])
 need(r['health']['classification']!='REJECT','current STOCK health')
 for path in K['plan'].values():
  if path.startswith('/boot/') or path.startswith('/home/gama/'):need(not os.path.lexists(path),'new prospective destination remains absent '+path)
 need(Path('/proc/sys/kernel/random/boot_id').read_text().strip()==boot,'same STOCK boot at end')
 r['classification']='PASS SAME HEALTHY STOCK FRESHNESS'
except Exception as e:r['classification']='STOP PASSIVE FRESHNESS';r['error']=repr(e)
print(json.dumps(r,indent=2),flush=True)
if r['classification'].startswith('STOP'):raise SystemExit(1)
'''

def transfer_source(name,size,digest,created):
 if name not in ['initrd.img-sgx535-firstload-diagnostic-01','diagnostic-entry.proposed']:raise ValueError('fixed incoming basename required')
 # Exact qualified copy primitive, no altered copy semantics.
 copy=Path('/home/gama/sgx535-gfx/tools/psb-dri-re/frozen_first_load_stage.py').read_text()
 const={'boot':B['stock_boot'],'note':B['stock_note_sha256'],'dest':B['plan']['incoming'],'name':name,'bytes':size,'sha256':digest,'dir':created}
 return 'import sys,json\nK='+repr(const)+'\n'+copy+'''\nr={'scope':'one exclusive exact-payload incoming transfer; no SGX','operations':[]}
def need(v,n):
 if not v:raise ValueError(n)
def event(x):r['operations'].append(x);print(json.dumps({'transfer_event':x}),flush=True)
try:
 need(os.geteuid()==1000,'normal approved incoming owner')
 need(open('/proc/sys/kernel/random/boot_id').read().strip()==K['boot'],'same verified STOCK boot')
 need(hashlib.sha256(open('/sys/module/gma500_gfx/notes/.note.gnu.build-id','rb').read()).hexdigest()==K['note'],'same STOCK loaded driver')
 fd=os.open(K['dest'],os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW)
 s=os.fstat(fd);pin=K['dir']
 need(stat.S_ISDIR(s.st_mode) and (s.st_dev,s.st_ino)==(pin['dev'],pin['ino']) and s.st_uid==1000 and s.st_gid==1000 and stat.S_IMODE(s.st_mode)==0o700,'same exclusive incoming directory')
 r['file']=copy_exclusive(fd,K['name'],sys.stdin.buffer,K['bytes'],K['sha256'],1000,1000,0o400,event)
 need(sys.stdin.buffer.read(1)==b'','exact payload length')
 need(open('/proc/sys/kernel/random/boot_id').read().strip()==K['boot'],'same STOCK boot after copy')
 os.close(fd);r['classification']='PASS EXACT EXCLUSIVE TRANSFER'
except Exception as e:r['classification']='STOP TRANSFER; PARTIAL RETAINED';r['error']=repr(e)
print(json.dumps(r,indent=2),flush=True)
if r['classification'].startswith('STOP'):raise SystemExit(1)
'''

def poststage_source(stage,created):
 if stage.get('classification')!='FROZEN CANDIDATE STAGING PASS; NO BOOT/SGX ACTION':raise ValueError('actual PASS staging receipt required')
 operations=stage['operations'];pub=stage['publication'];newcfg=next(x for x in operations if x['path']==pub['source'])
 if pub['destination']!='/boot/grub/custom.cfg' or pub['phase']!='directory-synced':raise ValueError('final publication receipt required')
 image=next(x for x in operations if x['path']==B['plan']['image']);backup=next(x for x in operations if x['path']==B['plan']['backup'])
 if image['sha256']!=B['candidate']['image']['sha256'] or newcfg['sha256']!=B['merged_config']['sha256']:raise ValueError('staged identities drift')
 s=(PRIOR/'stock-preflight.corrected.py').read_text();changes=[]
 # Rebind active CURRENT cfg expectations. Earlier config expectations read preserved backups and remain untouched.
 for n in ast.walk(ast.parse(s)):
  if isinstance(n,ast.Dict):
   try:v=ast.literal_eval(n)
   except:continue
   if isinstance(v,dict) and v.get('path')=='/boot/grub/custom.cfg' and v.get('sha256')==B['stock_custom_cfg']['sha256']:
    w=dict(v)
    for k in ['dev','ino','size','sha256','uid','gid','mode','nlink']:w[k]=newcfg[k]
    changes.append((n,repr(w)))
 s=edits(s,changes)
 # All remaining old-current content comparisons are active cfg comparisons, not historical backup hashes.
 s=replace_literal(s,B['stock_custom_cfg']['sha256'],B['merged_config']['sha256'])
 s=replace_literal(s,B['stock_custom_cfg']['size'],B['merged_config']['bytes'])
 # Menu-entry count is a phase binding; historical title presence/unique-prefix validation remains intact.
 t=ast.parse(s);changes=[]
 for n in ast.walk(t):
  if isinstance(n,ast.Dict):
   for k,v in zip(n.keys,n.values):
    if isinstance(k,ast.Constant) and k.value=='entries' and isinstance(v,ast.Constant) and v.value==8:changes.append((v,'9'))
 s=edits(s,changes)
 # Replace the prospective path-existence predicate with exact creation-receipt validation for the poststage phase.
 p=B['plan'];target=[p[k] for k in ['image','incoming','backup','pending']];node=None
 for n in ast.walk(ast.parse(s)):
  if isinstance(n,ast.For):
   try:v=ast.literal_eval(n.iter)
   except:continue
   if v==target:node=n
 if node is None:raise ValueError('prospective path phase-binding missing')
 pins={p['image']:image,p['backup']:backup}
 new='''for path in TARGET:
  if path==PENDING:check(not os.path.lexists(path),'publication pending absent '+path)
  elif path==INCOMING:
   st=os.lstat(path);pin=CREATED
   check(stat.S_ISDIR(st.st_mode) and (st.st_dev,st.st_ino)==(pin['dev'],pin['ino']) and st.st_uid==1000 and st.st_gid==1000 and stat.S_IMODE(st.st_mode)==0o700,'same protected incoming creation receipt')
   check(set(os.listdir(path))=={'initrd.img-sgx535-firstload-diagnostic-01','diagnostic-entry.proposed'},'exact qualified incoming inventory')
  else:
   actual=file_id(path);pin=PINS[path];result['files']['current-staged:'+path]=actual
   check(actual.get('regular') and not actual.get('symlink') and all(actual.get(k)==pin[k] for k in ['dev','ino','size','sha256','uid','gid','mode','nlink']),'new exact staged creation receipt '+path)
'''.replace('TARGET',repr(target)).replace('PENDING',repr(p['pending'])).replace('INCOMING',repr(p['incoming'])).replace('CREATED',repr(created)).replace('PINS',repr(pins))
 s=edits(s,[(node,new)])
 # Stock original ownership/health/source-absence/default/history guards remain exactly present.
 s=s.replace(" result['boot_id_end']=", " check(result['boot_id']=="+repr(B['stock_boot'])+",'same authorized STOCK staging boot')\n result['boot_id_end']=",1)
 compile(s,'poststage','exec');return s

def capture_wrapper(source,mode,phase=None):
 """Qualified timing wrapper; append only canonical actual child arguments."""
 from importlib.util import spec_from_file_location,module_from_spec
 path=Path('/home/gama/sgx535-gfx/tools/psb-dri-re/frozen_first_load_capture_v2.py')
 spec=spec_from_file_location('qualified_capture_v2',path);mod=module_from_spec(spec);spec.loader.exec_module(mod)
 wrapper=mod.wrapper_source(source,mode)
 if phase is not None:
  args={'first-owner':['--prior-boot',B['stock_boot']]}.get(phase)
  if args is None:raise ValueError('unsupported wrapper child phase')
  dispatch_args(phase,args)
  nodes=[n.args[0] for n in ast.walk(ast.parse(wrapper)) if isinstance(n,ast.Call) and isinstance(n.func,ast.Attribute) and isinstance(n.func.value,ast.Name) and n.func.value.id=='subprocess' and n.func.attr=='run' and n.args and isinstance(n.args[0],ast.List)]
  if len(nodes)!=1:raise ValueError('one qualified root child list required')
  node=nodes[0];values=ast.literal_eval(node)
  if values[:4]!=['sudo','-n','-p',''] or values[4:9]!=['python3','-I','-B','-S','-c']:raise ValueError('qualified root invocation shape drift')
  wrapper=edits(wrapper,[(node,repr(values+args))])
 return wrapper
