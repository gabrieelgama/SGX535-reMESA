import sys,json
K={'boot': 'fc623297-419e-4bce-85d1-271c1b937d0e', 'note': '484f90964d0c36b50256e42b1bc1cd8fb905fad8476b5a1bf5e549dc178792d7', 'dest': '/home/gama/sgx535-firstload-incoming-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba-transfer2-20261006T052417Z', 'name': 'initrd.img-sgx535-firstload-diagnostic-01', 'bytes': 50816631, 'sha256': '3d9eb6baa3d821b4ff98e60ea83f1a94022b5fb2881038d2ad91b95cdfb7448e', 'dir': {'path': '/home/gama/sgx535-firstload-incoming-85ec06b428c99fac7f9127919b7a488d204f4a77-3d9eb6ba-transfer2-20261006T052417Z', 'dev': 2049, 'ino': 1049054, 'uid': 1000, 'gid': 1000, 'mode': 448, 'phase': 'created'}}
"""Reviewed exclusive-copy primitive; no transport, boot or graphics operations.

Callers supply fixed qualified files and already-verified directory descriptors.
Creation receipts are emitted before copying. Any failure retains created files
for diagnosis; this function never deletes, overwrites or retries an operation.
"""
import hashlib
import os
import stat

def snapshot_fd(fd):
 st=os.fstat(fd);os.lseek(fd,0,os.SEEK_SET);digest=hashlib.sha256();size=0
 while True:
  chunk=os.read(fd,1048576)
  if not chunk:break
  digest.update(chunk);size+=len(chunk)
 return {'dev':st.st_dev,'ino':st.st_ino,'size':size,'sha256':digest.hexdigest(),
         'uid':st.st_uid,'gid':st.st_gid,'mode':stat.S_IMODE(st.st_mode),
         'nlink':st.st_nlink,'regular':stat.S_ISREG(st.st_mode)}

def copy_exclusive(parent,name,source,size,digest,uid,gid,mode,event):
 if not name or name in ('.','..') or '/' in name or type(size) is not int or size<=0:
  raise ValueError('fixed basename/size required')
 fd=os.open(name,os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW|os.O_RDWR,0o600,dir_fd=parent)
 created=os.fstat(fd)
 try:
  event({'phase':'created','name':name,'dev':created.st_dev,'ino':created.st_ino})
  remaining=size;hasher=hashlib.sha256()
  while remaining:
   chunk=source.read(min(remaining,1048576))
   if not chunk:raise ValueError('short payload')
   if len(chunk)>remaining:raise ValueError('oversize payload')
   remaining-=len(chunk);hasher.update(chunk);view=memoryview(chunk)
   while view:
    count=os.write(fd,view)
    if count<=0:raise OSError('short/zero write')
    view=view[count:]
  if hasher.hexdigest()!=digest:raise ValueError('payload hash mismatch')
  os.fchown(fd,uid,gid);os.fchmod(fd,mode);os.fsync(fd);os.fsync(parent)
  os.close(fd);fd=-1
  fd=os.open(name,os.O_RDONLY|os.O_NOFOLLOW,dir_fd=parent)
  row=snapshot_fd(fd)
  if (row['dev'],row['ino'])!=(created.st_dev,created.st_ino) or not row['regular'] or row['nlink']!=1:
   raise ValueError('inode identity/link mismatch')
  if (row['uid'],row['gid'],row['mode'])!=(uid,gid,mode):raise ValueError('inode ownership/mode mismatch')
  if row['size']!=size or row['sha256']!=digest:raise ValueError('destination readback mismatch')
  event(dict(row,phase='verified',name=name));return row
 except BaseException as exc:
  failure={'phase':'failed','name':name,'created_dev':created.st_dev,'created_ino':created.st_ino,'error':str(exc)}
  if fd>=0:
   try:failure.update(snapshot_fd(fd))
   except OSError as read_error:failure['partial_read_error']=str(read_error)
  event(failure);raise
 finally:
  if fd>=0:os.close(fd)

r={'scope':'one exclusive exact-payload incoming transfer; no SGX','operations':[]}
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
