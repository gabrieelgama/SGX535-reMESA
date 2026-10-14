/* SPDX-License-Identifier: MIT */
/* Real fd ownership, interposed DRM replies. No GPU or device node accessed. */
#include "remesa_loader.h"
#include "remesa_linux.h"
#include "remesa_screen.h"
#include "remesa_draw.h"
#include "remesa/native_uapi.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static int last_fd=-1,bad_name;static unsigned draws,versions;
int __wrap_ioctl(int fd,unsigned long op,...)
{
 va_list ap;va_start(ap,op);void *arg=va_arg(ap,void *);va_end(ap);
 assert(fcntl(fd,F_GETFD)&FD_CLOEXEC);last_fd=fd;
 if(op==DRM_IOCTL_VERSION) {
  struct drm_version *v=arg;++versions;
  memcpy(v->name,bad_name==1 ? "wrong!" : "gma500",6);v->name_len=bad_name==3 ? 32 : 6;return bad_name==2 ? 1 : 0;
 }
 assert(op==DRM_IOCTL_REMESA_NATIVE_DRAW);++draws;
 /* Simulated completion only; intentionally no raster/reference success. */
 struct remesa_native_draw *r=arg;r->operation=1;r->phase=9;r->events=7;
 r->issued=r->published=r->status=1;memset(r->color,0x5a,sizeof(r->color));return 0;
}
static struct pipe_resource *make(struct pipe_screen *s,bool color)
{
 struct pipe_resource t={0};t.target=color ? PIPE_TEXTURE_2D : PIPE_BUFFER;
 t.format=color ? PIPE_FORMAT_B8G8R8A8_UNORM : PIPE_FORMAT_R8_UNORM;
 t.width0=color ? 32 : 48;t.height0=color ? 32 : 1;t.depth0=t.array_size=1;
 t.bind=color ? PIPE_BIND_RENDER_TARGET : PIPE_BIND_VERTEX_BUFFER;
 return s->resource_create(s,&t);
}
static void drop(struct pipe_resource *r)
{ if(!--r->reference.count) r->screen->resource_destroy(r->screen,r); }
int main(void)
{
 char path[]="/tmp/remesa-fd-XXXXXX";int fd=mkstemp(path);assert(fd>=0);unlink(path);
 int e;assert(!remesa_loader_diagnostic_screen(-1,&e)&&e==EINVAL);
 assert(!remesa_loader_diagnostic_screen(fd,NULL)&&!versions);
 struct remesa_device dev={0};assert(!remesa_linux_open_fd(&dev,fd));int owned=last_fd;
 assert(owned!=fd&&fcntl(fd,F_GETFD)>=0);
 assert(remesa_linux_open_fd(&dev,fd)==EBUSY);
 assert(remesa_linux_open(&dev,"MUST-NOT-OPEN")==EBUSY);
 remesa_linux_close(&dev);assert(fcntl(owned,F_GETFD)==-1&&errno==EBADF);
 bad_name=1;assert(!remesa_loader_diagnostic_screen(fd,&e)&&e==ENODEV);
 assert(fcntl(last_fd,F_GETFD)==-1&&errno==EBADF);assert(fcntl(fd,F_GETFD)>=0);bad_name=0;
 bad_name=2;assert(!remesa_loader_diagnostic_screen(fd,&e)&&e==EPROTO);
 assert(fcntl(last_fd,F_GETFD)==-1&&errno==EBADF);
 bad_name=3;assert(!remesa_loader_diagnostic_screen(fd,&e)&&e==ENODEV);
 assert(fcntl(last_fd,F_GETFD)==-1&&errno==EBADF);bad_name=0;
 int ro=open("/dev/null",O_RDONLY);assert(ro>=0);
 assert(!remesa_loader_diagnostic_screen(ro,&e)&&e==EACCES);close(ro);
 struct pipe_screen *s=remesa_loader_diagnostic_screen(fd,&e);assert(s&&!e&&!draws);owned=last_fd;
 struct pipe_context *c=s->context_create(s,NULL,0);assert(c);
 struct pipe_resource *v=make(s,false),*t=make(s,true);assert(v&&t);
 struct pipe_box box={.width=48,.height=1,.depth=1};struct pipe_transfer *tr;
 float verts[3][4]={{8,8,.5f,1},{24,8,.5f,1},{8,24,.5f,1}};
 void *ptr=c->buffer_map(c,v,0,PIPE_MAP_WRITE,&box,&tr);assert(ptr);memcpy(ptr,verts,48);c->buffer_unmap(c,tr);
 struct pipe_vertex_buffer vb={0};vb.buffer.resource=v;++v->reference.count;c->set_vertex_buffers(c,1,&vb);
 struct pipe_vertex_element ve={.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT,.src_stride=16};
 void *state=c->create_vertex_elements_state(c,1,&ve);assert(state);c->bind_vertex_elements_state(c,state);
 struct pipe_framebuffer_state fb={0};fb.width=fb.height=32;fb.nr_cbufs=1;fb.cbufs[0].texture=t;fb.cbufs[0].format=t->format;
 c->set_framebuffer_state(c,&fb);assert(!remesa_bind_diagnostic_pipeline(c,0xffff00ff));
 struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.instance_count=1};struct pipe_draw_start_count_bias d={.count=3};
 c->draw_vbo(c,&info,0,NULL,&d,1);assert(!remesa_context_error(c)&&draws==1);
 struct pipe_fence_handle *f=NULL;c->flush(c,&f,0);assert(f);
 box.width=box.height=32;ptr=c->texture_map(c,t,0,PIPE_MAP_READ,&box,&tr);assert(ptr);
 assert(((unsigned char *)ptr)[0]==0x5a);c->texture_unmap(c,tr);
 s->destroy(s);assert(!s->context_create(s,NULL,0)&&!make(s,true));
 c->draw_vbo(c,&info,0,NULL,&d,1);assert(remesa_context_error(c)==ESHUTDOWN&&draws==1);
 /* Existing children retain cleanup/fence ownership; closing rejects new map
  * leases. Readback must occur before shutdown, as in the transfer contract. */
 ptr=c->texture_map(c,t,0,PIPE_MAP_READ,&box,&tr);assert(!ptr&&!tr);
 c->bind_vertex_elements_state(c,NULL);c->delete_vertex_elements_state(c,state);
 c->destroy(c);drop(v);drop(t);assert(fcntl(owned,F_GETFD)>=0);
 assert(s->fence_finish(s,NULL,f,0));s->fence_reference(s,&f,NULL);
 assert(fcntl(owned,F_GETFD)==-1&&errno==EBADF);assert(fcntl(fd,F_GETFD)>=0);close(fd);
 puts("PASS: fd admission/rollback, duplicate ownership, resource/context/fence deferred teardown, closed-screen draw rejection; simulated completion only");
}
