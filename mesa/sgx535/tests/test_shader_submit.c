/* SPDX-License-Identifier: MIT
 * Production Gallium original-program handoff, recording backend only.
 * No GPU encoding, rasterization or synthetic pixels are produced. */
#include "nir_fixture.h"
#include "vertex_nir_fixture.h"
int inherited_shader_submit_fixture(void);
#define main inherited_shader_submit_fixture
#include "test_driver.c"
#undef main

static unsigned checks;
#define CHECK(x) do {assert(x);++checks;} while(0)

static void callback_case(unsigned mode)
{
 struct recorder rec={0};struct remesa_device_ops selected=ops;
 if(mode==1) {
  selected.prepare_shader_programs=NULL;selected.submit_shader_programs=NULL;
  selected.destroy_shader_programs=NULL;
 }
 struct remesa_device device={.ops=&selected,.priv=&rec,.offline_test_backend=true};
 struct pipe_screen *screen=remesa_screen_create(&device);CHECK(screen);
 struct pipe_context *ctx=screen->context_create(screen,NULL,0);CHECK(ctx);
 unsigned vertex_size=mode==8?64:mode==9?76:48;
 struct pipe_resource *vb=buffer(screen,vertex_size,PIPE_BIND_VERTEX_BUFFER);CHECK(vb);
 struct pipe_resource *ib=NULL;
 struct pipe_resource rt={.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_B8G8R8A8_UNORM,
  .width0=32,.height0=32,.depth0=1,.array_size=1,.bind=PIPE_BIND_RENDER_TARGET};
 struct pipe_resource *target=screen->resource_create(screen,&rt);CHECK(target);
 const float positions[3][4]={{-.5,-.5,0,1},{.5,-.5,0,1},{-.5,.5,0,1}};
 struct pipe_box box={.width=vertex_size,.height=1,.depth=1};struct pipe_transfer *transfer=NULL;
 void *data=ctx->buffer_map(ctx,vb,0,PIPE_MAP_WRITE,&box,&transfer);CHECK(data&&transfer);
 memset(data,0xa5,vertex_size);
 unsigned base=mode==8?16:mode==9?8:0,stride=mode==9?24:16,fetch=mode==9?12:16;
 for(unsigned v=0;v<3;v++)memcpy((unsigned char *)data+base+v*stride,positions[v],fetch);
 ctx->buffer_unmap(ctx,transfer);
 struct pipe_framebuffer_state fb={.width=32,.height=32,.nr_cbufs=1};
 fb.cbufs[0].texture=target;fb.cbufs[0].format=target->format;ctx->set_framebuffer_state(ctx,&fb);
 struct pipe_vertex_buffer binding={.buffer.resource=vb,.buffer_offset=mode==8?12:mode==9?4:0};++vb->reference.count;
 ctx->set_vertex_buffers(ctx,1,&binding);
 struct pipe_vertex_element element={.src_format=mode==9?PIPE_FORMAT_R32G32B32_FLOAT:PIPE_FORMAT_R32G32B32A32_FLOAT,
  .src_stride=stride,.src_offset=mode==8||mode==9?4:0};
 void *ve=ctx->create_vertex_elements_state(ctx,1,&element);CHECK(ve);ctx->bind_vertex_elements_state(ctx,ve);
 struct pipe_shader_state shader={.type=PIPE_SHADER_IR_NIR};shader.ir.nir=vertex_fixture();
 void *vs=ctx->create_vs_state(ctx,&shader);CHECK(vs);ctx->bind_vs_state(ctx,vs);
 shader.ir.nir=fixture(0xff00ff00);void *fs=ctx->create_fs_state(ctx,&shader);CHECK(fs);ctx->bind_fs_state(ctx,fs);
 struct pipe_blend_state blend={0};blend.rt[0].colormask=PIPE_MASK_RGBA;
 struct pipe_depth_stencil_alpha_state depth={0};
 struct pipe_rasterizer_state raster={.fill_front=PIPE_POLYGON_MODE_FILL,.fill_back=PIPE_POLYGON_MODE_FILL};
 if(mode==9){raster.scissor=true;raster.depth_clip_near=true;raster.front_ccw=true;}
 void *bs=ctx->create_blend_state(ctx,&blend),*ds=ctx->create_depth_stencil_alpha_state(ctx,&depth),
      *rs=ctx->create_rasterizer_state(ctx,&raster);CHECK(bs&&ds&&rs);
 ctx->bind_blend_state(ctx,bs);ctx->bind_depth_stencil_alpha_state(ctx,ds);ctx->bind_rasterizer_state(ctx,rs);
 struct pipe_viewport_state viewport={.scale={16,16,.5},.translate={16,16,.5}};
 ctx->set_viewport_states(ctx,0,1,&viewport);CHECK(!remesa_context_error(ctx));
 if(mode==9){struct pipe_scissor_state scissor={.maxx=32,.maxy=32};ctx->set_scissor_states(ctx,0,1,&scissor);CHECK(!remesa_context_error(ctx));}
 /* Exercise real Mesa sampler defaults through the production callbacks. */
 struct pipe_sampler_state sampler={.wrap_s=PIPE_TEX_WRAP_CLAMP_TO_EDGE,.wrap_t=PIPE_TEX_WRAP_CLAMP_TO_EDGE,
  .wrap_r=PIPE_TEX_WRAP_CLAMP_TO_EDGE,.min_img_filter=PIPE_TEX_FILTER_NEAREST,
  .mag_img_filter=PIPE_TEX_FILTER_NEAREST,.min_mip_filter=PIPE_TEX_MIPFILTER_NONE,
  .max_lod=1000,.compare_mode=PIPE_TEX_COMPARE_NONE,.compare_func=PIPE_FUNC_LEQUAL};
 void *ss=ctx->create_sampler_state(ctx,&sampler);CHECK(ss);
 ctx->bind_sampler_states(ctx,PIPE_SHADER_FRAGMENT,0,1,&ss);CHECK(!remesa_context_error(ctx));
 void *empty=NULL;ctx->bind_sampler_states(ctx,PIPE_SHADER_FRAGMENT,0,1,&empty);ctx->delete_sampler_state(ctx,ss);
 if(mode==2)rec.shader_prepare_error=ENOTSUP;
 if(mode==3)rec.shader_prepare_null=true;
 if(mode==4)rec.map_mode=2;
 if(mode==5){rec.submit_error=EIO;rec.error_fence=true;}
 if(mode==6)ctx->bind_vs_state(ctx,NULL);
 struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.instance_count=1};
 struct pipe_draw_start_count_bias draw={.count=mode==7?2:3};
 const uint16_t indices[]={99,1,2,3};
 if(mode==8) {
  ib=buffer(screen,sizeof(indices),PIPE_BIND_INDEX_BUFFER);CHECK(ib);
  box=(struct pipe_box){.width=sizeof(indices),.height=1,.depth=1};
  data=ctx->buffer_map(ctx,ib,0,PIPE_MAP_WRITE,&box,&transfer);CHECK(data&&transfer);
  memcpy(data,indices,sizeof(indices));ctx->buffer_unmap(ctx,transfer);
  info.index_size=2;info.index.resource=ib;draw.start=1;draw.index_bias=-1;
 }
 unsigned maps=rec.maps;
 ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);
 CHECK(rec.maps==rec.unmaps);
 struct pipe_fence_handle *f=NULL;ctx->flush(ctx,&f,0);
 if(mode==0||mode==8||mode==9) {
  CHECK(!remesa_context_error(ctx)&&rec.shader_prepares==1&&rec.shader_submits==1&&rec.draws==1&&f);
  CHECK(rec.shader_allocs==1&&rec.shader_frees==0&&rec.last->programs);
  CHECK(rec.packet.argb==0xff00ff00&&rec.packet.index_count==3);
  const struct remesa_shader_draw_inputs *raw=&rec.last->programs->inputs;
  struct remesa_draw_plan raw_plan;
  CHECK(!remesa_shader_draw_inputs_validate(raw,&raw_plan));
  CHECK(raw_plan.primitive_count==1&&raw_plan.min_vertex==0&&raw_plan.max_vertex==2&&
        raw_plan.vertex_end_byte==base+2*stride+fetch);
  CHECK(raw->vertex_bytes==vertex_size&&raw->vertex_offset==binding.buffer_offset&&
        raw->element_offset==element.src_offset&&raw->vertex_stride==stride);
  CHECK(raw->position_format==element.src_format&&raw->viewport.scale[0]==16&&raw->viewport.translate[0]==16);
  for(unsigned v=0;v<3;v++)CHECK(!memcmp((const unsigned char *)raw->vertex_data+base+v*stride,positions[v],fetch));
  CHECK(rec.packet.positions[0][0]==8&&positions[0][0]==-.5f);
  CHECK(raw->index_size==info.index_size&&raw->start==draw.start&&raw->count==3&&raw->index_bias==draw.index_bias);
  if(mode==8)CHECK(raw->index_bytes==sizeof(indices)&&!memcmp(raw->index_data,indices,sizeof(indices)));
  if(mode==9)CHECK(raw->scissor_enabled&&raw->scissor.maxx==32&&raw->scissor.maxy==32&&
                  raw->depth_clip_near&&!raw->depth_clip_far&&raw->front_ccw);
  /* Destroy the source shader objects before fence retirement. The backend
   * token owns complete immutable programs beyond context/NIR lifetime. */
  ctx->delete_vs_state(ctx,vs);vs=NULL;ctx->delete_fs_state(ctx,fs);fs=NULL;
  CHECK(!remesa_shader_draw_programs_validate(&rec.last->programs->snapshot));
  CHECK(!screen->fence_finish(screen,ctx,f,0));
 } else {
  int expected=mode==1||mode==2||mode==6?ENOTSUP:mode==3?EPROTO:mode==4||mode==5?EIO:0;
  CHECK(remesa_context_error(ctx)==expected&&!f&&!rec.draws);
  CHECK(rec.shader_allocs==rec.shader_frees);
  if(mode==1||mode==6||mode==7)CHECK(!rec.shader_prepares&&!rec.shader_submits&&rec.maps==maps);
  if(mode==2||mode==3)CHECK(rec.shader_prepares==1&&!rec.shader_submits&&rec.maps==maps);
  if(mode==4)CHECK(rec.shader_prepares==1&&!rec.shader_submits);
  if(mode==5)CHECK(rec.shader_submits==1&&rec.fence_destroys==1);
 }
 if(vs)ctx->delete_vs_state(ctx,vs);
 if(fs)ctx->delete_fs_state(ctx,fs);
 ctx->delete_vertex_elements_state(ctx,ve);ctx->delete_blend_state(ctx,bs);
 ctx->delete_depth_stencil_alpha_state(ctx,ds);ctx->delete_rasterizer_state(ctx,rs);
 ctx->destroy(ctx);
 if(f) {
  CHECK(!remesa_shader_draw_programs_validate(&rec.last->programs->snapshot));
  CHECK(!memcmp((const unsigned char *)rec.last->programs->inputs.vertex_data+base,positions[0],fetch));
  rec.last->retired=true;rec.last->target->published=true;
  CHECK(screen->fence_finish(screen,NULL,f,0));screen->fence_reference(screen,&f,NULL);
 }
 resource_drop(vb);resource_drop(target);if(ib)resource_drop(ib);
 CHECK(rec.allocs==rec.frees&&rec.shader_allocs==rec.shader_frees);screen->destroy(screen);
}

static void partial_backend_cases(void)
{
 for(unsigned n=0;n<3;n++) {
  struct recorder rec={0};struct remesa_device_ops selected=ops;
  if(!n)selected.prepare_shader_programs=NULL;
  if(n==1)selected.submit_shader_programs=NULL;
  if(n==2)selected.destroy_shader_programs=NULL;
  struct remesa_device device={.ops=&selected,.priv=&rec,.offline_test_backend=true};
  CHECK(!remesa_screen_create(&device)&&!rec.allocs&&!rec.shader_prepares&&!rec.draws);
 }
}

static void framebuffer_rebind(void)
{
 struct recorder rec={0};struct remesa_device device={.ops=&ops,.priv=&rec,.offline_test_backend=true};
 struct pipe_screen *screen=remesa_screen_create(&device);CHECK(screen);
 struct pipe_context *ctx=screen->context_create(screen,NULL,0);CHECK(ctx);
 struct pipe_resource rt={.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_B8G8R8A8_UNORM,
  .width0=32,.height0=32,.depth0=1,.array_size=1,.bind=PIPE_BIND_RENDER_TARGET};
 struct pipe_resource *target=screen->resource_create(screen,&rt);CHECK(target);
 struct pipe_framebuffer_state fb={.width=32,.height=32,.nr_cbufs=1};
 fb.cbufs[0].texture=target;fb.cbufs[0].format=target->format;ctx->set_framebuffer_state(ctx,&fb);
 resource_drop(target); /* Context owns the remaining reference. */
 CHECK(target->reference.count==1&&!rec.frees);
 ctx->set_framebuffer_state(ctx,&fb);CHECK(!remesa_context_error(ctx)&&!rec.frees&&target->reference.count==1);
 struct pipe_framebuffer_state empty={0};ctx->set_framebuffer_state(ctx,&empty);CHECK(rec.frees==1);
 ctx->destroy(ctx);CHECK(rec.allocs==rec.frees);screen->destroy(screen);
}

int main(void)
{
 for(unsigned n=0;n<10;n++)callback_case(n);
 partial_backend_cases();framebuffer_rebind();
 printf("PASS %u original-shader Gallium handoff checks; recording only, zero GPU submissions\n",checks);
 return 0;
}
