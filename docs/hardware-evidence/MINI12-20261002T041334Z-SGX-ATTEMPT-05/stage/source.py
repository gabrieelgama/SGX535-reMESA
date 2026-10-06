import os,sys,stat,hashlib,json
BASE='/root/sgx535-frozen-seq1-203a5b9b'
try:
 assert os.geteuid()==0
 assert open('/proc/sys/kernel/random/boot_id').read().strip()=='203a5b9b-5a90-4fd3-8001-3c92cdefeddd'
 assert os.uname().release=='5.10.240-antix.1-486-smp' and os.uname().machine=='i686'
 assert hashlib.sha256(open('/sys/module/gma500_gfx/notes/.note.gnu.build-id','rb').read()).hexdigest()=='b78dc1ffde5cc661d076941e27e10b06edf62ad1da37651f3c38a98e622890c2'
 data=sys.stdin.buffer.read(771321)
 assert len(data)==771320 and hashlib.sha256(data).hexdigest()=='758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf'
 pfd=os.open('/root',os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW);pst=os.fstat(pfd)
 assert pst.st_uid==0 and not stat.S_IMODE(pst.st_mode)&0o022
 os.mkdir(os.path.basename(BASE),0o700,dir_fd=pfd);os.fsync(pfd)
 dfd=os.open(os.path.basename(BASE),os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW,dir_fd=pfd)
 fd=os.open('frozen-triangle-one-shot-i386',os.O_WRONLY|os.O_CREAT|os.O_EXCL|os.O_NOFOLLOW,0o700,dir_fd=dfd)
 with os.fdopen(fd,'wb') as f:f.write(data);f.flush();os.fsync(f.fileno())
 os.fsync(dfd)
 fd=os.open('frozen-triangle-one-shot-i386',os.O_RDONLY|os.O_NOFOLLOW,dir_fd=dfd);st=os.fstat(fd)
 with os.fdopen(fd,'rb') as f:readback=f.read()
 assert stat.S_ISREG(st.st_mode) and st.st_uid==0 and st.st_nlink==1 and stat.S_IMODE(st.st_mode)==0o700 and readback==data
 os.close(dfd);os.close(pfd)
 print(json.dumps({'classification':'PASS EXCLUSIVE CLIENT STAGING','base':BASE,'path':BASE+'/frozen-triangle-one-shot-i386','bytes':len(readback),'sha256':hashlib.sha256(readback).hexdigest(),'uid':st.st_uid,'mode':stat.S_IMODE(st.st_mode),'nlink':st.st_nlink,'dev':st.st_dev,'ino':st.st_ino,'fsync':'file and directories','no_sgx_yet':True}))
except Exception as exc:
 print(json.dumps({'classification':'STOP BEFORE IOCTL','error':repr(exc),'no_retry':True}));sys.exit(1)
