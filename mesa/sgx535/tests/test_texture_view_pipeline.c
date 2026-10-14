/* SPDX-License-Identifier: MIT */
#include "remesa_screen.h"
#include "remesa_device.h"
#include "pipe/p_context.h"
#include "util/u_inlines.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef NDEBUG
#error Tests require enabled assertions
#endif
/* Resource callbacks and fence events are mocked. No rendering or CPU texture
 * sampling implementation exists in this fixture. */
struct fixture { unsigned allocations,frees,maps,unmaps,submits,fences,waits;
                 int wait_error; };
static int allocate(void *p,size_t n,void **out)
{ struct fixture *f=p; *out=calloc(1,n); if (!*out) return ENOMEM;
  ++f->allocations; return 0; }
static void destroy_bo(void *p,void *bo)
{ ++((struct fixture *)p)->frees; free(bo); }
static int map_bo(void *p,void *bo,bool write,void **out)
{ (void)write; ++((struct fixture *)p)->maps; *out=bo; return 0; }
static void unmap_bo(void *p,void *bo,bool write)
{ (void)bo; (void)write; ++((struct fixture *)p)->unmaps; }
static int submit(void *p,void *bo,const struct remesa_draw_packet *packet,void **fence)
{ (void)bo; (void)packet; (void)fence; ++((struct fixture *)p)->submits; return ENOTSUP; }
static int wait(void *p,void *fence,uint64_t timeout,bool visible)
{ (void)fence; (void)timeout; assert(visible); struct fixture *f=p; ++f->waits; return f->wait_error; }
static void destroy_fence(void *p,void *fence)
{ (void)fence; ++((struct fixture *)p)->fences; }
static struct pipe_resource *resource(struct pipe_screen *s,enum pipe_format format)
{
   struct pipe_resource t={.target=PIPE_TEXTURE_2D,.format=format,.width0=137,
                           .height0=59,.depth0=1,.array_size=1};
   return s->resource_create(s,&t);
}
static struct pipe_sampler_view descriptor(enum pipe_format format)
{
   struct pipe_sampler_view v={.target=PIPE_TEXTURE_2D,.format=format,
      .swizzle_r=PIPE_SWIZZLE_X,.swizzle_g=PIPE_SWIZZLE_Y,
      .swizzle_b=PIPE_SWIZZLE_Z,.swizzle_a=PIPE_SWIZZLE_W}; return v;
}
int main(void)
{
   unsigned cases=0;
   struct fixture f={0};
   const struct remesa_device_ops ops={.allocate=allocate,.destroy_bo=destroy_bo,.map=map_bo,.unmap=unmap_bo,.submit=submit,.wait=wait,.destroy_fence=destroy_fence};
   struct remesa_device d={.ops=&ops,.priv=&f,.resource_only_backend=true,
      .resource_limits={.row_alignment=64,.max_width=4096,.max_height=4096,
                        .max_resource_bytes=64*1024*1024}};
   struct pipe_screen *s=remesa_screen_create(&d); assert(s);
   struct pipe_context *c=s->context_create(s,NULL,0),*other=s->context_create(s,NULL,0);
   assert(c&&other);
   assert(!s->is_format_supported(s,PIPE_FORMAT_R8G8B8A8_UNORM,PIPE_TEXTURE_2D,
                                  0,0,PIPE_BIND_SAMPLER_VIEW)); ++cases;
   struct pipe_resource *r=resource(s,PIPE_FORMAT_R8G8B8A8_UNORM); assert(r);
   struct pipe_sampler_view desc=descriptor(r->format);
   struct pipe_sampler_view *view=c->create_sampler_view(c,r,&desc);
   assert(view&&!remesa_context_error(c));
   assert(r->reference.count==2&&view->reference.count==1); ++cases;
   /* Host view metadata never enables hardware shader sampling. */
   struct pipe_sampler_view *views[]={view};
   c->set_sampler_views(c,PIPE_SHADER_FRAGMENT,0,1,0,views);
   assert(!remesa_context_error(c)&&view->reference.count==2); ++cases;
   other->set_sampler_views(other,PIPE_SHADER_FRAGMENT,0,1,0,views);
   assert(remesa_context_error(other)==ESTALE);
   other->set_sampler_views(other,PIPE_SHADER_FRAGMENT,0,1,0,NULL);
   assert(!remesa_context_error(other)); ++cases;
   struct pipe_sampler_view *alias=NULL;
   pipe_sampler_view_reference(&alias,view);
   pipe_sampler_view_release_ptr(&alias);
   assert(!alias&&view->reference.count==2); ++cases;
   struct pipe_box box={.x=136,.y=58,.width=1,.height=1,.depth=1};
   const unsigned char bytes[4]={3,17,61,251};
   c->texture_subdata(c,r,0,0,&box,bytes,4,4);
   assert(!remesa_context_error(c));
   struct pipe_transfer *tr=NULL;
   unsigned char *p=c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr);
   assert(p&&tr->stride==576&&!memcmp(p,bytes,4));
   assert(!c->create_sampler_view(c,r,&desc));
   assert(remesa_context_error(c)==EBUSY&&r->reference.count==3); ++cases;
   c->set_sampler_views(c,PIPE_SHADER_FRAGMENT,0,1,0,views);
   assert(remesa_context_error(c)==EBUSY&&view->reference.count==1); ++cases;
   c->texture_unmap(c,tr);
   c->set_sampler_views(c,PIPE_SHADER_FRAGMENT,0,1,0,views);
   assert(!remesa_context_error(c)&&view->reference.count==2); ++cases;
   unsigned before=f.maps;
   desc.u.tex.last_level=1;
   assert(!c->create_sampler_view(c,r,&desc));
   assert(remesa_context_error(c)==EINVAL&&r->reference.count==2&&f.maps==before); ++cases;
   desc=descriptor(r->format);
   c->set_sampler_views(c,PIPE_SHADER_FRAGMENT,0,0,1,NULL);
   assert(!remesa_context_error(c)&&view->reference.count==1); ++cases;
   pipe_resource_reference(&r,NULL);
   assert(f.allocations==f.frees+1);
   /* Late standard Mesa reference destruction cannot call the freed context. */
   c->destroy(c); other->destroy(other); s->destroy(s);
   pipe_sampler_view_release_ptr(&view);
   assert(!view&&f.allocations==f.frees&&f.maps==f.unmaps&&!f.submits); ++cases;

   s=remesa_screen_create(&d); c=s->context_create(s,NULL,0); assert(s&&c);
   r=resource(s,PIPE_FORMAT_B8G8R8A8_UNORM); assert(r);
   struct pipe_fence_handle *fence=NULL;
   assert(!remesa_resource_attach_writer(c,r,&f,&fence));
   f.wait_error=EIO; tr=NULL;
   assert(!c->texture_map(c,r,0,PIPE_MAP_READ,&box,&tr)&&!tr);
   f.wait_error=0; desc=descriptor(r->format); before=f.maps;
   assert(!c->create_sampler_view(c,r,&desc));
   assert(remesa_context_error(c)==EIO&&f.maps==before); ++cases;
   pipe_resource_reference(&r,NULL); s->fence_reference(s,&fence,NULL);
   c->destroy(c); s->destroy(s);
   assert(f.allocations==f.frees&&f.maps==f.unmaps&&!f.submits&&f.fences==1);
   printf("actual Gallium texture view lifecycle: %u cases PASS; no GPU sampling or execution\n",cases);
   return 0;
}
