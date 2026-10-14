/* SPDX-License-Identifier: MIT; actual Gallium CPU transfers, no GPU work. */
int existing_single_level_tests(void);
#define main existing_single_level_tests
#include "test_transfer_resources.c"
#undef main
#include "util/u_inlines.h"
#ifdef REMESA_TEST_MESA_UPLOAD
#include "util/u_transfer.h"
#endif
static struct pipe_resource *mips(struct pipe_screen *s,unsigned w,unsigned h,unsigned last,unsigned format)
{
 struct pipe_resource t={.target=PIPE_TEXTURE_2D,.format=format,
  .width0=w,.height0=h,.depth0=1,.array_size=1,.last_level=last};
 return s->resource_create(s,&t);
}
static unsigned last_level(unsigned w,unsigned h)
{unsigned n=w>h?w:h,last=0;while(n>1){n>>=1;++last;}return last;}
static unsigned minify(unsigned x,unsigned level){x>>=level;return x?x:1;}
int main(void)
{
 const unsigned sizes[][2]={{1,1},{7,5},{1,31},{31,1},{32,32},{320,240},
  {1280,800},{1366,768},{1920,1080}};
 struct fixture f={0};const struct remesa_device_ops ops={.allocate=allocate,.destroy_bo=drop,.map=map,.unmap=unmap,.submit=submit,.wait=wait,.destroy_fence=destroy_fence};
 struct remesa_device dev={.ops=&ops,.priv=&f,.resource_only_backend=true,
  .resource_limits={.max_width=4096,.max_height=4096,.row_alignment=64,.max_resource_bytes=64*1024*1024}};
 struct pipe_screen *s=remesa_screen_create(&dev);assert(s);
 struct pipe_context *c=s->context_create(s,NULL,0),*other=s->context_create(s,NULL,0);assert(c&&other);
 unsigned level_cases=0;
 for(unsigned dimension=0;dimension<sizeof(sizes)/sizeof(sizes[0]);dimension++)for(unsigned format=0;format<2;format++) {
  unsigned w=sizes[dimension][0],h=sizes[dimension][1],last=last_level(w,h);
  struct pipe_resource *r=mips(s,w,h,last,format?PIPE_FORMAT_R8G8B8A8_UNORM:PIPE_FORMAT_B8G8R8A8_UNORM);assert(r);
  size_t offset=0;uintptr_t base=0;
  for(unsigned level=0;level<=last;level++) {
   unsigned mw=minify(w,level),mh=minify(h,level),stride=(mw*4+63)&~63u;
   struct pipe_box box={.width=mw,.height=mh,.depth=1};struct pipe_transfer *t=NULL;
   unsigned char *p=c->texture_map(c,r,level,PIPE_MAP_WRITE,&box,&t);assert(p&&t->level==level);
   assert(t->stride==stride&&t->layer_stride==(size_t)stride*mh);
   if(!level)base=(uintptr_t)p;
   assert((uintptr_t)p==base+offset);offset+=(size_t)stride*mh;
   memset(p,0xcd,(size_t)stride*mh);c->texture_unmap(c,t);
   unsigned input_stride=mw*4+8;unsigned char *input=malloc((size_t)input_stride*mh);assert(input);
   memset(input,level+1,(size_t)input_stride*mh);
#ifdef REMESA_TEST_MESA_UPLOAD
   unsigned maps=f.maps;
   u_default_texture_subdata(c,r,level,0,&box,input,input_stride,(size_t)input_stride*mh);
   assert(f.maps==maps+1);
#else
   c->texture_subdata(c,r,level,PIPE_MAP_DISCARD_RANGE,&box,input,input_stride,(size_t)input_stride*mh);
   assert(!remesa_context_error(c));
#endif
   free(input);
   ++level_cases;
  }
  /* Check every level only after all uploads, so offsets cannot alias. */
  for(unsigned level=0;level<=last;level++) {
   unsigned mw=minify(w,level),mh=minify(h,level),stride=(mw*4+63)&~63u;
   struct pipe_box all={.width=mw,.height=mh,.depth=1};struct pipe_transfer *t=NULL;
   unsigned char *p=c->texture_map(c,r,level,PIPE_MAP_READ,&all,&t);assert(p);
   for(unsigned y=0;y<mh;y++)for(unsigned x=0;x<stride;x++)assert(p[(size_t)y*stride+x]==(x<mw*4?level+1:0xcd));
   c->texture_unmap(c,t);
   struct pipe_box corner={.x=mw-1,.y=mh-1,.width=1,.height=1,.depth=1};unsigned char pixel[4]={3,5,7,9};
   c->texture_subdata(c,r,level,0,&corner,pixel,4,4);assert(!remesa_context_error(c));
   p=c->texture_map(c,r,level,PIPE_MAP_READ,&corner,&t);assert(p&&!memcmp(p,pixel,4));c->texture_unmap(c,t);
   corner.width=2;unsigned maps=f.maps;
   assert(!c->texture_map(c,r,level,PIPE_MAP_READ,&corner,&t)&&!t&&f.maps==maps);
   c->texture_subdata(c,r,level,0,&corner,pixel,8,8);assert(remesa_context_error(c)==EINVAL&&f.maps==maps);
  }
  struct pipe_box one={.width=1,.height=1,.depth=1};struct pipe_transfer *t=NULL;
  unsigned maps=f.maps;assert(!c->texture_map(c,r,last+1,PIPE_MAP_READ,&one,&t)&&!t&&f.maps==maps);
  c->clear_texture(c,r,last+1,&one,"abcd");assert(remesa_context_error(c)==EINVAL&&f.maps==maps);
  release(r);
 }
 struct pipe_resource *r=mips(s,15,9,3,PIPE_FORMAT_B8G8R8A8_UNORM);assert(r);
 struct pipe_box region={.x=1,.y=1,.width=4,.height=3,.depth=1};struct pipe_transfer *t=NULL;
 unsigned char *p=c->texture_map(c,r,1,PIPE_MAP_WRITE|PIPE_MAP_FLUSH_EXPLICIT,&region,&t);assert(p&&t->stride==64);
 for(unsigned y=0;y<3;y++)memset(p+y*64,0xa7,16);
 struct pipe_box flushed={.x=1,.y=1,.width=2,.height=2,.depth=1};
 c->transfer_flush_region(c,t,&flushed);assert(!remesa_context_error(c));
 /* A lease on level1 must exclude a map/imported writer on every level. */
 struct pipe_transfer *foreign=NULL;struct pipe_box one={.width=1,.height=1,.depth=1};
 assert(!other->texture_map(other,r,0,PIPE_MAP_READ,&one,&foreign)&&!foreign);
 c->texture_unmap(c,t);
 struct pipe_box all={.width=7,.height=4,.depth=1};p=c->texture_map(c,r,1,PIPE_MAP_READ,&all,&t);assert(p);
 for(unsigned y=0;y<4;y++)for(unsigned x=0;x<64;x++)assert(p[y*64+x]==(y>=2&&x>=8&&x<16?0xa7:0));
 c->texture_unmap(c,t);
 /* In-place cross-level copy snapshots before mapping the destination. */
 struct pipe_box source={.x=2,.y=2,.width=2,.height=2,.depth=1};
 c->resource_copy_region(c,r,2,1,0,0,r,1,&source);assert(!remesa_context_error(c));
 struct pipe_box copied={.x=1,.width=2,.height=2,.depth=1};p=c->texture_map(c,r,2,PIPE_MAP_READ,&copied,&t);assert(p);
 for(unsigned y=0;y<2;y++)for(unsigned x=0;x<8;x++)assert(p[y*t->stride+x]==0xa7);
 c->texture_unmap(c,t);
 unsigned char color[4]={1,2,3,4};c->clear_texture(c,r,2,&copied,color);assert(!remesa_context_error(c));
 p=c->texture_map(c,r,2,PIPE_MAP_READ,&copied,&t);assert(p);
 for(unsigned y=0;y<2;y++)for(unsigned x=0;x<2;x++)assert(!memcmp(p+y*t->stride+x*4,color,4));
 c->texture_unmap(c,t);
 /* Invalid levels, levels beyond1x1, arrays, cubes and render mipmaps fail
  * without allocation. Whole-chain budget includes every aligned level. */
 unsigned allocations=f.allocations;
 assert(!mips(s,1,1,1,PIPE_FORMAT_B8G8R8A8_UNORM));
 assert(!mips(s,7,5,3,PIPE_FORMAT_B8G8R8A8_UNORM));
 assert(!mips(s,4,4,32,PIPE_FORMAT_B8G8R8A8_UNORM));
 assert(!mips(s,4,4,2,PIPE_FORMAT_R8G8B8A8_SRGB));
 assert(!mips(s,4,4,2,PIPE_FORMAT_DXT1_RGBA));
 struct pipe_resource templ={.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_B8G8R8A8_UNORM,
  .width0=32,.height0=32,.array_size=1,.depth0=1,.last_level=1,.bind=PIPE_BIND_RENDER_TARGET};
 assert(!s->resource_create(s,&templ));templ.bind=0;templ.array_size=2;assert(!s->resource_create(s,&templ));
 templ.array_size=1;templ.target=PIPE_TEXTURE_CUBE;assert(!s->resource_create(s,&templ));
 assert(!mips(s,4096,4096,12,PIPE_FORMAT_R8G8B8A8_UNORM));assert(f.allocations==allocations);
 struct remesa_texture_layout untouched={.count=77,.bytes=99};
 struct remesa_resource_limits limits={.max_width=UINT32_MAX,.max_height=UINT32_MAX,.row_alignment=64,.max_resource_bytes=UINT64_MAX};
 assert(remesa_texture_layout(&limits,UINT32_MAX,2,4,1,&untouched)==EOVERFLOW);
 assert(remesa_texture_layout(&limits,65536,15000,4,1,&untouched)==EOVERFLOW);
 assert(untouched.count==77&&untouched.bytes==99);
 limits=(struct remesa_resource_limits){.max_width=4,.max_height=4,.row_alignment=64,.max_resource_bytes=448};
 assert(!remesa_texture_layout(&limits,4,4,4,2,&untouched));assert(untouched.bytes==448&&untouched.pages==1);
 limits.max_resource_bytes=447;assert(remesa_texture_layout(&limits,4,4,4,2,&untouched)==ENOMEM&&untouched.bytes==448);
 struct pipe_sampler_view view={.target=PIPE_TEXTURE_2D,.format=r->format,
  .swizzle_r=PIPE_SWIZZLE_X,.swizzle_g=PIPE_SWIZZLE_Y,.swizzle_b=PIPE_SWIZZLE_Z,.swizzle_a=PIPE_SWIZZLE_W};
 struct pipe_sampler_view *mip_view=c->create_sampler_view(c,r,&view);assert(mip_view);
 c->set_sampler_views(c,PIPE_SHADER_FRAGMENT,0,1,0,&mip_view);
 assert(!remesa_context_error(c));
 c->set_sampler_views(c,PIPE_SHADER_FRAGMENT,0,0,1,NULL);
 pipe_sampler_view_release_ptr(&mip_view);
 /* Mip metadata does not enable a sampler-bound allocation or GPU sampling. */
 templ=(struct pipe_resource){.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_B8G8R8A8_UNORM,
  .width0=4,.height0=4,.depth0=1,.array_size=1,.last_level=2,.bind=PIPE_BIND_SAMPLER_VIEW};
 assert(!s->resource_create(s,&templ));
 f.partial=true;assert(!mips(s,4,4,2,PIPE_FORMAT_B8G8R8A8_UNORM));f.partial=false;
 /* Imported writer visibility and context destruction cover the entire BO. */
 struct pipe_fence_handle *fence=NULL;assert(!remesa_resource_attach_writer(c,r,&f,&fence));
 f.error=EAGAIN;unsigned maps=f.maps;assert(!c->texture_map(c,r,3,PIPE_MAP_READ|PIPE_MAP_DONTBLOCK,&one,&t)&&f.maps==maps);
 f.error=0;p=c->texture_map(c,r,3,PIPE_MAP_WRITE|PIPE_MAP_FLUSH_EXPLICIT,&one,&t);assert(p);memset(p,0xff,4);
 release(r);c->destroy(c);s->fence_reference(s,&fence,NULL);other->destroy(other);s->destroy(s);
 assert(f.allocations==f.frees&&f.maps==f.unmaps&&!f.submits);
 printf("PASS %u mip-level RGBA/BGRA cases, level isolation, padded/partial/explicit transfers, copies, CPU clears, bounds/budget/fences/teardown; zero SGX\n",level_cases);
 return 0;
}
