/* SPDX-License-Identifier: MIT
 * Actual NIR/Gallium callbacks. Packet recording and simulated retirement only;
 * no rasterizer, generated framebuffer, physical completion or SGX invocation. */
#include "nir_fixture.h"
#include "vertex_nir_fixture.h"
#define main inherited_driver_main
#include "test_driver.c"
#undef main
#ifdef NDEBUG
#error Callback tests require assertions
#endif
int main(void)
{
 const enum pipe_format formats[]={PIPE_FORMAT_R32G32_FLOAT,
  PIPE_FORMAT_R32G32B32_FLOAT,PIPE_FORMAT_R32G32B32A32_FLOAT};
 const float clip[3][4]={{-.5f,-.5f,0,1},{.5f,-.5f,0,1},{-.5f,.5f,0,1}};
 const float expected[3][4]={{8,8,.5f,1},{24,8,.5f,1},{8,24,.5f,1}};
 for(unsigned format=0;format<3;format++) {
  struct recorder rec={0};struct remesa_device dev={.ops=&ops,.priv=&rec,.offline_test_backend=true};
  struct pipe_screen *s=remesa_screen_create(&dev);assert(s);
  assert(s->is_format_supported(s,formats[format],PIPE_BUFFER,0,0,PIPE_BIND_VERTEX_BUFFER));
  struct pipe_context *c=s->context_create(s,NULL,PIPE_CONTEXT_PREFER_THREADED);assert(c);
  unsigned bytes=8+4*format;
  struct pipe_resource *vb=buffer(s,3*bytes,PIPE_BIND_VERTEX_BUFFER);assert(vb);
  unsigned char packed[48];for(unsigned i=0;i<3;i++)memcpy(packed+i*bytes,clip[i],bytes);
  c->buffer_subdata(c,vb,0,0,3*bytes,packed);assert(!remesa_context_error(c));
  struct pipe_vertex_buffer v={.buffer.resource=vb};++vb->reference.count;c->set_vertex_buffers(c,1,&v);
  struct pipe_vertex_element e={.src_format=formats[format],.src_stride=bytes};
  void *elements=c->create_vertex_elements_state(c,1,&e);assert(elements);c->bind_vertex_elements_state(c,elements);
  struct pipe_resource rt={.target=PIPE_TEXTURE_2D,.format=PIPE_FORMAT_B8G8R8A8_UNORM,
   .width0=32,.height0=32,.depth0=1,.array_size=1,.bind=PIPE_BIND_RENDER_TARGET};
  struct pipe_resource *target=s->resource_create(s,&rt);assert(target);
  struct pipe_framebuffer_state fb={.width=32,.height=32,.nr_cbufs=1};
  fb.cbufs[0].texture=target;fb.cbufs[0].format=target->format;c->set_framebuffer_state(c,&fb);
  nir_shader *nir=vertex_fixture();nir_function_impl *impl=nir_shader_get_entrypoint(nir);
  nir_block *block=nir_cf_node_as_block(exec_node_data(nir_cf_node,exec_list_get_head(&impl->body),node));
  nir_foreach_instr(instr,block)if(instr->type==nir_instr_type_intrinsic) {
   nir_intrinsic_instr *io=nir_instr_as_intrinsic(instr);
   if(io->intrinsic==nir_intrinsic_load_input)
    nir_intrinsic_set_io_semantics(io,(nir_io_semantics){.location=VERT_ATTRIB_POS,.num_slots=1});
  }
  nir->info.inputs_read=BITFIELD64_BIT(VERT_ATTRIB_POS);
  struct pipe_shader_state shader={.type=PIPE_SHADER_IR_NIR,.ir.nir=nir};
  void *vs=c->create_vs_state(c,&shader);assert(vs);c->bind_vs_state(c,vs);
  shader.ir.nir=fixture(0xffff00ff);void *fs=c->create_fs_state(c,&shader);assert(fs);c->bind_fs_state(c,fs);
  struct pipe_blend_state blend={0};blend.rt[0].colormask=PIPE_MASK_RGBA;
  struct pipe_depth_stencil_alpha_state depth={0};struct pipe_rasterizer_state raster={0};
  raster.fill_front=raster.fill_back=PIPE_POLYGON_MODE_FILL;
  void *bs=c->create_blend_state(c,&blend),*ds=c->create_depth_stencil_alpha_state(c,&depth),*rs=c->create_rasterizer_state(c,&raster);
  assert(bs&&ds&&rs);c->bind_blend_state(c,bs);c->bind_depth_stencil_alpha_state(c,ds);c->bind_rasterizer_state(c,rs);
  struct pipe_viewport_state viewport={.scale={16,16,.5f},.translate={16,16,.5f}};c->set_viewport_states(c,0,1,&viewport);
  struct pipe_draw_info info={.mode=MESA_PRIM_TRIANGLES,.instance_count=1};
  struct pipe_draw_start_count_bias draw={.count=3};c->draw_vbo(c,&info,0,NULL,&draw,1);
  assert(!remesa_context_error(c)&&rec.draws==1&&!memcmp(rec.packet.positions,expected,sizeof(expected)));
  struct pipe_fence_handle *f=NULL;c->flush(c,&f,0);assert(f&&!s->fence_finish(s,c,f,0));
  rec.last->retired=true;rec.last->target->published=true; /* Simulated fixture evidence. */
  assert(s->fence_finish(s,c,f,0));s->fence_reference(s,&f,NULL);
  c->destroy(c);resource_drop(vb);resource_drop(target);s->destroy(s);
  assert(rec.allocs==rec.frees&&rec.maps==rec.unmaps&&rec.fence_destroys==1);
 }
 puts("PASS 3 float position formats through compatibility NIR, state binding, draw_vbo and cleanup; packet recording, zero SGX");
 return 0;
}
