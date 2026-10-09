/* SPDX-License-Identifier: MIT */
#include "remesa_dri.h"
#include "remesa_loader.h"
#include "pipe/p_screen.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <unistd.h>
/* Sysfs PCI identity is tied to the descriptor's character device. It does not
 * establish SGX core revision, native ioctl support or execution permission. */
static int pci_value(int directory,const char *name,unsigned *value)
{
 char text[32];char *end;ssize_t n;int fd=openat(directory,name,O_RDONLY|O_CLOEXEC);
 if(fd<0) return errno;
 n=read(fd,text,sizeof(text)-1);int error=n<0 ? errno : 0;close(fd);
 if(error) return error;
 if((n!=6 && n!=7) || text[0]!='0' || text[1]!='x' || (n==7 && text[6]!='\n')) return EPROTO;
 for(unsigned i=2;i<6;i++)
  if(!((text[i]>='0'&&text[i]<='9') || (text[i]>='a'&&text[i]<='f') || (text[i]>='A'&&text[i]<='F'))) return EPROTO;
 text[n]=0;errno=0;unsigned long v=strtoul(text,&end,0);
 if(errno || end==text || v>0xffff) return EPROTO;
 if(*end=='\n') ++end;
 if(*end) return EPROTO;
 *value=(unsigned)v;return 0;
}
int remesa_dri_probe_fd(int fd,struct remesa_dri_identity *identity)
{
 struct stat st;char path[96];int directory,error;struct remesa_dri_identity v={0};
 if(fd<0 || !identity) return EINVAL;
 if(fstat(fd,&st)) return errno;
 if(!S_ISCHR(st.st_mode)) return ENODEV;
 /* DRM's allocated Linux major is 226. Reject unrelated character devices. */
 if(major(st.st_rdev)!=226) return ENODEV;
 int n=snprintf(path,sizeof(path),"/sys/dev/char/%u:%u/device",major(st.st_rdev),minor(st.st_rdev));
 if(n<0 || (size_t)n>=sizeof(path)) return EOVERFLOW;
 directory=open(path,O_RDONLY|O_DIRECTORY|O_CLOEXEC);if(directory<0) return errno;
 error=pci_value(directory,"vendor",&v.vendor);
 if(!error) error=pci_value(directory,"device",&v.device);
 close(directory);
 if(error) return error;
 if(v.vendor!=0x8086 || v.device!=0x8108) return ENODEV;
 *identity=v;return 0;
}
static struct pipe_screen *create_screen(int fd,const struct pipe_screen_config *config)
{
 struct remesa_dri_identity identity;struct pipe_screen *screen;int error;
 (void)config;
 /* Bind identity to a retained fd, not a caller descriptor that might close. */
 int guard=fcntl(fd,F_DUPFD_CLOEXEC,0);if(guard<0) return NULL;
 error=remesa_dri_probe_fd(guard,&identity);
 if(error) {close(guard);errno=error;return NULL;}
 /* Validate DRM name and duplicate ownership via the existing clean transport.
  * No draw/memory ioctl occurs. Never expose the diagnostic screen to Mesa's
  * state tracker: restricted NIR/CSOs do not supply general GL contracts. */
 screen=remesa_loader_diagnostic_screen(guard,&error);close(guard);
 if(!screen) {errno=error;return NULL;}
 screen->destroy(screen);errno=ENOTSUP;return NULL;
}
const struct drm_driver_descriptor gma500_driver_descriptor={
 .driver_name="gma500",.create_screen=create_screen
};
const char *remesa_dri_unavailable_reason(void)
{
 return "reMESA OpenGL unavailable: restricted NIR only; Mesa pixel-center/edge rules, general shaders, GPU clear, samplers and truthful GL caps are missing";
}
