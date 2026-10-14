/* SPDX-License-Identifier: MIT
 * CPU clear and mocked fence fixture. No SGX or generated raster output. */
#include "remesa_device.h"
#include "remesa_screen.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct fixture {unsigned alloc,free,maps,unmaps,waits,submits;int waiting,map_error;} f;
static int alloc(void *p,size_t n,void **o){(void)p;*o=calloc(1,n);f.alloc++;return *o?0:ENOMEM;}
static void drop(void *p,void *o){(void)p;free(o);f.free++;}
static int map_bo(void *p,void *o,bool w,void **out){(void)p;assert(w||o);f.maps++;if(f.map_error)return f.map_error;*out=o;return 0;}
static void unmap_bo(void *p,void *o,bool w){(void)p;(void)o;(void)w;f.unmaps++;}
static int submit(void *p,void *o,const struct remesa_draw_packet *d,void **out){(void)p;(void)o;(void)d;(void)out;f.submits++;return ENOTSUP;}
static int wait(void *p,void *o,uint64_t t,bool visible){(void)p;(void)o;(void)t;assert(visible);f.waits++;return f.waiting;}
static void destroy_fence(void *p,void *o){(void)p;(void)o;}
static struct pipe_resource *target(struct pipe_screen *s){struct pipe_resource r={0};r.target=PIPE_TEXTURE_2D;r.format=PIPE_FORMAT_B8G8R8A8_UNORM;r.width0=r.height0=32;r.array_size=r.depth0=1;r.bind=PIPE_BIND_RENDER_TARGET;return s->resource_create(s,&r);}
static void release(struct pipe_resource *r){if(!--r->reference.count)r->screen->resource_destroy(r->screen,r);}
int main(void){
 const struct remesa_device_ops ops={.allocate=alloc,.destroy_bo=drop,.map=map_bo,.unmap=unmap_bo,.submit=submit,.wait=wait,.destroy_fence=destroy_fence};
 struct remesa_device d={.ops=&ops,.resource_only_backend=true};struct pipe_screen *s=remesa_screen_create(&d);assert(s);
 struct pipe_context *c=s->context_create(s,NULL,0);struct pipe_resource *r=target(s);assert(c&&r);
 struct pipe_framebuffer_state fb={0};fb.width=fb.height=32;fb.nr_cbufs=1;fb.cbufs[0].texture=r;fb.cbufs[0].format=r->format;c->set_framebuffer_state(c,&fb);
 union pipe_color_union color={.f={1,0,.5f,1}};
 c->clear(c,PIPE_CLEAR_COLOR0,NULL,&color,0,0);assert(!remesa_context_error(c));assert(remesa_context_cpu_clear_count(c)==1);
 struct pipe_box box={.width=32,.height=32,.depth=1};struct pipe_transfer *tr=NULL;unsigned char *v=c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr);assert(v);
 for(unsigned i=0;i<1024;i++){assert(v[i*4]==128&&v[i*4+1]==0&&v[i*4+2]==255&&v[i*4+3]==255);}c->texture_unmap(c,tr);
 struct pipe_scissor_state rect={.minx=3,.miny=4,.maxx=5,.maxy=6};color.f[0]=0;color.f[1]=1;color.f[2]=0;color.f[3]=0;
 c->clear(c,PIPE_CLEAR_COLOR0,&rect,&color,0,0);assert(!remesa_context_error(c));
 v=c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr);assert(v);for(unsigned y=0;y<32;y++)for(unsigned x=0;x<32;x++){unsigned char *q=v+(y*32+x)*4;bool inside=x>=3&&x<5&&y>=4&&y<6;assert(q[0]==(inside?0:128)&&q[1]==(inside?255:0)&&q[2]==(inside?0:255)&&q[3]==(inside?0:255));}c->texture_unmap(c,tr);
 unsigned maps=f.maps;unsigned long long clears=remesa_context_cpu_clear_count(c);
 color.f[0]=NAN;c->clear(c,PIPE_CLEAR_COLOR0,NULL,&color,0,0);assert(remesa_context_error(c)==EINVAL&&f.maps==maps);color.f[0]=0;
 rect.maxx=33;c->clear(c,PIPE_CLEAR_COLOR0,&rect,&color,0,0);assert(remesa_context_error(c)==EINVAL&&f.maps==maps);
 c->clear(c,PIPE_CLEAR_DEPTH,NULL,&color,0,0);assert(remesa_context_error(c)==ENOTSUP&&f.maps==maps);
 c->clear(c,0,NULL,NULL,0,0);assert(!remesa_context_error(c)&&f.maps==maps);
 v=c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr);assert(v);maps=f.maps;c->clear(c,PIPE_CLEAR_COLOR0,NULL,&color,0,0);assert(remesa_context_error(c)==EBUSY&&f.maps==maps);c->texture_unmap(c,tr);
 struct pipe_fence_handle *fence=NULL;assert(!remesa_resource_attach_writer(c,r,&f,&fence));f.waiting=EAGAIN;maps=f.maps;
 c->clear(c,PIPE_CLEAR_COLOR0,NULL,&color,0,0);assert(remesa_context_error(c)==EAGAIN&&f.maps==maps);
 f.waiting=0;c->clear(c,PIPE_CLEAR_COLOR0,NULL,&color,0,0);assert(!remesa_context_error(c)&&f.waits>=2);s->fence_reference(s,&fence,NULL);
 f.map_error=EIO;c->clear(c,PIPE_CLEAR_COLOR0,NULL,&color,0,0);assert(remesa_context_error(c)==EIO);f.map_error=0;
 assert(remesa_context_cpu_clear_count(c)==clears+1&&f.submits==0);
 struct pipe_surface template={.format=r->format};struct pipe_surface *surface=c->create_surface(c,r,&template);assert(surface&&c->clear_render_target);
 c->clear_render_target(c,surface,&color,31,31,1,1,false);assert(!remesa_context_error(c));
 maps=f.maps;c->clear_render_target(c,surface,&color,0,0,1,1,true);assert(remesa_context_error(c)==ENOTSUP&&f.maps==maps);
 c->surface_destroy(c,surface);c->destroy(c);release(r);s->destroy(s);assert(f.alloc==f.free&&f.submits==0);
 puts("PASS CPU-only BGRA clear, bounds, fence/map/error/lifetime; zero GPU submissions");
}
