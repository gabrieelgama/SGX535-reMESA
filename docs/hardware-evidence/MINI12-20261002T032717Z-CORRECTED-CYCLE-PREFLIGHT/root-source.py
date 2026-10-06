import os,hashlib,json,stat,subprocess
paths=['/boot/vmlinuz-5.10.240-antix.1-486-smp','/boot/initrd.img-5.10.240-antix.1-486-smp','/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-01','/boot/grub/grub.cfg','/boot/grub/grubenv','/boot/grub/custom.cfg']
def read(p):
 with open(p,'rb') as f:return f.read()
def ident(p):
 try:
  s=os.lstat(p)
  d={'exists':True,'mode':oct(stat.S_IMODE(s.st_mode)),'uid':s.st_uid,'gid':s.st_gid,'nlink':s.st_nlink,'regular':stat.S_ISREG(s.st_mode),'symlink':stat.S_ISLNK(s.st_mode)}
  if d['regular']:
   b=read(p);d.update(size=len(b),sha256=hashlib.sha256(b).hexdigest())
  return d
 except FileNotFoundError:return {'exists':False}
r={'boot_id':read('/proc/sys/kernel/random/boot_id').decode().strip(),'uname':os.uname()._asdict(),'product':read('/sys/class/dmi/id/product_name').decode().strip(),'taint':read('/proc/sys/kernel/tainted').decode().strip(),'loaded':{}}
try:r['loaded']={'state':read('/sys/module/gma500_gfx/initstate').decode().strip(),'note_sha256':hashlib.sha256(read('/sys/module/gma500_gfx/notes/.note.gnu.build-id')).hexdigest(),'pci_driver':os.path.realpath('/sys/bus/pci/devices/0000:00:02.0/driver'),'drm_device':os.path.realpath('/sys/class/drm/card0/device')}
except Exception as x:r['loaded_error']=repr(x)
r['paths']={p:ident(p) for p in paths}
r['cmdline']=read('/proc/cmdline').decode().strip()
print(json.dumps(r))
