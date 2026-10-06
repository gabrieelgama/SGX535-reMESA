import sys
pass # historical sys.argv override removed; explicit dispatched arguments are authoritative
import os,sys,stat,pwd,json,hashlib,argparse,uuid
p=argparse.ArgumentParser();p.add_argument('--prepare',action='store_true');p.add_argument('--stock-boot');args=p.parse_args()
DEST='/home/gama/sgx535-square-incoming-8be2777b2a79eaa6651b89d19faf4d68cdcdc460-0e45ad4c-20261006T083635Z'
if not args.prepare:
 print(json.dumps({'mode':'PREPARATION ONLY','destination':DEST,'writes':False,'sgx':False}));sys.exit(0)
try:
 if str(uuid.UUID(args.stock_boot))!=args.stock_boot:raise ValueError('UUID spelling')
except (ValueError,TypeError,AttributeError):p.error('canonical fresh STOCK UUID required')
result={'classification':'IN PROGRESS','operations':[],'sgx':False,'boot':False,'retry':False}
def need(x,msg):
 if not x:raise RuntimeError(msg)
try:
 need(os.geteuid()==1000 and pwd.getpwnam('gama').pw_uid==1000 and pwd.getpwnam('gama').pw_gid==1000,'expected target user')
 need(os.uname().release=='5.10.240-antix.1-486-smp' and os.uname().machine=='i686','kernel drift')
 need(open('/proc/sys/kernel/random/boot_id').read().strip()==args.stock_boot,'STOCK boot drift')
 need(open('/sys/module/gma500_gfx/initstate').read().strip()=='live','STOCK Live state')
 need(hashlib.sha256(open('/sys/module/gma500_gfx/notes/.note.gnu.build-id','rb').read()).hexdigest()=='484f90964d0c36b50256e42b1bc1cd8fb905fad8476b5a1bf5e549dc178792d7','STOCK module identity')
 need(os.path.realpath('/sys/bus/pci/devices/0000:00:02.0/driver')=='/sys/bus/pci/drivers/gma500','PCI ownership drift')
 st=os.lstat('/home/gama');need(stat.S_ISDIR(st.st_mode) and st.st_uid==1000 and not st.st_mode&0o022,'protected user home')
 os.mkdir(DEST,0o700)
 st=os.lstat(DEST);result['operations'].append({'path':DEST,'dev':st.st_dev,'ino':st.st_ino,'uid':st.st_uid,'gid':st.st_gid,'mode':stat.S_IMODE(st.st_mode),'phase':'created'})
 need(stat.S_ISDIR(st.st_mode) and st.st_uid==1000 and st.st_gid==1000 and stat.S_IMODE(st.st_mode)==0o700 and not os.listdir(DEST),'exclusive empty protected incoming')
 fd=os.open('/home/gama',os.O_RDONLY|os.O_DIRECTORY|os.O_NOFOLLOW)
 try:os.fsync(fd)
 finally:os.close(fd)
 need(open('/proc/sys/kernel/random/boot_id').read().strip()==args.stock_boot,'boot changed during incoming preparation')
 result['classification']='INCOMING PREPARATION PASS';result['boot_id']=args.stock_boot
 print(json.dumps(result,indent=2))
except Exception as exc:
 result['classification']='HOLD: INCOMING INCOMPLETE OR NOT STARTED';result['error']=repr(exc);print(json.dumps(result,indent=2));sys.exit(1)
