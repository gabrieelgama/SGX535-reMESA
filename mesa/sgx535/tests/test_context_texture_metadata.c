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
int main(void)
{
 unsigned cases=0;struct fixture f={0};
 const struct remesa_device_ops ops={allocate,release,map,unmap,submit,wait,destroy_fence};
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
 printf("PASS %u actual Gallium metadata/fence bridge cases; mocked mapping and completion, zero SGX\n",cases);
}
