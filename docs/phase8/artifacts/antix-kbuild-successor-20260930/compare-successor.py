from pathlib import Path
import json,hashlib,difflib,re,tarfile,subprocess,stat
W=Path(__file__).resolve().parent
P=json.loads((W/'paths.json').read_text()); O=Path(P['output']); S=Path(P['source']); C=Path(P['captured_target']); Q=json.loads((W/'build-environment.json').read_text())
assert json.loads((W/'modules_prepare.json').read_text()).get('exit_code')==0
T=W/'captured-reference';T.mkdir(exist_ok=False)
with tarfile.open(C/'build_generated.tar') as tar:
 for m in tar.getmembers():
  assert not Path(m.name).is_absolute() and '..' not in Path(m.name).parts
  assert m.isdir() or m.isfile()
 tar.extractall(T,filter='data')
D=W/'comparison-diffs';D.mkdir(exist_ok=False)
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
EX='EXACT MATCH';META='EXPECTED/DOCUMENTED CROSS-BUILD METADATA DIFFERENCE';NON='UNDERSTOOD NON-ABI/NON-LAYOUT DIFFERENCE';DRIFT='UNEXPLAINED DRIFT'
BANNER_TARGET='gcc (Debian 14.2.0-19) 14.2.0';BANNER_NEW='i686-linux-gnu-gcc-14 (Debian 14.2.0-19) 14.2.0'
rows=[]
def compare(name,target,new,kind='exact'):
 t=target.read_bytes();n=new.read_bytes() if new.exists() else None
 row={'item':name,'target_path':str(target),'successor_path':str(new),'target_sha256':sha(target),'successor_sha256':sha(new) if n is not None else None,'target_bytes':len(t),'successor_bytes':len(n) if n is not None else None}
 if n==t:row.update(classification=EX,reason='Byte-exact match')
 elif kind=='compile-identity' and n is None:
  row.update(classification=NON,reason='compile.h is full-kernel init/version.o build-identity metadata; modules_prepare does not generate it; no copied/spoofed identity')
 elif kind=='banner' and n is not None and n.decode().replace(BANNER_NEW,BANNER_TARGET)==t.decode():
  row.update(classification=META,reason='Only the previously documented compiler executable-name banner; raw bytes retained',comparison_only_transform={'replace':BANNER_NEW,'with':BANNER_TARGET})
 elif kind=='dependencies' and n is not None:
  ts=t.decode();ns=n.decode();pat=re.compile(r'^ifneq "\$\(([^)]+)\)" "(.*)"$',re.M);td=dict(pat.findall(ts));nd=dict(pat.findall(ns));keys=[k for k in sorted(td.keys()|nd.keys()) if td.get(k)!=nd.get(k)]
  expected={'CC':('gcc',Q['CC']),'LD':('ld',Q['LD']),'srctree':('.',str(S)),'CC_VERSION_TEXT':(BANNER_TARGET,BANNER_NEW),'NM':('nm',Q['CROSS_COMPILE']+'nm'),'OBJCOPY':('objcopy',Q['CROSS_COMPILE']+'objcopy')}
  ok=set(keys)==set(expected) and all((td.get(k),nd.get(k))==v for k,v in expected.items())
  def scrub(text):return pat.sub(lambda m: 'ifneq "$('+m[1]+')" "<reviewed dependency metadata>"' if m[1] in expected else m[0],text)
  ok=ok and scrub(ts)==scrub(ns)
  row.update(classification=NON if ok else DRIFT,reason='Exactly six tool/path/banner dependency selectors change; dependency list and remaining content exact' if ok else 'Unexpected dependency metadata change',selectors=[{'key':k,'target':td.get(k),'successor':nd.get(k)} for k in keys],comparison_only_transform='replace only six explicitly validated ifneq dependency-selector values; raw diffs retained')
 else:row.update(classification=DRIFT,reason='Unexpected mismatch or missing required generated input')
 if n is not None and n!=t:
  diff=''.join(difflib.unified_diff(t.decode().splitlines(True),n.decode().splitlines(True),fromfile='target/'+name,tofile='successor/'+name))
  file=D/(name.replace('/','__')+'.diff');file.write_text(diff);row['raw_diff']=str(file.relative_to(W))
 rows.append(row);return row
headers=[]
for f in sorted(x for x in T.rglob('*') if x.is_file()):
 rel=str(f.relative_to(T));kind='banner' if rel=='include/generated/autoconf.h' else 'compile-identity' if rel=='include/generated/compile.h' else 'exact'
 headers.append(compare(rel,f,O/rel,kind))
compare('.config',C/'build__config',O/'.config','banner')
compare('include/config/auto.conf',C/'build_include_config_auto_conf',O/'include/config/auto.conf','banner')
compare('include/config/auto.conf.cmd',C/'build_include_config_auto_conf_cmd',O/'include/config/auto.conf.cmd','dependencies')
compare('include/config/kernel.release',C/'build_include_config_kernel_release',O/'include/config/kernel.release')
compare('source/Makefile',C/'build_Makefile',S/'Makefile')
# Whole semantic configuration comparisons preserve disabled and enabled values.
def config(path):
 d={}
 for l in path.read_text().splitlines():
  if l.startswith('CONFIG_') and '=' in l:k,v=l.split('=',1);assert k not in d;d[k]=v
  elif l.startswith('# CONFIG_') and l.endswith(' is not set'):k=l[2:-11];assert k not in d;d[k]='n'
 return d
ct=config(C/'build__config');cn=config(O/'.config');delta=[{'key':k,'target':ct.get(k),'successor':cn.get(k)} for k in sorted(ct.keys()|cn.keys()) if ct.get(k)!=cn.get(k)]
config_ok=len(ct)==len(cn)==8788 and len(delta)==1 and delta[0]=={'key':'CONFIG_CC_VERSION_TEXT','target':'"'+BANNER_TARGET+'"','successor':'"'+BANNER_NEW+'"'}
assert (T/'include/generated/autoconf.h').read_bytes()==(C/'build_include_generated_autoconf_h').read_bytes()
# Include every extra generated header/dependency record, not just captured intersections.
extras=[]
for base in ['include/generated','arch/x86/include/generated']:
 for f in sorted((O/base).rglob('*')):
  if f.is_file() and not (T/f.relative_to(O)).exists():
   rel=str(f.relative_to(O));allowed=rel in [f'arch/x86/include/generated/uapi/asm/.unistd_{abi}.h.cmd' for abi in ['32','64','x32']]+['arch/x86/include/generated/asm/.syscalls_32.h.cmd']
   extras.append({'item':rel,'sha256':sha(f),'classification':NON if allowed else DRIFT,'reason':'Kbuild command/dependency record, not interface/layout definitions' if allowed else 'Unreviewed extra generated input'})
log=(W/'modules_prepare.stdout').read_text();sysrel='arch/x86/include/generated/asm/syscalls_32.h';sys=O/sysrel;lines=[l for l in log.splitlines() if 'syscalltbl.sh' in l and sysrel in l]
prep=json.loads((W/'modules_prepare.json').read_text());st=sys.stat();fresh={'path':str(sys),'sha256':sha(sys),'size':st.st_size,'lines':len(sys.read_text().splitlines()),'mode':oct(stat.S_IMODE(st.st_mode)),'inode':st.st_ino,'device':st.st_dev,'mtime_ns':st.st_mtime_ns,'absent_before_preparation':not prep['successor_syscall_exists_before_stage'],'expected_generator_commands':lines,'matches_captured_target':sys.read_bytes()==(T/sysrel).read_bytes(),'contains_syscall_440':'(440,' in sys.read_text(),'contains_syscall_239':'(239,' in sys.read_text(),'distinct_from_failed_inode':(st.st_dev,st.st_ino)!=((Path(P['failed_output'])/sysrel).stat().st_dev,(Path(P['failed_output'])/sysrel).stat().st_ino)}
# Equality plus generation provenance proves completeness; no target header installed.
sys_ok=fresh['absent_before_preparation'] and len(lines)==1 and fresh['matches_captured_target'] and fresh['contains_syscall_440'] and fresh['distinct_from_failed_inode']
elf=[]
for rel,expected in [('scripts/basic/fixdep','AArch64'),('scripts/kconfig/conf','AArch64'),('scripts/mod/modpost','AArch64'),('scripts/mod/empty.o','Intel 80386')]:
 f=O/rel;r=subprocess.run(['/usr/bin/readelf','-h',str(f)],capture_output=True,text=True);ok=r.returncode==0 and expected in r.stdout and ('ELF32' if expected=='Intel 80386' else 'ELF64') in r.stdout
 elf.append({'file':rel,'sha256':sha(f),'argv':['/usr/bin/readelf','-h',str(f)],'exit_code':r.returncode,'stdout':r.stdout,'stderr':r.stderr,'expected_architecture_verified':ok})
gate=config_ok and sys_ok and all(x['classification']!=DRIFT for x in rows+extras) and all(x['expected_architecture_verified'] for x in elf) and (O/'include/config/kernel.release').read_text().strip()=='5.10.240-antix.1-486-smp'
result={'equivalence':'PASS' if gate else 'NOT ESTABLISHED','captured_header_count':len(headers),'exact_captured_header_count':sum(x['classification']==EX for x in headers),'full_comparison_rows':rows,'extra_generated_inputs':extras,'semantic_config':{'target_symbols':len(ct),'successor_symbols':len(cn),'differences':delta,'pass':config_ok},'syscall_generation':fresh,'elf_identities':elf,'unexpected_drift':[x for x in rows+extras if x['classification']==DRIFT],'source_Module_symvers_exists':(S/'Module.symvers').exists(),'output_Module_symvers_exists':(O/'Module.symvers').exists(),'candidate_compilation_in_this_task':False}
(W/'complete-equivalence.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps({'equivalence':result['equivalence'],'captured_headers':len(headers),'exact_headers':result['exact_captured_header_count'],'semantic_config':result['semantic_config'],'syscall':fresh,'non_exact_items':[{k:x[k] for k in ['item','classification','reason']} for x in rows+extras if x['classification']!=EX]},indent=2))
raise SystemExit(0 if gate else 1)
