/* SPDX-License-Identifier: MIT */
/* All mapping/completion receipts here are simulated host fixtures, NOT GPU
 * visibility, sampling, SGX completion or physical execution evidence. */
#include "remesa_texture_descriptor.h"
#include "remesa_resource_ref.h"
#include "pipe/p_context.h"
#include "util/u_inlines.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef NDEBUG
#error Assertions must remain enabled
#endif
static unsigned cases;
struct resource { struct pipe_resource base; struct remesa_texture_storage storage; };
struct fixture { struct pipe_screen screen; struct remesa_texture_mapping map;
                 unsigned queries,destroys; int error; };
static void destroy(struct pipe_screen *screen,struct pipe_resource *r)
{ struct fixture *f=(struct fixture *)screen; assert(!r->reference.count); ++f->destroys; free(r); }
static int storage(void *p,struct pipe_resource *r,unsigned level,struct remesa_texture_storage *out)
{ (void)p;if(level)return EINVAL; *out=((struct resource *)r)->storage; return 0; }
static int mapping(void *p,struct pipe_resource *r,struct remesa_texture_mapping *out)
{ struct fixture *f=p; assert(r->screen==&f->screen); ++f->queries;
  if (f->error) return f->error;
  *out=f->map; return 0; }
static struct pipe_sampler_state sampler(void)
{
   struct pipe_sampler_state s={0};
   s.wrap_s=s.wrap_t=s.wrap_r=PIPE_TEX_WRAP_CLAMP_TO_EDGE;
   s.min_img_filter=s.mag_img_filter=PIPE_TEX_FILTER_NEAREST;
   s.min_mip_filter=PIPE_TEX_MIPFILTER_NONE; return s;
}
static struct remesa_texture_mapping valid_mapping(void)
{
   return (struct remesa_texture_mapping){.owner=7,.resource_cookie=19,
      .allocation_generation=21,.mapping_generation=31,.gpu_va=0x10000,
      .mapped_bytes=8192,.resource_offset=64,.cpu_write_epoch=9,
      .device_visible_epoch=9,.gem_handle=4,.pins=1,.mapped=true,.pfn_verified=true};
}
static void fail(struct fixture *f,struct remesa_texture_binding_bank *b,
                 struct pipe_sampler_view *v,struct pipe_sampler_state *s,int error)
{
   struct remesa_texture_descriptor out,before;
   memset(&out,0xa5,sizeof(out)); before=out;
   assert(remesa_texture_descriptor_prepare(b,v,s,7,mapping,f,&out)==error);
   assert(!memcmp(&before,&out,sizeof(out))); ++cases;
}
static void layout_cases(void)
{
   const unsigned dims[][2]={{32,32},{320,240},{640,480},{800,600},
      {1024,768},{1280,720},{1280,800},{1366,768},{1600,900},{1920,1080},{37,13}};
   for (unsigned fmt=0;fmt<2;fmt++) for (unsigned n=0;n<sizeof(dims)/sizeof(dims[0]);n++) {
      struct fixture f={0}; f.screen.resource_destroy=destroy; f.map=valid_mapping();
      struct resource *raw=calloc(1,sizeof(*raw)); assert(raw);
      struct pipe_resource *r=&raw->base; r->screen=&f.screen; r->reference.count=1;
      r->target=PIPE_TEXTURE_2D; r->format=fmt?PIPE_FORMAT_B8G8R8A8_UNORM:PIPE_FORMAT_R8G8B8A8_UNORM;
      r->width0=dims[n][0]; r->height0=dims[n][1]; r->depth0=r->array_size=1;
      raw->storage.stride=(r->width0*4+63)&~63u;
      raw->storage.bytes=(size_t)raw->storage.stride*r->height0;
      raw->storage.resource_cookie=19; f.map.mapped_bytes=raw->storage.bytes+64;
      raw->storage.width=r->width0;raw->storage.height=r->height0;raw->storage.levels=1;
      struct remesa_texture_binding_bank *b=remesa_texture_binding_create(&f.screen,storage,&f,NULL,NULL,NULL);
      assert(b); struct pipe_sampler_view t={0},*v=NULL;
      t.target=PIPE_TEXTURE_2D; t.format=r->format; t.swizzle_r=PIPE_SWIZZLE_X;
      t.swizzle_g=PIPE_SWIZZLE_Y; t.swizzle_b=PIPE_SWIZZLE_Z; t.swizzle_a=PIPE_SWIZZLE_W;
      assert(!remesa_texture_view_create(b,r,&t,&v));
      struct pipe_sampler_state s=sampler(); struct remesa_texture_descriptor d;
      assert(!remesa_texture_descriptor_prepare(b,v,&s,7,mapping,&f,&d));
      assert(d.width==dims[n][0]&&d.height==dims[n][1]&&d.format==fmt+1);
      assert(d.image_bytes==raw->storage.bytes&&d.stride==raw->storage.stride);
      if (n==6) assert(d.image_bytes==4096000); /* checked semantic span, no GPU allocation */
      assert(!remesa_texture_descriptor_revalidate(&d,b,v,&s,7,mapping,&f));
      --f.map.mapped_bytes; fail(&f,b,v,&s,EINVAL); ++f.map.mapped_bytes;
      remesa_resource_reference(&r,NULL); remesa_texture_binding_close(b);
      pipe_sampler_view_release(v); assert(f.destroys==1); ++cases;
   }
}
int main(void)
{
   struct fixture f={0}; f.screen.resource_destroy=destroy; f.map=valid_mapping();
   struct resource *raw=calloc(1,sizeof(*raw)); assert(raw);
   struct pipe_resource *r=&raw->base;
   r->screen=&f.screen; r->reference.count=1; r->target=PIPE_TEXTURE_2D;
   r->format=PIPE_FORMAT_R8G8B8A8_UNORM; r->width0=33; r->height0=7;
   r->depth0=r->array_size=1;
   raw->storage=(struct remesa_texture_storage){.resource_cookie=19,.bytes=1344,.stride=192,
     .width=33,.height=7,.levels=1};
   struct remesa_texture_binding_bank *b=remesa_texture_binding_create(&f.screen,storage,&f,NULL,NULL,NULL);
   assert(b); struct pipe_sampler_view t={0},*v=NULL;
   t.target=PIPE_TEXTURE_2D; t.format=r->format;
   t.swizzle_r=PIPE_SWIZZLE_X; t.swizzle_g=PIPE_SWIZZLE_Y;
   t.swizzle_b=PIPE_SWIZZLE_Z; t.swizzle_a=PIPE_SWIZZLE_W;
   assert(!remesa_texture_view_create(b,r,&t,&v)); assert(r->reference.count==2); ++cases;
   struct pipe_sampler_state s=sampler(); struct remesa_texture_descriptor d;
   assert(!remesa_texture_descriptor_prepare(b,v,&s,7,mapping,&f,&d));
   assert(d.width==33&&d.height==7&&d.stride==192&&d.image_bytes==1344&&d.format==1);
   assert(r->reference.count==2&&v->reference.count==1); ++cases;
   assert(!remesa_texture_descriptor_revalidate(&d,b,v,&s,7,mapping,&f)); ++cases;
   uint8_t encoded[REMESA_TEXTURE_HOST_METADATA_BYTES],another[REMESA_TEXTURE_HOST_METADATA_BYTES]; memset(encoded,0xa5,sizeof(encoded));
   assert(!remesa_texture_descriptor_serialize(&d,encoded,sizeof(encoded)));
   assert(!memcmp(encoded,"RMTD\3\0\0\0",8));
   assert(encoded[8]==7&&encoded[16]==19&&encoded[24]==21&&encoded[32]==31);
   assert(encoded[42]==1&&encoded[56]==64&&encoded[64]==0x40&&encoded[65]==5);
   assert(encoded[104]==33&&encoded[108]==7&&encoded[112]==192&&encoded[116]==1);
   assert(encoded[120]==4&&encoded[124]==PIPE_SWIZZLE_X&&encoded[127]==PIPE_SWIZZLE_W);
   assert(!remesa_texture_descriptor_serialize(&d,another,sizeof(another)));
   assert(!memcmp(encoded,another,sizeof(encoded))); ++cases;
   /* Mesa retains these inactive defaults in a nearest/no-mip GL sampler.
    * They must prepare the same selected base-mip metadata. */
   s.max_lod=1000.0f; s.compare_func=PIPE_FUNC_LEQUAL;
   struct remesa_texture_descriptor mesa_defaults;
   assert(!remesa_texture_descriptor_prepare(b,v,&s,7,mapping,&f,&mesa_defaults));
   assert(!remesa_texture_descriptor_serialize(&mesa_defaults,another,sizeof(another)));
   assert(!memcmp(encoded,another,sizeof(encoded)));
   assert(!remesa_texture_descriptor_revalidate(&d,b,v,&s,7,mapping,&f)); ++cases;
   s=sampler();
   uint32_t address=0xa5a5a5a5;
   assert(!remesa_texture_descriptor_image_address(&d,&address)&&address==0x10040); ++cases;
   /* This is relocation-range admission only, not a texture format encoding. */
   struct remesa_texture_descriptor edge=d;
   edge.mapping.gpu_va=(UINT64_C(1)<<32)-edge.mapping.mapped_bytes;
   assert(!remesa_texture_descriptor_image_address(&edge,&address)); ++cases;
   address=0xa5a5a5a5;edge.mapping.gpu_va+=4;
   assert(remesa_texture_descriptor_image_address(&edge,&address)==ERANGE&&address==0xa5a5a5a5); ++cases;
   edge=d;edge.mapping.gpu_va=UINT64_C(1)<<32;
   assert(remesa_texture_descriptor_image_address(&edge,&address)==ERANGE&&address==0xa5a5a5a5); ++cases;
   size_t written=123; memset(another,0xa5,sizeof(another)); uint8_t before[sizeof(another)]; memcpy(before,another,sizeof(before));
   assert(remesa_texture_descriptor_emit_sgx(&d,another,sizeof(another),&written)==ENOTSUP);
   assert(written==123&&!memcmp(before,another,sizeof(before))); ++cases;
   assert(remesa_texture_descriptor_emit_sgx(NULL,NULL,0,NULL)==ENOTSUP); ++cases;
   struct remesa_texture_mapping good=f.map;
#define BAD(field,value,error) do { f.map=good; f.map.field=(value); fail(&f,b,v,&s,(error)); } while (0)
   BAD(owner,0,EINVAL); BAD(owner,8,ESTALE); BAD(resource_cookie,0,EINVAL);
   BAD(resource_cookie,18,ESTALE); BAD(gem_handle,0,EINVAL);
   BAD(allocation_generation,0,EINVAL); BAD(mapping_generation,0,EINVAL);
   BAD(quarantined,true,EIO); BAD(mapped,false,EBUSY); BAD(pfn_verified,false,EBUSY);
   BAD(pins,0,EBUSY); BAD(cpu_leases,1,EBUSY); BAD(cpu_leases,UINT32_MAX,EBUSY);
   BAD(gpu_va,0,EINVAL); BAD(gpu_va,3,EINVAL); BAD(gpu_va,UINT64_MAX-3,EINVAL);
   BAD(gpu_va,UINT64_C(1)<<32,ERANGE);
   BAD(gpu_va,(UINT64_C(1)<<32)-4096,ERANGE);
   BAD(mapped_bytes,0,EINVAL); BAD(mapped_bytes,UINT64_MAX,EINVAL);
   BAD(resource_offset,3,EINVAL); BAD(resource_offset,8196,EINVAL);
   BAD(resource_offset,8192,EINVAL); BAD(mapped_bytes,1407,EINVAL);
   BAD(cpu_write_epoch,0,EAGAIN); BAD(device_visible_epoch,8,EAGAIN);
   BAD(device_visible_epoch,10,EAGAIN); BAD(writer_operation,1,EPROTO);
   BAD(writer_generation,1,EPROTO); BAD(completed_operation,1,EPROTO);
   BAD(completed_generation,1,EPROTO);
   f.map=good; f.map.writer_operation=4; f.map.writer_generation=8;
   fail(&f,b,v,&s,EAGAIN); f.map.completed_operation=3; f.map.completed_generation=8;
   fail(&f,b,v,&s,EAGAIN); f.map.completed_operation=4; f.map.completed_generation=7;
   fail(&f,b,v,&s,EAGAIN); f.map.completed_generation=8;
   struct remesa_texture_descriptor gpu_written;
   assert(!remesa_texture_descriptor_prepare(b,v,&s,7,mapping,&f,&gpu_written)); ++cases;
   f.map.completed_generation=9;
   assert(remesa_texture_descriptor_revalidate(&gpu_written,b,v,&s,7,mapping,&f)==EAGAIN); ++cases;
   f.map=good;
#define STALE(field,value) do { f.map=good; f.map.field=(value); assert(remesa_texture_descriptor_revalidate(&d,b,v,&s,7,mapping,&f)==ESTALE); ++cases; } while (0)
   STALE(allocation_generation,22); STALE(mapping_generation,32); STALE(gpu_va,0x20000);
   STALE(mapped_bytes,16384); STALE(resource_offset,128); STALE(gem_handle,5);
   f.map=good; f.map.cpu_write_epoch=f.map.device_visible_epoch=10;
   assert(remesa_texture_descriptor_revalidate(&d,b,v,&s,7,mapping,&f)==ESTALE); ++cases;
   f.map=good;
#define SAMPLER_BAD(field,value) do { s=sampler(); s.field=(value); fail(&f,b,v,&s,ENOTSUP); } while (0)
   SAMPLER_BAD(wrap_s,PIPE_TEX_WRAP_REPEAT); SAMPLER_BAD(wrap_t,PIPE_TEX_WRAP_REPEAT);
   SAMPLER_BAD(wrap_r,PIPE_TEX_WRAP_REPEAT); SAMPLER_BAD(min_img_filter,PIPE_TEX_FILTER_LINEAR);
   SAMPLER_BAD(mag_img_filter,PIPE_TEX_FILTER_LINEAR); SAMPLER_BAD(min_mip_filter,PIPE_TEX_MIPFILTER_LINEAR);
   SAMPLER_BAD(compare_mode,1); SAMPLER_BAD(unnormalized_coords,1);
   SAMPLER_BAD(border_color_is_integer,1);
   SAMPLER_BAD(border_color_format,PIPE_FORMAT_R8G8B8A8_UNORM);
   SAMPLER_BAD(border_color.ui[0],1);
   SAMPLER_BAD(max_anisotropy,2); SAMPLER_BAD(seamless_cube_map,1);
   SAMPLER_BAD(reduction_mode,1); SAMPLER_BAD(pad,1);
   SAMPLER_BAD(lod_bias,NAN); SAMPLER_BAD(min_lod,-1);
   SAMPLER_BAD(max_lod,NAN); SAMPLER_BAD(max_lod,INFINITY); SAMPLER_BAD(max_lod,-1);
   s=sampler(); f.error=EIO; fail(&f,b,v,&s,EIO);
   f.error=-EIO; fail(&f,b,v,&s,EPROTO); f.error=0;
   unsigned queries=f.queries;
   fail(&f,b,(void *)(uintptr_t)1,&s,ESTALE); assert(f.queries==queries);
   ++raw->storage.resource_cookie; fail(&f,b,v,&s,ESTALE); --raw->storage.resource_cookie;
   v->swizzle_a=PIPE_SWIZZLE_0; fail(&f,b,v,&s,ESTALE); v->swizzle_a=PIPE_SWIZZLE_W;
   struct remesa_texture_descriptor invalid=d; invalid.image_bytes++;
   assert(remesa_texture_descriptor_serialize(&invalid,another,sizeof(another))==EINVAL);
   assert(!memcmp(before,another,sizeof(before))); ++cases;
   assert(remesa_texture_descriptor_serialize(&d,another,sizeof(another)-1)==EINVAL);
   assert(!memcmp(before,another,sizeof(before))); ++cases;
   /* Release frontend's resource reference: view still retains backing. */
   remesa_resource_reference(&r,NULL); assert(!r&&v->texture->reference.count==1); ++cases;
   assert(!remesa_texture_descriptor_revalidate(&d,b,v,&s,7,mapping,&f)); ++cases;
   /* Provider metadata staying identical must not hide a frontend write.
    * Revisions are monotonic: rollback must never resurrect an old snapshot. */
   raw->storage.content_epoch=1;
   assert(remesa_texture_descriptor_revalidate(&d,b,v,&s,7,mapping,&f)==ESTALE); ++cases;
   struct remesa_texture_descriptor revised;
   assert(!remesa_texture_descriptor_prepare(b,v,&s,7,mapping,&f,&revised)&&revised.content_epoch==1);
   assert(!remesa_texture_descriptor_serialize(&revised,another,sizeof(another))&&another[152]==1); ++cases;
   raw->storage.content_epoch=7;
   assert(remesa_texture_descriptor_revalidate(&revised,b,v,&s,7,mapping,&f)==ESTALE); ++cases;
   struct remesa_texture_descriptor unchanged=revised;
   unsigned epoch_queries=f.queries;
   raw->storage.content_epoch=1;
   assert(remesa_texture_descriptor_revalidate(&revised,b,v,&s,7,mapping,&f)==ESTALE);
   assert(f.queries==epoch_queries); ++cases;
   assert(remesa_texture_descriptor_prepare(b,v,&s,7,mapping,&f,&unchanged)==ESTALE);
   assert(!memcmp(&unchanged,&revised,sizeof(revised))&&f.queries==epoch_queries); ++cases;
   raw->storage.content_epoch=UINT64_MAX;
   assert(remesa_texture_descriptor_revalidate(&revised,b,v,&s,7,mapping,&f)==ESTALE); ++cases;
   remesa_texture_binding_close(b); fail(&f,b,v,&s,EINVAL);
   pipe_sampler_view_release(v); assert(f.destroys==1); ++cases;
   layout_cases();
   printf("PASS %u semantic texture descriptor cases; simulated mapping/completion; SGX sampling unavailable\n",cases);
   return 0;
}
