/* SPDX-License-Identifier: MIT */
#include "remesa_dri.h"
#include "remesa_linux.h"
#include "remesa_screen.h"
#include "pipe/p_screen.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
unsigned remesa_dri_missing_contracts(void)
{
 return REMESA_DRI_MISSING_SHADER_PIPELINE | REMESA_DRI_MISSING_TEXTURE_SAMPLING |
        REMESA_DRI_MISSING_RASTER_RULES | REMESA_DRI_MISSING_RENDER_STATES |
        REMESA_DRI_MISSING_COLOR_PRESERVATION;
}
const char *remesa_dri_stage_name(enum remesa_dri_stage stage)
{
 switch(stage) {
 case REMESA_DRI_RETAIN_FD:return "retain-fd";
 case REMESA_DRI_PCI_IDENTITY:return "pci-identity";
 case REMESA_DRI_DRM_TRANSPORT:return "drm-transport";
 case REMESA_DRI_GALLIUM_SCREEN:return "gallium-screen";
 case REMESA_DRI_OPENGL_ADMISSION:return "opengl-admission";
 }
 return "invalid-stage";
}
static int close_device(struct remesa_device *device)
{ return remesa_linux_close_checked(device); }
struct pipe_screen *remesa_dri_create_screen(int fd,
 const struct pipe_screen_config *config,struct remesa_dri_report *report)
{
 struct remesa_dri_report result={.stage=REMESA_DRI_RETAIN_FD};
 struct remesa_device device={0};struct pipe_screen *screen=NULL;int error;
 (void)config;
 if(!report) {errno=EINVAL;return NULL;}
 /* Bind identity to a retained fd, not a caller descriptor that might close. */
 int guard=fcntl(fd,F_DUPFD_CLOEXEC,0);
 if(guard<0) {error=errno;goto done;}
 result.stage=REMESA_DRI_PCI_IDENTITY;
 error=remesa_dri_probe_fd(guard,&result.identity);
 if(error) {close(guard);goto done;}
 result.identity_validated=true;
 /* Validate DRM name and duplicate ownership via the existing clean transport.
  * No draw/memory ioctl occurs. Never expose the diagnostic screen to Mesa's
  * state tracker: restricted NIR/CSOs do not supply general GL contracts. */
 result.stage=REMESA_DRI_DRM_TRANSPORT;
 error=remesa_linux_open_fd(&device,guard);close(guard);
 if(error) goto done;
 result.transport_validated=true;
 result.stage=REMESA_DRI_GALLIUM_SCREEN;
 screen=remesa_screen_create_owned(&device,close_device);
 if(!screen) {remesa_linux_close(&device);error=ENOMEM;goto done;}
 result.screen_created=true;
 /* Reaching this point establishes allocation and DRM descriptor ownership,
  * not a complete OpenGL implementation. The temporary diagnostic screen must
  * be destroyed before admission rejection; it is never returned to Mesa. */
 screen->destroy(screen);
 result.stage=REMESA_DRI_OPENGL_ADMISSION;
 result.missing_contracts=remesa_dri_missing_contracts();error=ENOTSUP;
done:
 result.error=error;*report=result;errno=error;return NULL;
}
static struct pipe_screen *create_screen(int fd,const struct pipe_screen_config *config)
{
 struct remesa_dri_report report;
 struct pipe_screen *screen=remesa_dri_create_screen(fd,config,&report);
 const char *trace=getenv("REMESA_DRI_TRACE");
 if(trace && !strcmp(trace,"1"))
  fprintf(stderr,"reMESA DRI: stage=%s errno=%d identity=%u transport=%u screen=%u pci=%04x:%04x missing=0x%x\n",
          remesa_dri_stage_name(report.stage),report.error,
          report.identity_validated,report.transport_validated,report.screen_created,
          report.identity.vendor,report.identity.device,report.missing_contracts);
 errno=report.error;return screen;
}
const struct drm_driver_descriptor gma500_driver_descriptor={
 .driver_name="gma500",.create_screen=create_screen
};
const char *remesa_dri_unavailable_reason(void)
{
 return "reMESA OpenGL unavailable: baseline shader pipeline, texture sampling, raster rules, render states and COLOR preservation are missing; diagnostic Gallium screen is not an OpenGL screen";
}
