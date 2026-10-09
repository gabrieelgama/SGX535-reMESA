/* SPDX-License-Identifier: MIT */
/* Synchronous, single-attempt Linux transport. Host staging is never a
 * rasterizer: target bytes become readable only from a validated GPU response.
 * Kernel ownership/quarantine protects actual SGX backing on uncertain failure.
 */
#include "remesa_linux.h"
#include "remesa/native_uapi.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

_Static_assert(sizeof(struct remesa_native_draw)==4240,"native/i386 UAPI");
struct linux_bo { size_t size; unsigned char *bytes; bool readable, busy, failed; };
struct linux_fence { int error; bool published; };
struct linux_device {
 int fd; bool attempted;
 struct remesa_linux_receipt receipt;
 struct remesa_native_draw response;
};
int remesa_linux_request_build(const struct remesa_draw_packet *p,struct remesa_native_draw *r)
{
 struct remesa_native_draw v={0};
 if(!r || !p) return EINVAL;
 if(p->vertex_count>REMESA_NATIVE_MAX_VERTICES || p->index_count>REMESA_NATIVE_MAX_INDICES) return ENOTSUP;
 if(remesa_draw_packet_validate(p)!=REMESA_PACKET_OK) return EINVAL;
 v.version=REMESA_NATIVE_ABI_VERSION;v.width=p->width;v.height=p->height;
 v.vertex_count=p->vertex_count;v.index_count=p->index_count;v.argb=p->argb;
 memcpy(v.positions,p->positions,p->vertex_count*16);memcpy(v.indices,p->indices,p->index_count*2);
 *r=v;return 0;
}
static int allocate(void *priv,size_t size,void **out)
{
 struct linux_bo *b;
 (void)priv;if(!out) return EINVAL;*out=NULL;
 if(!size || size>1024*1024) return EINVAL;
 b=calloc(1,sizeof(*b));if(!b) return ENOMEM;
 b->bytes=calloc(1,size);if(!b->bytes) { free(b);return ENOMEM; }
 b->size=size;*out=b;return 0;
}
static void destroy_bo(void *priv,void *bo)
{ struct linux_bo *b=bo;(void)priv;if(b) { free(b->bytes);free(b); } }
static int map(void *priv,void *bo,bool write,void **out)
{
 struct linux_bo *b=bo;(void)priv;if(!b || !out) return EINVAL;*out=NULL;
 if(b->failed) return EIO;
 if(b->busy || (!write && !b->readable)) return EAGAIN;
 *out=b->bytes;return 0;
}
static void unmap(void *priv,void *bo,bool write)
{ struct linux_bo *b=bo;(void)priv;if(b && write) b->readable=true; }
static int submit(void *priv,void *bo,const struct remesa_draw_packet *p,void **out)
{
 struct linux_device *d=priv;struct linux_bo *b=bo;
 struct remesa_native_draw request,original;struct linux_fence *f;int error;
 if(!d || !b || !out) return EINVAL;
 *out=NULL;
 if(d->attempted) return EBUSY;
 if(b->size!=REMESA_NATIVE_COLOR_BYTES) return EINVAL;
 error=remesa_linux_request_build(p,&request);if(error) return error;
 f=calloc(1,sizeof(*f));if(!f) return ENOMEM;
 original=request;b->readable=false;b->busy=true;
 /* Consume before syscall. EINTR, EFAULT and all other errors never retry. */
 d->attempted=true;d->receipt.attempted=1;
 error=ioctl(d->fd,DRM_IOCTL_REMESA_NATIVE_DRAW,&request);
 d->response=request;d->receipt.ioctl_errno=error<0 ? errno : 0;
 d->receipt.operation=request.operation;d->receipt.operation_errno=request.operation_errno;
 d->receipt.phase=request.phase;d->receipt.events=request.events;
 d->receipt.issued=request.issued;d->receipt.cpu_published=request.published;
 if(error<0) f->error=d->receipt.ioctl_errno;
 else if(error!=0) f->error=EPROTO;
 else if(memcmp(&request,&original,offsetof(struct remesa_native_draw,operation)) ||
         request.operation!=1 || request.operation_errno ||
         request.status!=REMESA_NATIVE_STATUS_COMPLETED || request.phase!=9 ||
         request.events!=7 || request.issued!=1 || request.published!=1)
  f->error=request.operation_errno ? EIO : EPROTO;
 else { memcpy(b->bytes,request.color,sizeof(request.color));b->readable=true;f->published=true; }
 b->failed=f->error!=0;b->busy=false;*out=f;return f->error;
}
static int wait(void *priv,void *fence,uint64_t ns,bool visible)
{ struct linux_fence *f=fence;(void)priv;(void)ns;if(!f) return EINVAL;return f->error ? f->error : visible && !f->published ? EAGAIN : 0; }
static void destroy_fence(void *priv,void *fence) { (void)priv;free(fence); }
static const struct remesa_device_ops ops={allocate,destroy_bo,map,unmap,submit,wait,destroy_fence};
static int admit_fd(struct remesa_device *device,int fd)
{
 struct linux_device *d;char name[32]={0};struct drm_version v={0};int error;
 v.name_len=sizeof(name)-1;v.name=name;
 int result=ioctl(fd,DRM_IOCTL_VERSION,&v);
 if(result<0) error=errno;
 else if(result) error=EPROTO;
 else error=v.name_len==strlen("gma500") && !memcmp(name,"gma500",v.name_len) ? 0 : ENODEV;
 if(error) { close(fd);return error; }
 d=calloc(1,sizeof(*d));if(!d) { close(fd);return ENOMEM; }
 d->fd=fd;device->ops=&ops;device->priv=d;device->qualified_hardware_backend=true;return 0;
}
static bool empty_device(const struct remesa_device *d)
{ return !d->ops && !d->priv && !d->offline_test_backend && !d->qualified_hardware_backend && !d->resource_only_backend && d->framebuffer_mode==REMESA_FRAMEBUFFER_REPLACE; }
int remesa_linux_open(struct remesa_device *device,const char *node)
{
 int fd;
 if(!device || !node) return EINVAL;
 if(!empty_device(device)) return EBUSY;
 fd=open(node,O_RDWR|O_CLOEXEC);if(fd<0) return errno;
 return admit_fd(device,fd);
}
int remesa_linux_open_fd(struct remesa_device *device,int borrowed_fd)
{
 int fd,flags;
 if(!device || borrowed_fd<0) return EINVAL;
 if(!empty_device(device)) return EBUSY;
 flags=fcntl(borrowed_fd,F_GETFL);
 if(flags<0) return errno;
 if((flags&O_ACCMODE)!=O_RDWR) return EACCES;
 fd=fcntl(borrowed_fd,F_DUPFD_CLOEXEC,0);if(fd<0) return errno;
 return admit_fd(device,fd);
}
void remesa_linux_close(struct remesa_device *device)
{ if(device && device->ops==&ops) { struct linux_device *d=device->priv;if(d) { close(d->fd);free(d); }memset(device,0,sizeof(*device)); } }
int remesa_linux_last_receipt(const struct remesa_device *device,struct remesa_linux_receipt *r)
{ if(!device || device->ops!=&ops || !device->priv || !r) return EINVAL;*r=((struct linux_device *)device->priv)->receipt;return 0; }
int remesa_linux_response(const struct remesa_device *device,struct remesa_native_draw *r)
{ struct linux_device *d;if(!device || device->ops!=&ops || !device->priv || !r) return EINVAL;d=device->priv;if(!d->attempted) return EAGAIN;*r=d->response;return 0; }
