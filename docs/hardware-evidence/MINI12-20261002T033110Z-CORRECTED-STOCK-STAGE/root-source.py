import os,hashlib,json,stat,shutil,base64
paths=['/boot/vmlinuz-5.10.240-antix.1-486-smp','/boot/initrd.img-5.10.240-antix.1-486-smp','/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-01','/boot/grub/grub.cfg','/boot/grub/grubenv','/boot/grub/custom.cfg','/boot/initrd.img-5.10.240-antix.1-486-smp-sgx535-firstload-corrected-01','/boot/grub/custom.cfg.cycle06-preserved']
def rd(p):
 with open(p,'rb') as f:return f.read()
def ident(p):
 try:
  s=os.lstat(p);d={'exists':True,'mode':oct(stat.S_IMODE(s.st_mode)),'uid':s.st_uid,'gid':s.st_gid,'nlink':s.st_nlink,'regular':stat.S_ISREG(s.st_mode),'symlink':stat.S_ISLNK(s.st_mode)}
  if d['regular']:
   b=rd(p);d.update(size=len(b),sha256=hashlib.sha256(b).hexdigest())
   if p.endswith('/custom.cfg'):d['base64']=base64.b64encode(b).decode()
  return d
 except FileNotFoundError:return {'exists':False}
r={'boot_id':rd('/proc/sys/kernel/random/boot_id').decode().strip(),'kernel':os.uname().release,'machine':os.uname().machine,'product':rd('/sys/class/dmi/id/product_name').decode().strip(),'cmdline':rd('/proc/cmdline').decode().strip(),'taint':rd('/proc/sys/kernel/tainted').decode().strip(),'loaded':{'state':rd('/sys/module/gma500_gfx/initstate').decode().strip(),'note_sha256':hashlib.sha256(rd('/sys/module/gma500_gfx/notes/.note.gnu.build-id')).hexdigest(),'pci_driver':os.path.realpath('/sys/bus/pci/devices/0000:00:02.0/driver'),'drm_device':os.path.realpath('/sys/class/drm/card0/device')},'paths':{p:ident(p) for p in paths},'boot_free_bytes':shutil.disk_usage('/boot').free,'boot_parent':{p:{'mode':oct(stat.S_IMODE(os.stat(p).st_mode)),'uid':os.stat(p).st_uid,'gid':os.stat(p).st_gid,'symlink':os.path.islink(p)} for p in ['/boot','/boot/grub']}}
print(json.dumps(r))
