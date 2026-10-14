/* SPDX-License-Identifier: MIT */
#include "remesa_texture_binding.h"
#include "remesa_resource_ref.h"
#include "pipe/p_context.h"
#include "util/u_inlines.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef NDEBUG
#error Assertions must remain enabled
#endif
struct memory { unsigned calls, live, fail;
                struct pipe_resource *injected_resource;
                int injected_count; };
struct fixture { struct pipe_screen screen; unsigned resources, destroys,queries;
                 int query_error; };
struct resource { struct pipe_resource base; struct remesa_texture_storage storage;
                  struct remesa_texture_storage mips[REMESA_TEXTURE_STORAGE_LEVELS]; };
static void *allocate(void *p,size_t bytes)
{
   struct memory *m=p;
   if (++m->calls==m->fail) return NULL;
   void *v=malloc(bytes); if (v) {
      ++m->live;
      /* Deterministic competing-reference fixture, not GPU evidence. */
      if (m->injected_resource)
         __atomic_store_n(&m->injected_resource->reference.count,
                          m->injected_count,__ATOMIC_RELAXED);
   }
   return v;
}
static void release(void *p,void *v)
{ struct memory *m=p; assert(m->live&&v); --m->live; free(v); }
static void resource_destroy(struct pipe_screen *screen,struct pipe_resource *r)
{
   struct fixture *f=(struct fixture *)screen;
   assert(f->resources && r->reference.count==0);
   --f->resources; ++f->destroys; free(r);
}
static struct pipe_resource *resource_create(struct fixture *f,unsigned w,unsigned h,
                                            enum pipe_format format)
{
   struct resource *r=calloc(1,sizeof(*r)); assert(r);
   r->base.screen=&f->screen; r->base.reference.count=1;
   r->base.target=PIPE_TEXTURE_2D; r->base.format=format;
   r->base.width0=w; r->base.height0=h; r->base.depth0=1; r->base.array_size=1;
   r->storage=(struct remesa_texture_storage){.resource_cookie=123,
       .stride=(w*4u+63u)&~63u,.width=w,.height=h,.levels=1};
   r->storage.bytes=(size_t)r->storage.stride*h; ++f->resources; return &r->base;
}
static int query(void *p,struct pipe_resource *r,unsigned level,struct remesa_texture_storage *out)
{
   struct fixture *f=p; ++f->queries;
   if (f->query_error) return f->query_error;
   if(level>r->last_level)return EINVAL;
   *out=level?((struct resource *)r)->mips[level]:((struct resource *)r)->storage; return 0;
}
static struct remesa_texture_binding_bank *bank(struct fixture *f,struct memory *m)
{ return remesa_texture_binding_create(&f->screen,query,f,allocate,release,m); }
static struct pipe_sampler_view descriptor(enum pipe_format format)
{
   struct pipe_sampler_view v={0}; v.target=PIPE_TEXTURE_2D; v.format=format;
   v.swizzle_r=PIPE_SWIZZLE_X; v.swizzle_g=PIPE_SWIZZLE_Y;
   v.swizzle_b=PIPE_SWIZZLE_Z; v.swizzle_a=PIPE_SWIZZLE_W; return v;
}
int main(void)
{
   unsigned cases=0;
   struct fixture f={0},foreign={0}; struct memory m={0},other_memory={0};
   f.screen.resource_destroy=resource_destroy;
   foreign.screen.resource_destroy=resource_destroy;
   struct remesa_texture_binding_bank *b=bank(&f,&m),*other=bank(&f,&other_memory);
   assert(b&&other&&m.live==1); assert(!remesa_texture_draw_admit(b)); ++cases;
   struct pipe_resource *r=resource_create(&f,33,7,PIPE_FORMAT_R8G8B8A8_UNORM);
   struct pipe_sampler_view desc=descriptor(r->format),*v=NULL;
   assert(!remesa_texture_view_create(b,r,&desc,&v)&&v);
   assert(r->reference.count==2&&v->reference.count==1); ++cases;
   struct remesa_texture_storage storage;
   assert(!remesa_texture_view_storage(b,v,&storage));
   assert(storage.stride==192&&storage.bytes==1344&&storage.resource_cookie==123); ++cases;
   /* Template mutations do not change the driver-owned descriptor. */
   desc.swizzle_a=PIPE_SWIZZLE_0; assert(v->swizzle_a==PIPE_SWIZZLE_W); ++cases;
   struct pipe_sampler_view *views[]={v};
   assert(!remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,1,1,0,views));
   assert(v->reference.count==2&&r->reference.count==2);
   assert(remesa_texture_draw_admit(b)==ENOTSUP); ++cases;
   /* Self-rebinding retains before releasing the prior binding. */
   assert(!remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,1,1,0,views));
   assert(v->reference.count==2); ++cases;
   struct pipe_sampler_view *alias=NULL;
   pipe_sampler_view_reference(&alias,v); assert(v->reference.count==3);
   pipe_sampler_view_release_ptr(&alias); assert(!alias&&v->reference.count==2); ++cases;
   assert(remesa_texture_view_destroy(b,v)==EBUSY); ++cases;
   assert(remesa_texture_views_bind(other,PIPE_SHADER_FRAGMENT,0,1,0,views)==ESTALE);
   assert(remesa_texture_draw_admit(other)==ESTALE);
   assert(!remesa_texture_views_bind(other,PIPE_SHADER_FRAGMENT,0,1,0,NULL)); ++cases;
   uint64_t cookie=((struct resource *)r)->storage.resource_cookie;
   ++((struct resource *)r)->storage.resource_cookie;
   assert(remesa_texture_draw_admit(b)==ESTALE); ++cases;
   ((struct resource *)r)->storage.resource_cookie=cookie;
   f.query_error=EIO; assert(remesa_texture_draw_admit(b)==EIO); ++cases;
   f.query_error=-1; assert(remesa_texture_draw_admit(b)==EPROTO); ++cases;
   f.query_error=0;
   struct pipe_resource *backing=v->texture;
   v->texture=(void *)(uintptr_t)1;
   assert(remesa_texture_draw_admit(b)==ESTALE); v->texture=backing; ++cases;
   v->swizzle_r=PIPE_SWIZZLE_1;
   assert(remesa_texture_draw_admit(b)==ESTALE); v->swizzle_r=PIPE_SWIZZLE_X; ++cases;
   v->u.tex.last_level=1;
   assert(remesa_texture_draw_admit(b)==ESTALE); v->u.tex.last_level=0; ++cases;
   ++r->height0; assert(remesa_texture_draw_admit(b)==EINVAL); --r->height0; ++cases;
   pipe_sampler_view_release(v); assert(v->reference.count==1);
   assert(!remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,1,0,1,NULL));
   assert(r->reference.count==1&&v->reference.count==0);
   assert(remesa_texture_view_destroy(b,v)==ESTALE);
   assert(remesa_texture_view_storage(b,v,&storage)==ESTALE); ++cases;
   views[0]=v;
   assert(remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,1,1,0,views)==ESTALE);
   assert(remesa_texture_draw_admit(b)==ESTALE);
   assert(!remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,1,1,0,NULL)); ++cases;
   views[0]=(void *)(uintptr_t)1;
   assert(remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,1,1,0,views)==ESTALE);
   assert(!remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,1,1,0,NULL)); ++cases;
   desc=descriptor(r->format); desc.u.tex.first_layer=1; v=(void *)(uintptr_t)1;
   assert(remesa_texture_view_create(b,r,&desc,&v)==ENOTSUP&&!v); ++cases;
   desc=descriptor(r->format); desc.u.tex.last_level=1;
   assert(remesa_texture_view_create(b,r,&desc,&v)==EINVAL&&!v); ++cases;
   desc=descriptor(r->format); desc.swizzle_r=7;
   assert(remesa_texture_view_create(b,r,&desc,&v)==ENOTSUP&&!v); ++cases;
   desc=descriptor(PIPE_FORMAT_B8G8R8A8_UNORM);
   assert(remesa_texture_view_create(b,r,&desc,&v)==ENOTSUP&&!v); ++cases;
   desc=descriptor(r->format); desc.is_tex2d_from_buf=true;
   assert(remesa_texture_view_create(b,r,&desc,&v)==ENOTSUP&&!v); ++cases;
   desc=descriptor(r->format); r->array_size=2;
   assert(remesa_texture_view_create(b,r,&desc,&v)==ENOTSUP&&!v); r->array_size=1; ++cases;
   r->nr_samples=4;
   assert(remesa_texture_view_create(b,r,&desc,&v)==ENOTSUP&&!v); r->nr_samples=0; ++cases;
   struct pipe_resource *foreign_resource=resource_create(&foreign,2,2,r->format);
   assert(remesa_texture_view_create(b,foreign_resource,&desc,&v)==EINVAL&&!v);
   remesa_resource_reference(&foreign_resource,NULL); assert(!foreign.resources); ++cases;
   struct resource *raw=(struct resource *)r;
   struct remesa_texture_storage saved=raw->storage;
   raw->storage.resource_cookie=0;
   assert(remesa_texture_view_create(b,r,&desc,&v)==EINVAL&&!v); ++cases;
   raw->storage=saved; raw->storage.stride=128;
   assert(remesa_texture_view_create(b,r,&desc,&v)==EINVAL&&!v); ++cases;
   raw->storage=saved; raw->storage.bytes--;
   assert(remesa_texture_view_create(b,r,&desc,&v)==EINVAL&&!v); ++cases;
   raw->storage=saved; m.fail=m.calls+1;
   assert(remesa_texture_view_create(b,r,&desc,&v)==ENOMEM&&!v);
   assert(r->reference.count==1); m.fail=0; ++cases;
   unsigned allocations=m.calls,live=m.live;
   r->reference.count=INT_MAX; v=(void *)(uintptr_t)1;
   assert(remesa_texture_view_create(b,r,&desc,&v)==EOVERFLOW&&!v);
   assert(r->reference.count==INT_MAX&&m.calls==allocations&&m.live==live);
   r->reference.count=1; ++cases;
   m.injected_resource=r; m.injected_count=INT_MAX;
   assert(remesa_texture_view_create(b,r,&desc,&v)==EOVERFLOW&&!v);
   assert(r->reference.count==INT_MAX&&m.calls==allocations+1&&m.live==live);
   r->reference.count=1; ++cases;
   m.injected_count=0;
   assert(remesa_texture_view_create(b,r,&desc,&v)==ESTALE&&!v);
   assert(!r->reference.count&&m.calls==allocations+2&&m.live==live);
   r->reference.count=1; m.injected_resource=NULL; ++cases;
   assert(!remesa_texture_view_create(b,r,&desc,&v));
   v->reference.count=INT_MAX; views[0]=v;
   assert(remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,0,1,0,views)==EOVERFLOW);
   v->reference.count=1; pipe_sampler_view_release(v);
   assert(!remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,0,1,0,NULL)); ++cases;
   assert(remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,UINT_MAX,1,0,NULL)==EINVAL);
   assert(remesa_texture_draw_admit(b)==EINVAL); ++cases;
   remesa_texture_binding_close(b); remesa_texture_binding_close(other);
   assert(!m.live&&!other_memory.live);
   remesa_resource_reference(&r,NULL); assert(!f.resources); ++cases;

   /* Actual Mesa release after rendering-context teardown reaches only the
    * independent destruction proxy. No stale query_user is dereferenced. */
   m=(struct memory){0}; b=bank(&f,&m); assert(b);
   r=resource_create(&f,1280,800,PIPE_FORMAT_B8G8R8A8_UNORM);
   desc=descriptor(r->format);
   assert(!remesa_texture_view_create(b,r,&desc,&v)); views[0]=v;
   assert(!remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,0,1,0,views));
   remesa_texture_binding_close(b); assert(v->reference.count==1&&m.live==2);
   remesa_resource_reference(&r,NULL); assert(f.resources==1);
   pipe_sampler_view_release_ptr(&v);
   assert(!v&&!m.live&&!f.resources); ++cases;
   /* All listed resolutions exercise checked padded host layout admission;
    * none is an assertion of GPU sampling or SGX raster dimensions. */
   const unsigned dimensions[][2]={{32,32},{320,240},{640,480},{800,600},
       {1024,768},{1280,720},{1280,800},{1366,768},{1600,900},{1920,1080}};
   b=bank(&f,&m);
   for (unsigned i=0;i<sizeof(dimensions)/sizeof(dimensions[0]);i++) {
      r=resource_create(&f,dimensions[i][0],dimensions[i][1],
                         i&1?PIPE_FORMAT_R8G8B8A8_UNORM:PIPE_FORMAT_B8G8R8A8_UNORM);
      desc=descriptor(r->format); desc.swizzle_a=PIPE_SWIZZLE_1;
      assert(!remesa_texture_view_create(b,r,&desc,&v));
      assert(!remesa_texture_view_storage(b,v,&storage));
      assert((uint64_t)storage.stride*r->height0<=storage.bytes);
      pipe_sampler_view_reference(&v,NULL); remesa_resource_reference(&r,NULL);
      assert(!f.resources); ++cases;
   }
   /* Immutable mip layout, interval selection and backing identity are all
    * revalidated. These descriptors remain draw-blocking host objects. */
   r=resource_create(&f,37,13,PIPE_FORMAT_R8G8B8A8_UNORM);r->last_level=5;
   raw=(struct resource *)r;size_t offset=0;
   for(unsigned level=0;level<=5;level++) {
      unsigned width=37u>>level,height=13u>>level;if(!width)width=1;if(!height)height=1;
      struct remesa_texture_storage *row=level?&raw->mips[level]:&raw->storage;
      *row=(struct remesa_texture_storage){.resource_cookie=123,.width=width,.height=height,
       .levels=6,.stride=(width*4+63)&~63u,.offset=offset};
      offset+=(size_t)row->stride*height;
   }
   raw->storage.bytes=offset;for(unsigned level=1;level<=5;level++)raw->mips[level].bytes=offset;
   desc=descriptor(r->format);desc.u.tex.first_level=2;desc.u.tex.last_level=4;
   assert(!remesa_texture_view_create(b,r,&desc,&v));++cases;
   assert(!remesa_texture_view_storage(b,v,&storage)&&storage.width==9&&storage.height==3&&storage.offset==raw->mips[2].offset);++cases;
   struct remesa_texture_storage unchanged=storage;
   assert(remesa_texture_view_level_storage(b,v,1,&storage)==EINVAL&&!memcmp(&unchanged,&storage,sizeof(storage)));++cases;
   views[0]=v;assert(!remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,0,1,0,views));
   assert(remesa_texture_draw_admit(b)==ENOTSUP);++cases;
   raw->mips[5].offset++;
   assert(remesa_texture_draw_admit(b)==EINVAL);--raw->mips[5].offset;++cases;
   raw->mips[4].width++;
   assert(remesa_texture_view_storage(b,v,&storage)==EINVAL&&!memcmp(&unchanged,&storage,sizeof(storage)));--raw->mips[4].width;++cases;
   /* Even valid changed backing metadata cannot replace the retained snapshot. */
   raw->storage.bytes+=64;for(unsigned level=1;level<=5;level++)raw->mips[level].bytes+=64;
   assert(remesa_texture_draw_admit(b)==ESTALE);++cases;
   raw->storage.bytes-=64;for(unsigned level=1;level<=5;level++)raw->mips[level].bytes-=64;
   struct pipe_sampler_view bad=desc;bad.u.tex.first_level=5;bad.u.tex.last_level=4;
   struct pipe_sampler_view *rejected=(void *)(uintptr_t)1;
   assert(remesa_texture_view_create(b,r,&bad,&rejected)==EINVAL&&!rejected);++cases;
   bad=desc;bad.u.tex.last_layer=1;
   assert(remesa_texture_view_create(b,r,&bad,&rejected)==ENOTSUP&&!rejected);++cases;
   assert(!remesa_texture_views_bind(b,PIPE_SHADER_FRAGMENT,0,0,1,NULL));
   pipe_sampler_view_release_ptr(&v);remesa_resource_reference(&r,NULL);assert(!f.resources);++cases;
   remesa_texture_binding_close(b); assert(!m.live);
   m=(struct memory){.fail=1}; assert(!bank(&f,&m)&&!m.live); ++cases;
   assert(!remesa_texture_binding_create(&f.screen,query,&f,allocate,NULL,&m)); ++cases;
   m=(struct memory){0}; b=bank(&f,&m); assert(b);
   r=resource_create(&f,1,1,PIPE_FORMAT_R8G8B8A8_UNORM); desc=descriptor(r->format);
   for (unsigned i=0;i<1024;i++) {
      assert(!remesa_texture_view_create(b,r,&desc,&v));
      pipe_sampler_view_release(v);
   }
   assert(remesa_texture_view_create(b,r,&desc,&v)==ENOSPC&&!v);
   assert(r->reference.count==1);
   remesa_resource_reference(&r,NULL); remesa_texture_binding_close(b);
   assert(!m.live&&!f.resources); ++cases;
   printf("texture binding: %u cases PASS; metadata/ownership only; GPU sampling unsupported\n",cases);
   return 0;
}
