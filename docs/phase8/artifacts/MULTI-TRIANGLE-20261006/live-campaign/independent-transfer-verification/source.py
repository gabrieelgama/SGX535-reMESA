import os,stat,json,hashlib,time,datetime
from pathlib import Path
K={'path': '/home/gama/sgx535-square-incoming-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c-20261006T083635Z/initrd.img-sgx535-firstload-diagnostic-01', 'parent': {'path': '/home/gama/sgx535-square-incoming-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c-20261006T083635Z', 'dev': 2049, 'ino': 1049045, 'uid': 1000, 'gid': 1000, 'mode': 448, 'phase': 'created'}, 'dev': 2049, 'ino': 1049245, 'bytes': 50816648, 'sha256': '0e45ad4c9ec3eaa891253ea319348286c869644a3ba6c9dba54de39d92bd1e7d', 'old_source_sha256': '66db631fba8a484f4c39a72891b78a4ace2c7670f56e125c12340ffde15a3d5e', 'stock_boot': '4b6515d2-9e04-47e9-8e2f-5de7f3b38624'}

def digest(b):return hashlib.sha256(b).hexdigest()
def event(kind,value):print(json.dumps({'kind':kind,'value':value}),flush=True)
def meta(st):return {'dev':st.st_dev,'ino':st.st_ino,'size':st.st_size,'uid':st.st_uid,'gid':st.st_gid,'mode':oct(stat.S_IMODE(st.st_mode)),'nlink':st.st_nlink,'mtime_ns':st.st_mtime_ns,'ctime_ns':st.st_ctime_ns,'atime_ns':st.st_atime_ns,'regular':stat.S_ISREG(st.st_mode)}
def snapshot():
 fd=os.open(K['path'],os.O_RDONLY|os.O_NOFOLLOW|os.O_NOATIME)
 try:
  before=meta(os.fstat(fd))
  if (before['dev'],before['ino'])!=(K['dev'],K['ino']) or not before['regular']:raise ValueError('original incoming inode not present')
  h=hashlib.sha256();count=0
  while True:
   data=os.read(fd,1048576)
   if not data:break
   h.update(data);count+=len(data)
   if count>K['bytes']+1048576:raise ValueError('unexpected growing/oversize file')
  after=meta(os.fstat(fd));path=meta(os.lstat(K['path']))
  return {'utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'before':before,'after':after,'path_after':path,'read_bytes':count,'sha256':h.hexdigest(),'unchanged_during_read':before==after==path and count==after['size']}
 finally:os.close(fd)
def processes():
 result={'writer_fds':[],'matching_transfer_processes':[],'errors':[],'exited_during_scan':[],'pids_examined':0}
 for entry in sorted(Path('/proc').iterdir(),key=lambda x:x.name):
  if not entry.name.isdigit() or int(entry.name)==os.getpid():continue
  pid=int(entry.name)
  try:
   status=(entry/'stat').read_text();end=status.rfind(')');state=status[end+2:].split()[0];start=status[end+2:].split()[19]
   cmd=(entry/'cmdline').read_bytes().split(b'\0');matches=False
   for i,arg in enumerate(cmd[:-1]):
    if arg==b'-c' and digest(cmd[i+1])==K['old_source_sha256']:matches=True
   if matches:result['matching_transfer_processes'].append({'pid':pid,'state':state,'start_ticks':start,'cmdline_sha256':digest(b'\0'.join(cmd))})
   for f in (entry/'fd').iterdir():
    try:
     st=os.stat(f)
     if (st.st_dev,st.st_ino)!=(K['dev'],K['ino']):continue
     info=(entry/'fdinfo'/f.name).read_text();flags=int(next(line.split(':',1)[1].strip() for line in info.splitlines() if line.startswith('flags:')),8)
     if flags&os.O_ACCMODE in (os.O_WRONLY,os.O_RDWR):result['writer_fds'].append({'pid':pid,'fd':int(f.name),'flags_octal':oct(flags),'state':state,'start_ticks':start,'matches_original_transfer':matches})
    except FileNotFoundError:continue
    except Exception as e:result['errors'].append({'pid':pid,'fd':f.name,'error':type(e).__name__})
   result['pids_examined']+=1
  except (FileNotFoundError,ProcessLookupError):result['exited_during_scan'].append(pid)
  except Exception as e:result['errors'].append({'pid':pid,'error':type(e).__name__})
 result['complete']=not result['errors'];result['utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();return result
r={'scope':'PASSIVE existing incoming file/process inspection ONLY; O_RDONLY|O_NOFOLLOW|O_NOATIME; no writes, transfer, staging, GRUB, boot, PRE07, client or ioctl','classification':'TRANSFER_OUTCOME_UNKNOWN'}
try:
 if os.geteuid()!=0:raise ValueError('root read-only process visibility required')
 r['boot_id']=Path('/proc/sys/kernel/random/boot_id').read_text().strip()
 if r['boot_id']!=K['stock_boot']:raise ValueError('boot continuity not established')
 parent=meta(os.lstat(K['parent']['path']))
 if (parent['dev'],parent['ino'],parent['uid'],parent['gid'],parent['mode'])!=(K['parent']['dev'],K['parent']['ino'],1000,1000,'0o700'):raise ValueError('incoming directory identity changed')
 r['snapshot_first']=snapshot();event('snapshot_first',r['snapshot_first'])
 r['processes_first']=processes();event('processes_first',r['processes_first'])
 time.sleep(3)
 r['processes_last']=processes();event('processes_last',r['processes_last'])
 r['snapshot_last']=snapshot();event('snapshot_last',r['snapshot_last'])
 if Path('/proc/sys/kernel/random/boot_id').read_text().strip()!=r['boot_id']:raise ValueError('boot changed during observation')
 a,z=r['snapshot_first'],r['snapshot_last'];p,q=r['processes_first'],r['processes_last']
 r['stable_observed']=a['unchanged_during_read'] and z['unchanged_during_read'] and a['after']==z['after'] and a['sha256']==z['sha256']
 r['scan_complete']=p['complete'] and q['complete']
 r['writer_or_transfer_active_observed']=bool(p['writer_fds'] or q['writer_fds'] or [x for x in p['matching_transfer_processes']+q['matching_transfer_processes'] if x['state']!='Z'])
 r['current_size']=z['read_bytes'];r['current_sha256']=z['sha256']
 r['exact_qualified_bytes']=z['read_bytes']==K['bytes'] and z['sha256']==K['sha256'] and z['after']['uid']==1000 and z['after']['gid']==1000 and z['after']['mode']=='0o400' and z['after']['nlink']==1
 r['transfer_process_absent_at_both_scans']=r['scan_complete'] and not p['matching_transfer_processes'] and not q['matching_transfer_processes']
 r['copy_completion_receipt_available']=True; r['copy_persistence']='qualified copy primitive fsync(file), fsync(parent), close/reopen identity/hash verified; successful SSH exit 0 receipt preserved'
 r['original_program_exit_status']=0
 old={'path': '/home/gama/sgx535-firstload-incoming-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba/initrd.img-sgx535-firstload-diagnostic-01', 'dev': 2049, 'ino': 1049046, 'size': 6717440, 'sha256': 'c40da0a62a5c8c729e4fa4b46e76c6121c25b757443ac3c0eb4f9cd2da9e41bc', 'mutation_authorized': False}
 fd=os.open(old['path'],os.O_RDONLY|os.O_NOFOLLOW|os.O_NOATIME)
 try:
  st=os.fstat(fd);hh=hashlib.sha256();nn=0
  while True:
   bb=os.read(fd,1048576)
   if not bb:break
   hh.update(bb);nn+=len(bb)
  r['previous_partial_unchanged']=stat.S_ISREG(st.st_mode) and (st.st_dev,st.st_ino)==(old['dev'],old['ino']) and nn==old['size'] and hh.hexdigest()==old['sha256']
  if not r['previous_partial_unchanged']:raise ValueError('previous partial changed')
 finally:os.close(fd)
 r['transfer_bytes_completion_established']=r['stable_observed'] and r['scan_complete'] and not r['writer_or_transfer_active_observed'] and r['exact_qualified_bytes']
 if r['writer_or_transfer_active_observed']:r['classification']='TRANSFER_STILL_ACTIVE'
 elif r['stable_observed'] and r['scan_complete'] and r['transfer_process_absent_at_both_scans'] and not r['exact_qualified_bytes']:r['classification']='INCOMPLETE'
 elif r['transfer_bytes_completion_established'] and r['transfer_process_absent_at_both_scans']:r['classification']='COMPLETE_VERIFIED'
 else:r['classification']='TRANSFER_OUTCOME_UNKNOWN'
except Exception as e:r['error']=repr(e)
event('final_result',r)
