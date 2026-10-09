/* SPDX-License-Identifier: MIT */
/* Run the real Gallium application with link-time DRM syscall interposition.
 * The intentionally blank response is a fixture, NEVER GPU evidence. */
#define main draw_application_main
#include "gallium_first_draw.c"
#undef main
#include <assert.h>
#include <stdarg.h>
#include <sys/ioctl.h>
static unsigned draws;
int __real_open(const char *,int,...);
int __real_close(int);
int __wrap_open(const char *name,int flags,...)
{
 if(!strcmp(name,"MOCK-DRM-NO-HARDWARE")) return 321;
 if(flags&O_CREAT) { va_list a;va_start(a,flags);int mode=va_arg(a,int);va_end(a);return __real_open(name,flags,mode); }
 return __real_open(name,flags);
}
int __wrap_close(int fd) { return fd==321 ? 0 : __real_close(fd); }
int __wrap_ioctl(int fd,unsigned long op,...)
{
 va_list a;va_start(a,op);void *arg=va_arg(a,void *);va_end(a);assert(fd==321);
 if(op==DRM_IOCTL_VERSION) { struct drm_version *v=arg;memcpy(v->name,"gma500",6);v->name_len=6;return 0; }
 assert(op==DRM_IOCTL_REMESA_NATIVE_DRAW && ++draws==1);
 struct remesa_native_draw *r=arg;
 assert(r->vertex_count==3 && r->index_count==3 && r->argb==0xffff00ff);
 assert(r->positions[0][0]==0x41000000 && r->positions[1][0]==0x41c00000);
 assert(r->indices[0]==0 && r->indices[1]==1 && r->indices[2]==2);
 r->operation=1;r->phase=9;r->events=7;r->issued=r->published=r->status=1;
 /* Blank fixture tests actual readback propagation and mismatch classification. */
 memset(r->color,0,sizeof(r->color));return 0;
}
int main(void)
{
 char dir[]="/tmp/remesa-gallium-mock-XXXXXX";assert(mkdtemp(dir));
 char path[256],response[280];snprintf(path,sizeof(path),"%s/output",dir);
 snprintf(response,sizeof(response),"%s.native-response",path);
 char *args[]={"interposed-test","--execute-once","MOCK-DRM-NO-HARDWARE",path,NULL};
 assert(draw_application_main(4,args)==3 && draws==1);
 FILE *f=fopen(path,"rb");assert(f);unsigned char data[4096];
 assert(fread(data,1,sizeof(data),f)==sizeof(data) && fgetc(f)==EOF);fclose(f);
 for(size_t i=0;i<sizeof(data);++i) assert(data[i]==0);
 f=fopen(response,"rb");assert(f);assert(!fseek(f,0,SEEK_END) && ftell(f)==4240);fclose(f);
 assert(!unlink(path) && !unlink(response) && !rmdir(dir));
 puts("PASS: actual Gallium draw_vbo reached interposed ioctl once; blank fixture readback classified unexpected; NO GPU evidence");return 0;
}
