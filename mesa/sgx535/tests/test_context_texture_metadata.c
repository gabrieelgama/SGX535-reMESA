/* SPDX-License-Identifier: MIT */
#include "remesa_screen.h"
#include "remesa_device.h"
#include "pipe/p_context.h"
#include "util/u_inlines.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef NDEBUG
#error Assertions must remain enabled
#endif
/* Simulated provider metadata/completion. No rasterization or SGX operation. */
struct fixture { unsigned allocations,frees,maps,unmaps,waits,queries,submits;
 int wait_error;struct remesa_texture_mapping mapping; };
static int allocate(void *u,size_t n,void **out)
{ struct fixture *f=u;*out=calloc(1,n);if(!*out)return ENOMEM;++f->allocations;return 0; }
static void release(void *u,void *bo){++((struct fixture *)u)->frees;free(bo);}
static int map(void *u,void *bo,bool write,void **out)
{ (void)write;++((struct fixture *)u)->maps;*out=bo;return 0; }
static void unmap(void *u,void *bo,bool write)
{ (void)bo;(void)write;++((struct fixture *)u)->unmaps; }
static int submit(void *u,void *bo,const struct remesa_draw_packet *p,void **f)
{ (void)bo;(void)p;(void)f;++((struct fixture *)u)->submits;return ENOTSUP; }
static int wait(void *u,void *f,uint64_t timeout,bool visible)
{ (void)f;assert(timeout==0);assert(visible);++((struct fixture *)u)->waits;return ((struct fixture *)u)->wait_error; }
static void destroy_fence(void *u,void *f){(void)u;(void)f;}
static int query(void *u,struct pipe_resource *resource,struct remesa_texture_mapping *out)
{ struct fixture *f=u;assert(resource);++f->queries;*out=f->mapping;return 0; }
static unsigned epoch_cases(const struct remesa_device_ops *ops)
{
 struct fixture f={0};unsigned cases=0;
 struct remesa_device device={.ops=ops,.priv=&f,.resource_only_backend=true,
  .resource_limits={.row_alignment=64,.max_width=4096,.max_height=4096,.max_resource_bytes=64*1024*1024}};
 struct pipe_screen *s=remesa_screen_create(&device);assert(s);
 struct pipe_context *c=s->context_create(s,NULL,0),*other=s->context_create(s,NULL,0);assert(c&&other);
 struct pipe_resource templ={.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_R8G8B8A8_UNORM,
  .width0=1280,.height0=800,.last_level=10,.depth0=1,.array_size=1};
 struct pipe_resource *r=s->resource_create(s,&templ);assert(r);
 struct pipe_sampler_view desc={.target=PIPE_TEXTURE_2D,.format=r->format,
  .swizzle_r=PIPE_SWIZZLE_X,.swizzle_g=PIPE_SWIZZLE_Y,.swizzle_b=PIPE_SWIZZLE_Z,.swizzle_a=PIPE_SWIZZLE_W};
 desc.u.tex.first_level=5;desc.u.tex.last_level=10;
 struct pipe_sampler_view *v=c->create_sampler_view(c,r,&desc);assert(v);
 struct remesa_texture_storage storage;
 assert(!remesa_context_texture_view_storage(c,v,&storage));
 f.mapping=(struct remesa_texture_mapping){.owner=55,.resource_cookie=storage.resource_cookie,
  .allocation_generation=1,.mapping_generation=2,.gpu_va=0x10000,.mapped_bytes=storage.bytes+128,
  .resource_offset=128,.cpu_write_epoch=1,.device_visible_epoch=1,.gem_handle=3,.pins=1,.mapped=true,.pfn_verified=true};
 struct pipe_sampler_state sampler={.wrap_s=PIPE_TEX_WRAP_CLAMP_TO_EDGE,.wrap_t=PIPE_TEX_WRAP_CLAMP_TO_EDGE,
  .wrap_r=PIPE_TEX_WRAP_CLAMP_TO_EDGE,.min_img_filter=PIPE_TEX_FILTER_NEAREST,.mag_img_filter=PIPE_TEX_FILTER_NEAREST,
  .min_mip_filter=PIPE_TEX_MIPFILTER_NONE};
 struct remesa_texture_descriptor d,current;
 assert(!remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&d)&&!d.content_epoch);++cases;
 uint32_t address=0;
 assert(!remesa_texture_descriptor_image_address(&d,&address)&&address==0x10080+storage.offset);++cases;
 assert(!remesa_context_revalidate_texture_descriptor(c,&d,v,&sampler,55,query,&f));++cases;
 unsigned queries=f.queries;
 assert(remesa_context_revalidate_texture_descriptor(other,&d,v,&sampler,55,query,&f)==ESTALE&&f.queries==queries);++cases;
 struct pipe_box box={.width=1,.height=1,.depth=1};struct pipe_transfer *tr=NULL;
 unsigned char *ptr=c->texture_map(c,r,10,PIPE_MAP_WRITE|PIPE_MAP_FLUSH_EXPLICIT,&box,&tr);assert(ptr&&tr);ptr[0]=17;
 assert(remesa_context_revalidate_texture_descriptor(c,&d,v,&sampler,55,query,&f)==EBUSY&&f.queries==queries);++cases;
 c->texture_unmap(c,tr); /* Unflushed staging never mutates backing or epoch. */
 assert(!remesa_context_revalidate_texture_descriptor(c,&d,v,&sampler,55,query,&f));++cases;
 ptr=c->texture_map(c,r,10,PIPE_MAP_WRITE|PIPE_MAP_FLUSH_EXPLICIT,&box,&tr);assert(ptr&&tr);ptr[0]=29;
 struct pipe_box empty=box;empty.width=0;
 c->transfer_flush_region(c,tr,&empty);assert(!remesa_context_error(c));c->texture_unmap(c,tr);
 assert(!remesa_context_revalidate_texture_descriptor(c,&d,v,&sampler,55,query,&f));++cases;
 ptr=c->texture_map(c,r,10,PIPE_MAP_WRITE|PIPE_MAP_FLUSH_EXPLICIT,&box,&tr);assert(ptr&&tr);ptr[0]=31;
 c->transfer_flush_region(c,tr,&box);assert(!remesa_context_error(c));c->texture_unmap(c,tr);
 /* Provider epochs deliberately remain unchanged; the frontend still detects
  * a write anywhere in the shared mip allocation. */
 assert(remesa_context_revalidate_texture_descriptor(c,&d,v,&sampler,55,query,&f)==ESTALE);++cases;
 assert(!remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&d)&&d.content_epoch==1);++cases;
 ptr=other->texture_map(other,r,5,PIPE_MAP_READ,&box,&tr);assert(ptr&&tr);other->texture_unmap(other,tr);
 assert(!remesa_context_revalidate_texture_descriptor(c,&d,v,&sampler,55,query,&f));++cases;
 const unsigned char pixel[4]={7,13,19,255};other->texture_subdata(other,r,5,0,&box,pixel,4,4);
 assert(!remesa_context_error(other));
 assert(remesa_context_revalidate_texture_descriptor(c,&d,v,&sampler,55,query,&f)==ESTALE);++cases;
 assert(!remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&d)&&d.content_epoch==2);++cases;
 unsigned char serialized[REMESA_TEXTURE_HOST_METADATA_BYTES];
 assert(!remesa_texture_descriptor_serialize(&d,serialized,sizeof(serialized))&&serialized[152]==2);++cases;
 struct pipe_fence_handle *fence=NULL;assert(!remesa_resource_attach_writer(c,r,&f,&fence));f.wait_error=EAGAIN;
 queries=f.queries;
 assert(remesa_context_revalidate_texture_descriptor(c,&d,v,&sampler,55,query,&f)==EAGAIN&&f.queries==queries);++cases;
 f.wait_error=0;
 assert(remesa_context_revalidate_texture_descriptor(c,&d,v,&sampler,55,query,&f)==ESTALE);++cases;
 assert(!remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&current)&&current.content_epoch==3);++cases;
 s->fence_reference(s,&fence,NULL);
 assert(!remesa_resource_attach_writer(c,r,&f,&fence));f.wait_error=EIO;queries=f.queries;
 assert(remesa_context_revalidate_texture_descriptor(c,&current,v,&sampler,55,query,&f)==EIO&&f.queries==queries);++cases;
 f.wait_error=0;
 assert(remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&d)==EIO&&f.queries==queries);++cases;
 s->fence_reference(s,&fence,NULL);pipe_resource_reference(&r,NULL);
 c->destroy(c);other->destroy(other);s->destroy(s);pipe_sampler_view_release_ptr(&v);
 assert(f.allocations==f.frees&&f.maps==f.unmaps&&!f.submits);++cases;
 return cases;
}
static unsigned mip_cases(const struct remesa_device_ops *ops)
{
 struct fixture f={0};unsigned cases=0;
 struct remesa_device d={.ops=ops,.priv=&f,.resource_only_backend=true,
  .resource_limits={.row_alignment=64,.max_width=4096,.max_height=4096,.max_resource_bytes=64*1024*1024}};
 struct pipe_screen *s=remesa_screen_create(&d);assert(s);
 struct pipe_context *c=s->context_create(s,NULL,0),*other=s->context_create(s,NULL,0);assert(c&&other);
 struct pipe_resource templ={.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_B8G8R8A8_UNORM,
  .width0=1280,.height0=800,.last_level=10,.depth0=1,.array_size=1};
 struct pipe_resource *r=s->resource_create(s,&templ);assert(r);
 struct pipe_sampler_view desc={.target=PIPE_TEXTURE_2D,.format=r->format,
  .swizzle_r=PIPE_SWIZZLE_X,.swizzle_g=PIPE_SWIZZLE_Y,.swizzle_b=PIPE_SWIZZLE_Z,.swizzle_a=PIPE_SWIZZLE_W};
 struct pipe_sampler_state sampler={.wrap_s=PIPE_TEX_WRAP_CLAMP_TO_EDGE,.wrap_t=PIPE_TEX_WRAP_CLAMP_TO_EDGE,
  .wrap_r=PIPE_TEX_WRAP_CLAMP_TO_EDGE,.min_img_filter=PIPE_TEX_FILTER_NEAREST,.mag_img_filter=PIPE_TEX_FILTER_NEAREST,
  .min_mip_filter=PIPE_TEX_MIPFILTER_NONE};
 for(unsigned first=0;first<=10;first++)for(unsigned last=first;last<=10;last++) {
  desc.u.tex.first_level=first;desc.u.tex.last_level=last;
  struct pipe_sampler_view *v=c->create_sampler_view(c,r,&desc);assert(v&&r->reference.count==2);
  struct remesa_texture_storage storage={0},base;
  assert(!remesa_context_texture_view_storage(c,v,&base));
  size_t offset=0;
  for(unsigned level=0;level<=10;level++) {
   unsigned width=1280u>>level,height=800u>>level;if(!width)width=1;if(!height)height=1;
   unsigned stride=(width*4+63)&~63u;
   if(level>=first&&level<=last) {
    assert(!remesa_context_texture_view_level_storage(c,v,level,&storage));
    assert(storage.offset==offset&&storage.width==width&&storage.height==height&&
           storage.stride==stride&&storage.levels==11&&storage.resource_cookie==base.resource_cookie);
   } else {
    struct remesa_texture_storage before=storage;
    assert(remesa_context_texture_view_level_storage(c,v,level,&storage)==EINVAL);
    assert(!memcmp(&storage,&before,sizeof(storage)));
   }
   offset+=(size_t)stride*height;
  }
  assert(base.bytes==offset&&base.levels==11);
  f.mapping=(struct remesa_texture_mapping){.owner=55,.resource_cookie=base.resource_cookie,
   .allocation_generation=1,.mapping_generation=2,.gpu_va=0x10000,.mapped_bytes=offset+128,
   .resource_offset=128,.cpu_write_epoch=1,.device_visible_epoch=1,.gem_handle=3,.pins=1,.mapped=true,.pfn_verified=true};
  struct remesa_texture_descriptor image,before;memset(&image,0xa5,sizeof(image));before=image;
  assert(!remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&image));
  assert(image.width==base.width&&image.height==base.height&&image.level_offset==base.offset&&
         image.resource_bytes==base.bytes&&image.mapping.resource_offset==128&&
         image.first_level==first&&image.last_level==last);
  unsigned char encoded[REMESA_TEXTURE_HOST_METADATA_BYTES];
  assert(!remesa_texture_descriptor_serialize(&image,encoded,sizeof(encoded)));
  assert(!memcmp(encoded,"RMTD\3\0\0\0",8));
  before=image;unsigned queries=f.queries;
  assert(remesa_context_prepare_texture_descriptor(other,v,&sampler,55,query,&f,&image)==ESTALE);
  assert(f.queries==queries&&!memcmp(&image,&before,sizeof(image)));
  /* A CPU lease anywhere in this BO invalidates every mip descriptor. */
  struct pipe_transfer *transfer=NULL;struct pipe_box box={.width=1,.height=1,.depth=1};
  assert(c->texture_map(c,r,10,PIPE_MAP_READ,&box,&transfer));
  assert(remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&image)==EBUSY);
  assert(f.queries==queries&&!memcmp(&image,&before,sizeof(image)));c->texture_unmap(c,transfer);
  c->set_sampler_views(c,PIPE_SHADER_FRAGMENT,0,1,0,&v);assert(!remesa_context_error(c));
  c->set_sampler_views(c,PIPE_SHADER_FRAGMENT,0,0,1,NULL);assert(v->reference.count==1);
  pipe_sampler_view_release_ptr(&v);assert(r->reference.count==1);++cases;
 }
 desc.u.tex.first_level=5;desc.u.tex.last_level=4;assert(!c->create_sampler_view(c,r,&desc)&&remesa_context_error(c)==EINVAL);++cases;
 desc.u.tex.first_level=0;desc.u.tex.last_level=11;assert(!c->create_sampler_view(c,r,&desc)&&remesa_context_error(c)==EINVAL);++cases;
 desc.u.tex.last_level=10;desc.u.tex.first_layer=1;assert(!c->create_sampler_view(c,r,&desc)&&remesa_context_error(c)==ENOTSUP);++cases;
 desc.u.tex.first_layer=0;desc.u.tex.first_level=3;
 struct pipe_sampler_view *v=c->create_sampler_view(c,r,&desc);assert(v);
 struct remesa_texture_storage storage;assert(!remesa_context_texture_view_storage(c,v,&storage));
 f.mapping.resource_cookie=storage.resource_cookie;
 struct pipe_fence_handle *fence=NULL;assert(!remesa_resource_attach_writer(c,r,&f,&fence));
 f.wait_error=EAGAIN;unsigned queries=f.queries;struct remesa_texture_descriptor image;
 assert(remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&image)==EAGAIN&&f.queries==queries);++cases;
 f.wait_error=0;assert(!remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&image));
 s->fence_reference(s,&fence,NULL);pipe_resource_reference(&r,NULL);
 c->destroy(c);other->destroy(other);s->destroy(s);assert(f.allocations==f.frees+1);
 pipe_sampler_view_release_ptr(&v);
 assert(f.allocations==f.frees&&f.maps==f.unmaps&&!f.submits);++cases;
 return cases;
}
int main(void)
{
 unsigned cases=0;struct fixture f={0};
 const struct remesa_device_ops ops={.allocate=allocate,.destroy_bo=release,.map=map,.unmap=unmap,.submit=submit,.wait=wait,.destroy_fence=destroy_fence};
 struct remesa_device d={.ops=&ops,.priv=&f,.resource_only_backend=true,
  .resource_limits={.row_alignment=64,.max_width=4096,.max_height=4096,.max_resource_bytes=64*1024*1024}};
 struct pipe_screen *s=remesa_screen_create(&d);assert(s);
 struct pipe_context *c=s->context_create(s,NULL,0),*other=s->context_create(s,NULL,0);assert(c&&other);
 struct pipe_resource t={.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_R8G8B8A8_UNORM,
  .width0=137,.height0=59,.depth0=1,.array_size=1};
 struct pipe_resource *r=s->resource_create(s,&t);assert(r);
 struct pipe_sampler_view desc={.target=PIPE_TEXTURE_2D,.format=r->format,
  .swizzle_r=PIPE_SWIZZLE_X,.swizzle_g=PIPE_SWIZZLE_Y,.swizzle_b=PIPE_SWIZZLE_Z,.swizzle_a=PIPE_SWIZZLE_W};
 struct pipe_sampler_view *v=c->create_sampler_view(c,r,&desc);assert(v);
 struct remesa_texture_storage storage={0};
 assert(!remesa_context_texture_view_storage(c,v,&storage));assert(storage.resource_cookie&&storage.stride==576);++cases;
 assert(remesa_context_texture_view_storage(other,v,&storage)==ESTALE);++cases;
 struct pipe_sampler_state sampler={.wrap_s=PIPE_TEX_WRAP_CLAMP_TO_EDGE,.wrap_t=PIPE_TEX_WRAP_CLAMP_TO_EDGE,
  .wrap_r=PIPE_TEX_WRAP_CLAMP_TO_EDGE,.min_img_filter=PIPE_TEX_FILTER_NEAREST,
  .mag_img_filter=PIPE_TEX_FILTER_NEAREST,.min_mip_filter=PIPE_TEX_MIPFILTER_NONE};
 f.mapping=(struct remesa_texture_mapping){.owner=55,.resource_cookie=storage.resource_cookie,
  .allocation_generation=1,.mapping_generation=2,.gpu_va=0x10000,.mapped_bytes=36864,
  .cpu_write_epoch=1,.device_visible_epoch=1,.gem_handle=3,.pins=1,.mapped=true,.pfn_verified=true};
 struct remesa_texture_descriptor out,before;memset(&out,0xa5,sizeof(out));before=out;
 /* A foreign Gallium context is not a reMESA private context, even when it
  * supplies a valid pipe_context object. Reject before private casts/query. */
 struct pipe_context foreign={0};struct remesa_texture_storage storage_before=storage;
 unsigned foreign_queries=f.queries;
 assert(remesa_context_texture_view_storage(&foreign,v,&storage)==EINVAL);
 assert(!memcmp(&storage,&storage_before,sizeof(storage)));++cases;
 assert(remesa_context_texture_view_level_storage(&foreign,v,0,&storage)==EINVAL);
 assert(!memcmp(&storage,&storage_before,sizeof(storage)));++cases;
 assert(remesa_context_prepare_texture_descriptor(&foreign,v,&sampler,55,query,&f,&out)==EINVAL);
 assert(!memcmp(&out,&before,sizeof(out))&&f.queries==foreign_queries);++cases;
 assert(remesa_context_revalidate_texture_descriptor(&foreign,&out,v,&sampler,55,query,&f)==EINVAL);
 assert(f.queries==foreign_queries);++cases;
 assert(remesa_context_prepare_texture_descriptor(other,v,&sampler,55,query,&f,&out)==ESTALE);
 assert(!f.queries&&!memcmp(&out,&before,sizeof(out)));++cases;
 assert(!remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&out));
 assert(out.width==137&&out.height==59&&out.stride==576&&out.mapping.resource_cookie==storage.resource_cookie);++cases;
 before=out;f.mapping.mapping_generation=0;
 assert(remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&out)==EINVAL);
 assert(!memcmp(&out,&before,sizeof(out)));++cases;f.mapping.mapping_generation=2;
 struct pipe_transfer *tr=NULL;struct pipe_box box={.width=1,.height=1,.depth=1};
 assert(c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr)&&tr);
 unsigned queries=f.queries;
 assert(remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&out)==EBUSY);
 assert(f.queries==queries&&!memcmp(&out,&before,sizeof(out)));++cases;c->texture_unmap(c,tr);
 struct pipe_fence_handle *fence=NULL;assert(!remesa_resource_attach_writer(c,r,&f,&fence));
 f.wait_error=EAGAIN;queries=f.queries;
 /* A trusted-query fixture claiming no writer cannot override this frontend's
  * actual pending writer fence: poll first, never inspect the mapping. */
 assert(remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&out)==EAGAIN);
 assert(f.queries==queries&&f.waits==1&&!memcmp(&out,&before,sizeof(out)));++cases;
 f.wait_error=0;assert(!remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&out));
 assert(f.waits==2&&f.queries==queries+1);++cases;
 f.mapping.device_visible_epoch=0;before=out;
 assert(remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&out)==EAGAIN);
 assert(!memcmp(&before,&out,sizeof(out)));++cases;f.mapping.device_visible_epoch=1;
 f.mapping.owner=56;assert(remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&out)==ESTALE);++cases;
 f.mapping.owner=55;sampler.min_img_filter=PIPE_TEX_FILTER_LINEAR;
 assert(remesa_context_prepare_texture_descriptor(c,v,&sampler,55,query,&f,&out)==ENOTSUP);++cases;
 assert(!f.submits);s->fence_reference(s,&fence,NULL);pipe_resource_reference(&r,NULL);
 c->destroy(c);other->destroy(other);s->destroy(s);pipe_sampler_view_release_ptr(&v);
 assert(f.allocations==f.frees&&f.maps==f.unmaps&&!f.submits);++cases;
 cases+=mip_cases(&ops);
 cases+=epoch_cases(&ops);
 printf("PASS %u actual Gallium metadata/fence bridge cases including 66 mip ranges; mocked mapping and completion, zero SGX\n",cases);
}
