import os,stat,hashlib,json,subprocess,datetime,sys
R='5.10.240-antix.1-486-smp'; B='/sys/bus/pci/devices/0000:00:02.0'; E='/home/gama/sgx535-firstload-corrected-incoming-01'; UID=1000; GID=1000
ID='10d6abfa-e7a8-4311-97ae-213f503cee20'; NOTE='484f90964d0c36b50256e42b1bc1cd8fb905fad8476b5a1bf5e549dc178792d7'
IMG='/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-corrected-01'; CFG='/boot/grub/custom.cfg'; BACK='/boot/grub/custom.cfg.cycle06-preserved'; TMP='/boot/grub/.custom.cfg.corrected-01.pending'
IMAGE_SHA='55e7a8be6f62c9a1d59f1c532b8790399c21a2706884747e504edfffb45bf52d'; IMAGE_SIZE=50804478
ENTRY_SHA='16f0ef7abc9cf330808c843ed0b17afef99cd562407bd801edc3bed5295563f8'; ENTRY_SIZE=1004
OLD_CFG_SHA='181d33aa42cc9a73d35a50b5e7b1636121edb7a607c2949540fa04a4d0d32612'; OLD_CFG_SIZE=974
MERGED_SHA='269cce045b81da002318f9b478e07f05d235a773c3c82fd6557d8d50da7f38c8'; MERGED_SIZE=2040
NEW_PATH='/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-corrected-01'
NEW_TITLE='EXPERIMENTAL SGX535 rev121 CORRECTED FIRST-LOAD ONLY (no triangle)'; NEW_ID='sgx535-rev121-firstload-corrected-01'
STOCK=[('/boot/vmlinuz-'+R,5984416,'cda6e6c7f61cae83793c974f745af888211bb3543d411283b9bcf79ce5304438'),('/boot/initrd.img-'+R,50863580,'f02cde6c7e712fa0620f99438ef5dc81c131b481a84dc9dbc4f7b854c364e341'),('/boot/grub/grub.cfg',10438,'396ac7dff0bdea48393fd0a25a5fc633d40e2368caf14872b855eba03e96089f'),('/boot/grub/grubenv',1024,'72c291233d508c8ed06305f0bf9d33130ea206bfaf67873dbe77413f77d19927'),('/lib/modules/'+R+'/kernel/drivers/gpu/drm/gma500/gma500_gfx.ko',None,'7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb')]
result={'classification':'IN PROGRESS','start_utc':datetime.datetime.now(datetime.timezone.utc).isoformat(),'operations':[],'stock_checks':[]}
def need(x,msg):
 if not x:raise RuntimeError(msg)
def digest(b):return hashlib.sha256(b).hexdigest()
def read_nofollow(path):
 fd=os.open(path,os.O_RDONLY|os.O_NOFOLLOW);s=os.fstat(fd);need(stat.S_ISREG(s.st_mode) and s.st_nlink==1,'regular single-link input '+path);data=b''
 while True:
  q=os.read(fd,1024*1024)
  if not q:break
  data+=q
 os.close(fd);return s,data
def absent(path):
 try:os.lstat(path);return False
 except FileNotFoundError:return True
def parent_sync(path):
 fd=os.open(os.path.dirname(path),os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW);os.fsync(fd);os.close(fd)
def create_file(path,data):
 fd=os.open(path,os.O_WRONLY|os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW,0o600)
 view=memoryview(data);written=0
 while written<len(view):written+=os.write(fd,view[written:])
 os.fchown(fd,0,0);os.fchmod(fd,0o644);os.fsync(fd);s=os.fstat(fd);os.close(fd);parent_sync(path)
 st,b=read_nofollow(path);need(digest(b)==digest(data) and len(b)==len(data),'write readback '+path);need(st.st_uid==0 and st.st_gid==0 and stat.S_IMODE(st.st_mode)==0o644 and st.st_nlink==1,'published metadata '+path)
 result['operations'].append({'path':path,'size':len(b),'sha256':digest(b),'dev':st.st_dev,'ino':st.st_ino,'uid':st.st_uid,'gid':st.st_gid,'mode':oct(stat.S_IMODE(st.st_mode)),'nlink':st.st_nlink,'fsync':'PASS','readback':'PASS'})
def check_stock():
 for p,size,h in STOCK:
  s,b=read_nofollow(p);ok=(size is None or len(b)==size) and digest(b)==h
  result['stock_checks'].append({'path':p,'size':len(b),'sha256':digest(b),'pass':ok});need(ok,'stock file drift '+p)
need(os.geteuid()==0,'root required');need(os.uname().release==R and os.uname().machine=='i686','kernel identity drift')
need(open('/proc/sys/kernel/random/boot_id').read().strip()==ID,'boot changed since fresh preflight')
need(open('/sys/module/gma500_gfx/initstate').read().strip()=='live','stock module no longer Live')
need(digest(open('/sys/module/gma500_gfx/notes/.note.gnu.build-id','rb').read())==NOTE,'loaded module identity changed')
need(os.path.realpath(B+'/driver')=='/sys/bus/pci/drivers/gma500','stock PCI ownership changed')
need(os.path.realpath('/sys/class/drm/card0/device')=='/sys/devices/pci0000:00/0000:00:02.0','stock DRM ownership changed')
check_stock()
need(absent(IMG),'corrected image destination already exists')
need(absent(BACK),'Cycle06 config backup destination already exists')
need(absent(TMP),'pending config path already exists')
src_img=E+'/initrd.img-sgx535-firstload-01';src_entry=E+'/corrected-entry.proposed'
si,ib=read_nofollow(src_img);need(si.st_uid==UID and si.st_gid==GID and len(ib)==IMAGE_SIZE and digest(ib)==IMAGE_SHA,'incoming corrected image identity')
se,entry=read_nofollow(src_entry);need(se.st_uid==UID and se.st_gid==GID and len(entry)==ENTRY_SIZE and digest(entry)==ENTRY_SHA,'incoming entry identity')
entry_text=entry.decode('utf-8');need(entry_text.count(NEW_TITLE)==1 and entry_text.count(NEW_ID)==1 and entry_text.count(NEW_PATH)==1,'corrected entry fields')
need('savedefault' not in entry_text and 'save_env' not in entry_text,'entry must not save boot choice')
need(stat.S_ISDIR(os.lstat('/boot').st_mode) and stat.S_ISDIR(os.lstat('/boot/grub').st_mode) and os.stat('/boot').st_uid==0 and os.stat('/boot/grub').st_uid==0,'boot parent trust')
need(os.statvfs('/boot').f_bavail*os.statvfs('/boot').f_frsize>=134217728+IMAGE_SIZE,'boot free-space margin')
# Stage image first, exclusively; Cycle06 image remains untouched.
create_file(IMG,ib)
# Preserve the exact Cycle06 custom.cfg as a distinct exclusive backup.
oldst,old=read_nofollow(CFG);need(oldst.st_uid==0 and oldst.st_gid==0 and stat.S_IMODE(oldst.st_mode)==0o644 and len(old)==OLD_CFG_SIZE and digest(old)==OLD_CFG_SHA,'Cycle06 custom.cfg identity changed')
create_file(BACK,old)
merged=old+(b'' if old.endswith(b'\n') else b'\n')+b'\n# Corrected private-GPU-VA candidate; manual selection only.\n'+entry
need(len(merged)==MERGED_SIZE and digest(merged)==MERGED_SHA,'merged GRUB config identity')
need(merged.startswith(old) and merged.count(b'menuentry ')==2 and merged.count(NEW_PATH.encode())==1 and merged.count(NEW_ID.encode())==1 and b'savedefault' not in merged and b'save_env' not in merged,'merged GRUB semantics')
create_file(TMP,merged)
# Recheck the exact original inode/bytes immediately before atomic publication.
cur,bcur=read_nofollow(CFG);need((cur.st_dev,cur.st_ino)==(oldst.st_dev,oldst.st_ino) and digest(bcur)==OLD_CFG_SHA,'custom.cfg changed before publish')
os.replace(TMP,CFG);parent_sync(CFG)
finalst,final=read_nofollow(CFG);need(len(final)==MERGED_SIZE and digest(final)==MERGED_SHA and final.startswith(old),'published custom.cfg readback')
result['published_custom_cfg']={'size':len(final),'sha256':digest(final),'old_cycle06_bytes_exact_prefix':final.startswith(old),'backup_sha256':digest(read_nofollow(BACK)[1]),'backup_size':len(read_nofollow(BACK)[1]),'menuentry_count':final.count(b'menuentry '),'corrected_path_count':final.count(NEW_PATH.encode()),'savedefault':b'savedefault' in final,'save_env':b'save_env' in final}
# Recheck all preserved stock and Cycle06 artifacts after publication.
check_stock();oldst2,oldimg=read_nofollow('/boot/initrd.img-'+R+'-sgx535-firstload-01');need(len(oldimg)==50804481 and digest(oldimg)=='4ac6bd1bed7dbf53b3653037f3e6e757160672560176b1938a63488636efaa71','Cycle06 image changed')
need(open('/proc/sys/kernel/random/boot_id').read().strip()==ID,'boot changed during staging')
need(open('/sys/module/gma500_gfx/initstate').read().strip()=='live' and digest(open('/sys/module/gma500_gfx/notes/.note.gnu.build-id','rb').read())==NOTE,'stock module changed during staging')
need(os.path.realpath(B+'/driver')=='/sys/bus/pci/drivers/gma500','PCI owner changed during staging')
result['classification']='STAGING PASS; NO REBOOT/BOOT/SGX ACTION';result['end_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();print(json.dumps(result,indent=2))
