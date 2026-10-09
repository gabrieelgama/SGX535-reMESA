/* SPDX-License-Identifier: MIT */
/* Offline ABI probe for a REAL Mesa build. No device discovery, GPU ioctl,
 * framebuffer generation or X server access. A loadable DRIL stub is not GL. */
#include "dri_util.h"
#include "remesa_loader_sandbox.h"
#include <sys/ioctl.h>
#include <dlfcn.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* DRIL uses the legacy DRI extension record; Mesa's private frontend header
 * defines the same two-field prefix. Check that prefix in this pinned build. */
_Static_assert(offsetof(__DRIextension,name)==0,"DRI name ABI");
_Static_assert(offsetof(__DRIextension,version)==sizeof(void *),"DRI version ABI");

static int descriptor_count(void)
{
 DIR *d=opendir("/proc/self/fd");if(!d)return -1;
 int n=0;struct dirent *e;
 while((e=readdir(d))) if(strcmp(e->d_name,".")&&strcmp(e->d_name,"..")) ++n;
 closedir(d);return n;
}

int main(int argc,char **argv)
{
 if(argc!=3 && argc!=4) {fprintf(stderr,"usage: %s gma500_dri.so libgallium.so [--deny-ioctls]\n",argv[0]);return 2;}
 if(argc==4) {
  if(strcmp(argv[3],"--deny-ioctls")){fprintf(stderr,"SANDBOX_OPTION_STOP\n");return 2;}
  int error=remesa_loader_deny_ioctls();
  if(error){fprintf(stderr,"SANDBOX_SETUP_STOP: %d\n",error);return 1;}
  errno=0;if(ioctl(-1,0,NULL)!=-1 || errno!=EPERM){fprintf(stderr,"SANDBOX_ENFORCEMENT_STOP\n");return 1;}
  puts("IOCTL_SANDBOX=ENFORCED (all ioctls rejected before driver entry)");
 }
 void *compat=dlopen(argv[1],RTLD_NOW|RTLD_LOCAL);
 if(!compat) {fprintf(stderr,"DRIL_LOAD_STOP: %s\n",dlerror());return 1;}
 const __DRIextension **(*extensions)(void)=dlsym(compat,"__driDriverGetExtensions_gma500");
 if(!extensions) {fprintf(stderr,"DRIL_ENTRY_STOP\n");dlclose(compat);return 1;}
 const __DRIextension **list=extensions();unsigned n=0;
 if(!list) {dlclose(compat);return 1;}
 for(;n<32&&list[n];n++) printf("DRIL_EXTENSION=%s:%d\n",list[n]->name,list[n]->version);
 if(!n||n==32) {dlclose(compat);return 1;}
 void *render=dlopen(argv[2],RTLD_NOW|RTLD_LOCAL);
 if(!render) {fprintf(stderr,"GALLIUM_LOAD_STOP: %s\n",dlerror());dlclose(compat);return 1;}
 __typeof__(&driCreateNewScreen3) create=dlsym(render,"driCreateNewScreen3");
 __typeof__(&driDestroyScreen) destroy=dlsym(render,"driDestroyScreen");
 if(!create||!destroy) {fprintf(stderr,"GALLIUM_ENTRY_STOP\n");dlclose(render);dlclose(compat);return 1;}
 const __DRIextension *loader[]={NULL};
 /* fd=-1 deliberately exercises actual frontend failure/cleanup. DRI3 cannot
  * fall back to a software screen. Never enumerate/open real DRM devices. */
 int before=descriptor_count();if(before<0) {dlclose(render);dlclose(compat);return 1;}
 for(unsigned i=0;i<100;i++) {
  const struct dri_config **configs=NULL;errno=0;
  struct dri_screen *screen=create(0,-1,loader,DRI_SCREEN_DRI3,&configs,false,false,NULL);
  if(screen||configs) {fprintf(stderr,"UNEXPECTED_SCREEN_SUCCESS\n");if(screen)destroy(screen);dlclose(render);dlclose(compat);return 1;}
 }
 if(descriptor_count()!=before) {fprintf(stderr,"DESCRIPTOR_LEAK_STOP\n");dlclose(render);dlclose(compat);return 1;}
 puts("PASS: real DRIL entry and Mesa frontend load; 100 invalid-fd screens rejected without fd leaks; NO GPU EXECUTION");
 puts("OPENGL_CONTEXT=BLOCKED: restricted shaders only; general shaders/textures, raster semantics, framebuffer preservation and truthful caps unsupported");
 dlclose(render);dlclose(compat);return 0;
}
