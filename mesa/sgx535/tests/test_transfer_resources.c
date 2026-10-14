/* SPDX-License-Identifier: MIT; real Gallium callbacks, no rasterization. */
#include "remesa_screen.h"
#include "remesa_device.h"
#include "remesa_resource_layout.h"
#include "remesa_present.h"
#include "pipe/p_screen.h"
#include "pipe/p_context.h"
#include "pipe/p_state.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef NDEBUG
#error "Transfer tests require enabled assertions"
#endif
struct fixture {unsigned allocations,frees,maps,unmaps,waits,fences,submits;int error;bool fail,partial;};
static int allocate(void *v,size_t bytes,void **out){struct fixture *f=v;if(f->fail)return ENOMEM;*out=calloc(1,bytes);if(!*out)return ENOMEM;++f->allocations;return f->partial?ENOMEM:0;}
static void drop(void *v,void *bo){++((struct fixture *)v)->frees;free(bo);}
static int map(void *v,void *bo,bool write,void **out){(void)write;++((struct fixture *)v)->maps;*out=bo;return 0;}
static void unmap(void *v,void *bo,bool write){(void)bo;(void)write;++((struct fixture *)v)->unmaps;}
static int submit(void *v,void *bo,const struct remesa_draw_packet *p,void **f){(void)bo;(void)p;(void)f;++((struct fixture *)v)->submits;return ENOTSUP;}
static int wait(void *v,void *f,uint64_t ns,bool visible){(void)f;(void)ns;assert(visible);++((struct fixture *)v)->waits;return ((struct fixture *)v)->error;}
static void destroy_fence(void *v,void *f){(void)f;++((struct fixture *)v)->fences;}
static struct pipe_resource *create(struct pipe_screen *s,unsigned w,unsigned h,enum pipe_format format,unsigned bind)
{struct pipe_resource t={0};t.target=PIPE_TEXTURE_2D;t.format=format;t.width0=w;t.height0=h;t.depth0=t.array_size=1;t.bind=bind;return s->resource_create(s,&t);}
static void release(struct pipe_resource *r){if(!--r->reference.count)r->screen->resource_destroy(r->screen,r);}
static void dimensions(void)
{
 const unsigned sizes[][2]={{32,32},{320,240},{640,480},{800,600},{1024,768},
  {1280,720},{1280,800},{1366,768},{1600,900},{1920,1080},{137,59},{1,1},{4096,1}};
 struct fixture f={0};const struct remesa_device_ops ops={.allocate=allocate,.destroy_bo=drop,.map=map,.unmap=unmap,.submit=submit,.wait=wait,.destroy_fence=destroy_fence};
 struct remesa_device dev={.ops=&ops,.priv=&f,.resource_only_backend=true,
  .resource_limits={.max_width=4096,.max_height=4096,.row_alignment=64,.max_resource_bytes=64*1024*1024}};
 struct pipe_screen *s=remesa_screen_create(&dev);assert(s);struct pipe_context *c=s->context_create(s,NULL,0);assert(c);
 for(unsigned i=0;i<sizeof(sizes)/sizeof(sizes[0]);i++)for(unsigned fmt=0;fmt<2;fmt++) {
  unsigned w=sizes[i][0],h=sizes[i][1],stride=(w*4+63)&~63u;
  struct pipe_resource *r=create(s,w,h,fmt?PIPE_FORMAT_R8G8B8A8_UNORM:PIPE_FORMAT_B8G8R8A8_UNORM,0);assert(r);
  struct pipe_transfer *tr=NULL;struct pipe_box all={.width=w,.height=h,.depth=1};
  unsigned char *p=c->texture_map(c,r,0,PIPE_MAP_WRITE,&all,&tr);assert(p&&tr->stride==stride&&tr->layer_stride==stride*h);
  /* Seed padding separately so partial uploads cannot hide row overrun. */
  memset(p,0xcd,(size_t)stride*h);c->texture_unmap(c,tr);
  unsigned char *src=malloc((size_t)(w*4+12)*h);assert(src);
  for(unsigned y=0;y<h;y++)for(unsigned x=0;x<w*4+12;x++)src[(size_t)y*(w*4+12)+x]=(unsigned char)(x+y*3);
  c->texture_subdata(c,r,0,0,&all,src,w*4+12,(uintptr_t)(w*4+12)*h);assert(!remesa_context_error(c));
  p=c->texture_map(c,r,0,PIPE_MAP_READ,&all,&tr);assert(p);
  for(unsigned y=0;y<h;y++) {
   assert(!memcmp(p+(size_t)y*stride,src+(size_t)y*(w*4+12),w*4));
   for(unsigned x=w*4;x<stride;x++)assert(p[(size_t)y*stride+x]==0xcd);
  }
  c->texture_unmap(c,tr);free(src);
  struct pipe_box corner={.x=w-1,.y=h-1,.width=1,.height=1,.depth=1};unsigned char pixel[4]={1,3,5,7};
  c->texture_subdata(c,r,0,0,&corner,pixel,4,4);assert(!remesa_context_error(c));
  p=c->texture_map(c,r,0,PIPE_MAP_READ,&corner,&tr);assert(p&&!memcmp(p,pixel,4));c->texture_unmap(c,tr);
  corner.width=2;unsigned before=f.maps;c->texture_subdata(c,r,0,0,&corner,pixel,8,8);assert(remesa_context_error(c)==EINVAL&&f.maps==before);
  release(r);
 }
 c->destroy(c);s->destroy(s);assert(f.allocations==f.frees&&f.maps==f.unmaps&&!f.submits);
 dev.resource_limits=(struct remesa_resource_limits){.max_width=2000,.max_height=1080,.row_alignment=64,.max_resource_bytes=4096000};
 s=remesa_screen_create(&dev);assert(s);
 struct pipe_resource *limited=create(s,1280,800,PIPE_FORMAT_B8G8R8A8_UNORM,0);assert(limited);release(limited);
 unsigned allocations=f.allocations;
 assert(!create(s,1920,1080,PIPE_FORMAT_B8G8R8A8_UNORM,0));
 assert(!create(s,2001,1,PIPE_FORMAT_B8G8R8A8_UNORM,0));
 assert(!create(s,1,1081,PIPE_FORMAT_B8G8R8A8_UNORM,0));assert(f.allocations==allocations);
 f.partial=true;assert(!create(s,2,2,PIPE_FORMAT_B8G8R8A8_UNORM,0));f.partial=false;assert(f.allocations==f.frees);s->destroy(s);
 dev.resource_limits.row_alignment=256;s=remesa_screen_create(&dev);assert(s);
 assert(!create(s,32,32,PIPE_FORMAT_B8G8R8A8_UNORM,PIPE_BIND_RENDER_TARGET));s->destroy(s);
 /* Layout rejection has no allocator side effects and never changes output. */
 struct remesa_resource_layout result={.stride=23,.bytes=29,.pages=31};
 struct remesa_resource_limits limits={.max_width=UINT32_MAX,.max_height=UINT32_MAX,.row_alignment=64,.max_resource_bytes=UINT64_MAX};
 assert(remesa_resource_layout(&limits,UINT32_MAX,1,4,&result)==EOVERFLOW);
 assert(result.stride==23&&result.bytes==29&&result.pages==31);
 assert(remesa_resource_layout(&limits,1,UINT32_MAX,4,&result)==EOVERFLOW);
 limits.row_alignment=3;assert(remesa_resource_layout(&limits,1,1,4,&result)==EINVAL);
 limits.row_alignment=64;limits.max_resource_bytes=63;assert(remesa_resource_layout(&limits,1,1,4,&result)==ENOMEM);
 limits.max_resource_bytes=64;assert(!remesa_resource_layout(&limits,1,1,4,&result));assert(result.stride==64&&result.bytes==64&&result.pages==1);
 limits.max_width=1;assert(remesa_resource_layout(&limits,2,1,4,&result)==EINVAL);
 limits.max_width=4096;limits.max_resource_bytes=UINT64_MAX;assert(!remesa_resource_layout(&limits,1280,800,4,&result));assert(result.bytes==4096000&&result.pages==1000);
 puts("PASS 26 RGBA/BGRA resolution cases through1920x1080 plus arbitrary sizes: padded rows, full/partial uploads, pixel integrity, checked limits; NO SGX");
}
int main(void)
{
 dimensions();
 struct fixture f={0};const struct remesa_device_ops ops={.allocate=allocate,.destroy_bo=drop,.map=map,.unmap=unmap,.submit=submit,.wait=wait,.destroy_fence=destroy_fence};
 struct remesa_device dev={.ops=&ops,.priv=&f,.resource_only_backend=true};struct pipe_screen *s=remesa_screen_create(&dev);assert(s);struct pipe_context *c=s->context_create(s,NULL,0);assert(c);
 for(unsigned format=0;format<2;format++) {
  struct pipe_resource *r=create(s,320,240,format?PIPE_FORMAT_R8G8B8A8_UNORM:PIPE_FORMAT_B8G8R8A8_UNORM,0);assert(r);
  struct pipe_box box={.x=317,.y=238,.width=3,.height=2,.depth=1};struct pipe_transfer *tr=NULL;
  unsigned char upload[28];memset(upload,0xa5,sizeof(upload));memset(upload,0x37,12);memset(upload+16,0x91,12);
  unsigned maps=f.maps;c->texture_subdata(c,r,0,0,&box,upload,11,0);assert(remesa_context_error(c)==EINVAL&&f.maps==maps);
  c->texture_subdata(c,r,0,0,&box,upload,16,27);assert(remesa_context_error(c)==EINVAL&&f.maps==maps);
  c->texture_subdata(c,r,0,PIPE_MAP_UNSYNCHRONIZED,&box,upload,16,28);assert(remesa_context_error(c)==EINVAL&&f.maps==maps);
  c->texture_subdata(c,r,0,0,&box,upload,16,28);assert(!remesa_context_error(c));
  unsigned char *p=c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr);assert(p&&tr&&tr->stride==1280&&tr->layer_stride==307200);c->texture_unmap(c,tr);
  box=(struct pipe_box){.width=320,.height=240,.depth=1};p=c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr);assert(p);
  for(unsigned y=0;y<240;y++)for(unsigned x=0;x<1280;x++)assert(p[y*1280+x]==(y>=238&&x>=1268?(y==238?0x37:0x91):0));
  c->texture_unmap(c,tr);
  unsigned char snapshot[240*1296];memset(snapshot,0xa5,sizeof(snapshot));
  if(format)assert(remesa_present_snapshot(c,r,snapshot,1296,sizeof(snapshot))==EINVAL);
  else {
   assert(!remesa_present_snapshot(c,r,snapshot,1296,sizeof(snapshot)));
   for(unsigned y=0;y<240;y++)for(unsigned x=0;x<1296;x++)assert(snapshot[y*1296+x]==(x>=1280?0xa5:y>=238&&x>=1268?(y==238?0x37:0x91):0));
  }
  box=(struct pipe_box){.x=320,.width=1,.height=1,.depth=1};assert(!c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr)&&!tr);
  box=(struct pipe_box){.y=239,.width=1,.height=2,.depth=1};assert(!c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr));
  struct pipe_surface view={.format=r->format};assert(!c->create_surface(c,r,&view));struct pipe_framebuffer_state fb={.width=320,.height=240,.nr_cbufs=1};fb.cbufs[0].texture=r;fb.cbufs[0].format=r->format;c->set_framebuffer_state(c,&fb);assert(remesa_context_error(c)==ENOTSUP);release(r);
 }
 assert(!create(s,0,240,PIPE_FORMAT_B8G8R8A8_UNORM,0));assert(!create(s,4097,1,PIPE_FORMAT_B8G8R8A8_UNORM,0));
 assert(!create(s,320,240,PIPE_FORMAT_B8G8R8A8_UNORM,PIPE_BIND_RENDER_TARGET));assert(!create(s,32,32,PIPE_FORMAT_R8G8B8A8_UNORM,PIPE_BIND_RENDER_TARGET));assert(!create(s,32,32,PIPE_FORMAT_B8G8R8A8_UNORM,PIPE_BIND_SAMPLER_VIEW));
 f.fail=true;assert(!create(s,2,2,PIPE_FORMAT_B8G8R8A8_UNORM,0));f.fail=false;
 struct pipe_resource templ={.target=PIPE_BUFFER,.format=PIPE_FORMAT_R8_UNORM,.width0=64,.height0=1,.depth0=1,.array_size=1,.bind=PIPE_BIND_VERTEX_BUFFER};
 struct pipe_resource *buffer=s->resource_create(s,&templ);assert(buffer);unsigned char input[16];memset(input,0x7b,sizeof(input));
 unsigned oldmaps=f.maps;c->buffer_subdata(c,buffer,0,60,16,input);assert(remesa_context_error(c)==EINVAL&&f.maps==oldmaps);
 c->buffer_subdata(c,buffer,0,64,0,NULL);assert(!remesa_context_error(c)&&f.maps==oldmaps);
 c->buffer_subdata(c,buffer,PIPE_MAP_UNSYNCHRONIZED,0,16,input);assert(remesa_context_error(c)==EINVAL&&f.maps==oldmaps);
 c->buffer_subdata(c,buffer,0,4,16,input);assert(!remesa_context_error(c));
 struct pipe_box bufferbox={.width=64,.height=1,.depth=1};struct pipe_transfer *buffertr=NULL;unsigned char *bp=c->buffer_map(c,buffer,0,PIPE_MAP_READ,&bufferbox,&buffertr);assert(bp);
 for(unsigned i=0;i<64;i++)assert(bp[i]==(i>=4&&i<20?0x7b:0));
 c->buffer_unmap(c,buffertr);release(buffer);
 /* Explicit simulated writer admission on resource-only backend. */
 struct pipe_resource *r=create(s,4,2,PIPE_FORMAT_B8G8R8A8_UNORM,0);assert(r);struct pipe_fence_handle *fence=NULL;assert(!remesa_resource_attach_writer(c,r,&f,&fence));
 struct pipe_box box={.width=4,.height=2,.depth=1};struct pipe_transfer *tr=NULL;unsigned maps=f.maps;f.error=EAGAIN;assert(!c->texture_map(c,r,0,PIPE_MAP_READ|PIPE_MAP_DONTBLOCK,&box,&tr));assert(f.maps==maps);
 f.error=EINTR;assert(!c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr));assert(f.maps==maps);
 unsigned char zero[32]={0};c->texture_subdata(c,r,0,PIPE_MAP_DONTBLOCK,&box,zero,16,32);assert(remesa_context_error(c)==EINTR&&f.maps==maps);
 f.error=0;void *p=c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr);assert(p);release(r);assert(f.allocations==f.frees+1);c->texture_unmap(c,tr);s->fence_reference(s,&fence,NULL);assert(f.allocations==f.frees&&f.fences==1);
 r=create(s,4,2,PIPE_FORMAT_B8G8R8A8_UNORM,0);assert(r);assert(!remesa_resource_attach_writer(c,r,&f,&fence));f.error=EIO;maps=f.maps;assert(!c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr));f.error=0;assert(!c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr));assert(f.maps==maps);
 release(r);s->fence_reference(s,&fence,NULL);c->destroy(c);s->destroy(s);assert(f.allocations==f.frees&&f.maps==f.unmaps&&f.submits==0);
 puts("PASS Gallium transfer-only RGBA/BGRA320x240: pitch, bounds, fences, lifetime; NO SGX/sampling/render claim");return 0;
}
