/* SPDX-License-Identifier: MIT. Real Gallium callbacks, CPU primitive clipping,
 * packet recording and explicitly SIMULATED completion; no raster pixels/GPU. */
#include "nir_fixture.h"
#include "vertex_nir_fixture.h"
#include "remesa_draw.h"
#define main inherited_recording_driver
#include "test_driver.c"
#undef main
#include <math.h>
static void direct_pack_cases(void)
{
 struct remesa_prepared_triangles *in=calloc(1,sizeof(*in));
 struct remesa_draw_packet *out=malloc(sizeof(*out)),*saved=malloc(sizeof(*saved));assert(in&&out&&saved);
 in->width=in->height=32;in->argb=0xffff00ff;in->vertex_count=6;
 const float positions[6][4]={{8,8,.5,1},{24,8,.5,1},{8,24,.5,1},{8,8,.5,1},{8,24,.5,1},{24,24,.5,1}};
 memcpy(in->positions,positions,sizeof(positions));
 assert(remesa_pack_prepared_triangles(in,out)==REMESA_DRAW_VALIDATED);
 const uint16_t indices[6]={0,1,2,0,2,3};assert(out->vertex_count==4&&out->index_count==6&&!memcmp(out->indices,indices,sizeof(indices)));
 for(unsigned mode=0;mode<8;mode++) {
  memset(out,0x7a,sizeof(*out));*saved=*out;in->width=in->height=32;in->vertex_count=6;memcpy(in->positions,positions,sizeof(positions));
  switch(mode) {
  case 0:in->positions[1][0]=NAN;break;
  case 1:in->vertex_count=REMESA_DRAW_MAX_INDICES+3;break;
  case 2:in->vertex_count=1050;for(unsigned i=0;i<1050;i++){in->positions[i][0]=(float)i/100;in->positions[i][1]=0;in->positions[i][2]=.5;in->positions[i][3]=1;}break;
  case 3:in->width=64;break;
  case 4:in->height=16;break;
  case 5:in->vertex_count=0;break;
  case 6:in->vertex_count=4;break;
  case 7:in->positions[1][3]=2;break;
  }
  enum remesa_draw_status result=remesa_pack_prepared_triangles(in,out);
  assert(mode==5?result==REMESA_DRAW_EMPTY:result!=REMESA_DRAW_VALIDATED);
  assert(!memcmp(out,saved,sizeof(*out)));
 }
 /* Exact and partial aliasing must reject before writing either object. */
 in->width=in->height=32;in->vertex_count=3;memcpy(in->positions,positions,sizeof(positions));
 struct remesa_prepared_triangles *backup=malloc(sizeof(*backup));assert(backup);*backup=*in;
 assert(remesa_pack_prepared_triangles(in,(struct remesa_draw_packet *)in)==REMESA_DRAW_INVALID&&!memcmp(in,backup,sizeof(*in)));
 assert(remesa_pack_prepared_triangles(in,(struct remesa_draw_packet *)((char *)in+16))==REMESA_DRAW_INVALID&&!memcmp(in,backup,sizeof(*in)));
 free(backup);free(saved);free(out);free(in);
}
static void callback_case(unsigned mode)
{
 struct recorder rec={0};struct remesa_device dev={.ops=&ops,.priv=&rec,.offline_test_backend=true,.qualified_hardware_backend=mode==10};
 struct pipe_screen *screen=remesa_screen_create(&dev);assert(screen);struct pipe_context *ctx=screen->context_create(screen,NULL,0);assert(ctx);
 struct pipe_resource *vb=buffer(screen,96,PIPE_BIND_VERTEX_BUFFER),*ib=buffer(screen,6,PIPE_BIND_INDEX_BUFFER);assert(vb&&ib);
 struct pipe_resource rt={.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_B8G8R8A8_UNORM,.width0=32,.height0=32,.depth0=1,.array_size=1,.bind=PIPE_BIND_RENDER_TARGET};
 struct pipe_resource *target=screen->resource_create(screen,&rt);assert(target);
 struct pipe_framebuffer_state fb={.width=32,.height=32,.nr_cbufs=1};fb.cbufs[0].texture=target;fb.cbufs[0].format=target->format;ctx->set_framebuffer_state(ctx,&fb);
 struct pipe_vertex_buffer binding={.buffer.resource=vb};++vb->reference.count;ctx->set_vertex_buffers(ctx,1,&binding);
 struct pipe_vertex_element element={.src_format=PIPE_FORMAT_R32G32B32A32_FLOAT,.src_stride=16};void *ve=ctx->create_vertex_elements_state(ctx,1,&element);assert(ve);ctx->bind_vertex_elements_state(ctx,ve);
 struct pipe_shader_state shader={.type=PIPE_SHADER_IR_NIR};shader.ir.nir=vertex_fixture();void *vs=ctx->create_vs_state(ctx,&shader);assert(vs);ctx->bind_vs_state(ctx,vs);
 shader.ir.nir=fixture(0xffff00ff);void *fs=ctx->create_fs_state(ctx,&shader);assert(fs);ctx->bind_fs_state(ctx,fs);
 struct pipe_blend_state blend={0};blend.rt[0].colormask=PIPE_MASK_RGBA;struct pipe_depth_stencil_alpha_state depth={0};
 struct pipe_rasterizer_state raster={0};raster.fill_front=raster.fill_back=PIPE_POLYGON_MODE_FILL;raster.depth_clip_near=raster.depth_clip_far=true;
 if(mode==9)raster.depth_clip_far=false;
 void *bs=ctx->create_blend_state(ctx,&blend),*ds=ctx->create_depth_stencil_alpha_state(ctx,&depth),*rs=ctx->create_rasterizer_state(ctx,&raster);assert(bs&&ds&&rs);
 ctx->bind_blend_state(ctx,bs);ctx->bind_depth_stencil_alpha_state(ctx,ds);ctx->bind_rasterizer_state(ctx,rs);
 struct pipe_viewport_state viewport={.scale={16,16,.5},.translate={16,16,.5}};ctx->set_viewport_states(ctx,0,1,&viewport);assert(!remesa_context_error(ctx));
 float positions[6][4]={{-2,0,0,1},{0,-.5,0,1},{0,.5,0,1}};
 struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.instance_count=1};struct pipe_draw_start_count_bias draw={.count=3};
 if(mode==1){const float p[3][4]={{-1,-1,0,2},{1,-1,0,2},{-1,1,0,2}};memcpy(positions,p,sizeof(p));}
 if(mode==2){const float p[6][4]={{-.5,-.5,0,1},{.5,-.5,0,1},{-.5,.5,0,1},{-.5,-.5,0,1},{-.5,.5,0,1},{.5,.5,0,1}};memcpy(positions,p,sizeof(p));draw.count=6;}
 if(mode==3)for(unsigned i=0;i<3;i++)positions[i][0]=2;
 if(mode==4)positions[1][0]=NAN;
 if(mode==5){info.index_size=2;info.index.resource=ib;}
 struct pipe_box box={.width=96,.height=1,.depth=1};struct pipe_transfer *transfer=NULL;void *map=ctx->buffer_map(ctx,vb,0,PIPE_MAP_WRITE,&box,&transfer);assert(map&&transfer);memcpy(map,positions,sizeof(positions));ctx->buffer_unmap(ctx,transfer);
 box.width=6;map=ctx->buffer_map(ctx,ib,0,PIPE_MAP_WRITE,&box,&transfer);assert(map&&transfer);const uint16_t indices[]={0,1,999};memcpy(map,indices,sizeof(indices));ctx->buffer_unmap(ctx,transfer);
 if(mode==6)rec.map_mode=1;
 if(mode==7)rec.map_mode=2;
 if(mode==8)ctx->bind_vs_state(ctx,NULL);
 if(mode==11)rec.submit_error=EIO;
 if(mode==12)draw.count=4;
 if(mode==13){shader.ir.nir=fixture(0xffff00ff);assert(!ctx->create_vs_state(ctx,&shader));ctx->bind_vs_state(ctx,NULL);}
 ctx->draw_vbo(ctx,&info,0,NULL,&draw,1);assert(rec.maps==rec.unmaps);
 bool success=mode<3;struct pipe_fence_handle *f=NULL;ctx->flush(ctx,&f,0);
 if(success) {
  assert(!remesa_context_error(ctx)&&rec.draws==1&&f);
  if(mode==0){const float expected[4][4]={{0,20,.5,1},{0,12,.5,1},{16,8,.5,1},{16,24,.5,1}};const uint16_t ix[6]={0,1,2,0,2,3};assert(rec.packet.vertex_count==4&&rec.packet.index_count==6&&!memcmp(rec.packet.positions,expected,sizeof(expected))&&!memcmp(rec.packet.indices,ix,sizeof(ix)));}
  if(mode==1){const float expected[3][4]={{8,8,.5,1},{24,8,.5,1},{8,24,.5,1}};assert(rec.packet.index_count==3&&!memcmp(rec.packet.positions,expected,sizeof(expected)));}
  if(mode==2)assert(rec.packet.index_count==6&&rec.packet.vertex_count==4);
  assert(!screen->fence_finish(screen,ctx,f,0));box=(struct pipe_box){.width=32,.height=32,.depth=1};transfer=NULL;assert(!ctx->texture_map(ctx,target,0,PIPE_MAP_READ,&box,&transfer)&&!transfer);
 } else {assert(!rec.draws&&!f);assert(mode==3?!remesa_context_error(ctx):remesa_context_error(ctx)!=0);}
 ctx->bind_vertex_elements_state(ctx,NULL);ctx->delete_vertex_elements_state(ctx,ve);ctx->delete_vs_state(ctx,vs);ctx->delete_fs_state(ctx,fs);ctx->delete_blend_state(ctx,bs);ctx->delete_depth_stencil_alpha_state(ctx,ds);ctx->delete_rasterizer_state(ctx,rs);ctx->destroy(ctx);
 if(success){assert(!screen->fence_finish(screen,NULL,f,0));rec.last->retired=true;assert(!screen->fence_finish(screen,NULL,f,0));rec.last->target->published=true;assert(screen->fence_finish(screen,NULL,f,0));screen->fence_reference(screen,&f,NULL);}
 resource_drop(vb);resource_drop(ib);resource_drop(target);assert(rec.allocs==rec.frees&&rec.maps==rec.unmaps);screen->destroy(screen);
}
int main(void)
{
 direct_pack_cases();for(unsigned mode=0;mode<14;mode++)callback_case(mode);
 puts("PASS 14 actual Gallium clipping lifecycle scenarios and 11 direct packer cases; CPU preparation, simulated completion, zero SGX/pixels");return 0;
}
