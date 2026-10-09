/* SPDX-License-Identifier: MIT; actual Mesa NIR structures, zero GPU. */
#include "remesa_nir.h"
#include "vertex_nir_fixture.h"
#define main previous_fragment_tests
#include "test_nir.c"
#undef main
#include <errno.h>
#include <stdio.h>
int main(void)
{
 unsigned cases=0,slot=99;
 assert(remesa_nir_position_vertex(NULL,&slot)==EINVAL&&slot==99);cases++;
 nir_shader *s=vertex_fixture();assert(!remesa_nir_position_vertex(s,&slot)&&slot==0);ralloc_free(s);cases++;
 for(unsigned mode=0;mode<4;mode++) {
  s=vertex_fixture();slot=99;
  nir_function *fn=exec_node_data(nir_function,exec_list_get_head(&s->functions),node);
  nir_block *b=exec_node_data(nir_block,exec_list_get_head(&fn->impl->body),cf_node.node);
  nir_instr *first=exec_node_data(nir_instr,exec_list_get_head(&b->instr_list),node);
  nir_intrinsic_instr *input=exec_node_data(nir_intrinsic_instr,first->node.next,instr.node);
  nir_intrinsic_set_io_semantics(input,(nir_io_semantics){.location=VERT_ATTRIB_POS,.num_slots=1});
  s->info.inputs_read=mode==0?0:mode==1?BITFIELD64_BIT(VERT_ATTRIB_POS):
                       mode==2?BITFIELD64_BIT(VERT_ATTRIB_GENERIC0):
                       BITFIELD64_BIT(VERT_ATTRIB_POS)|BITFIELD64_BIT(VERT_ATTRIB_GENERIC0);
  int ret=remesa_nir_position_vertex(s,&slot);
  assert(mode<2?(!ret&&slot==0):(ret&&slot==99));ralloc_free(s);cases++;
 }
 for(unsigned mode=0;mode<21;mode++) {
  s=vertex_fixture();slot=99;
  nir_function *fn=exec_node_data(nir_function,exec_list_get_head(&s->functions),node);
  nir_block *b=exec_node_data(nir_block,exec_list_get_head(&fn->impl->body),cf_node.node);
  nir_load_const_instr *zero=exec_node_data(nir_load_const_instr,exec_list_get_head(&b->instr_list),instr.node);
  nir_intrinsic_instr *input=exec_node_data(nir_intrinsic_instr,zero->instr.node.next,instr.node);
  nir_intrinsic_instr *store=exec_node_data(nir_intrinsic_instr,exec_list_get_tail(&b->instr_list),instr.node);
  switch(mode) {
   case 0:s->info.stage=MESA_SHADER_FRAGMENT;break;
   case 1:input->intrinsic=nir_intrinsic_load_vertex_id;break;
   case 2:input->def.bit_size=16;break;
   case 3:input->def.num_components=3;break;
   case 4:nir_intrinsic_set_component(input,1);break;
   case 5:nir_intrinsic_set_base(input,1);break;
   case 6:nir_intrinsic_set_dest_type(input,nir_type_int32);break;
   case 7:nir_intrinsic_set_io_semantics(input,(nir_io_semantics){.location=VERT_ATTRIB_GENERIC1,.num_slots=1});break;
   case 8:nir_intrinsic_set_io_semantics(store,(nir_io_semantics){.location=VARYING_SLOT_COL0,.num_slots=1});break;
   case 9:nir_intrinsic_set_write_mask(store,7);break;
   case 10:zero->value[0].u32=1;break;
   case 11:store->src[0].ssa=&zero->def;break;
   case 12:s->info.io_lowered=false;break;
   case 13:fn->num_params=1;break;
   case 14:fn->is_entrypoint=false;break;
   case 15:s->num_uniforms=1;break;
   case 16:s->info.writes_memory=true;break;
   case 17:s->info.outputs_written=BITFIELD64_BIT(VARYING_SLOT_COL0);break;
   case 18:s->info.inputs_read=BITFIELD64_BIT(VERT_ATTRIB_GENERIC1);break;
   case 19:input->instr.block=NULL;break;
   case 20:nir_intrinsic_set_io_xfb(store,(nir_io_xfb){.out[0].num_components=4});break;
  }
  assert(remesa_nir_position_vertex(s,&slot)!=0&&slot==99);ralloc_free(s);cases++;
 }
 static const struct remesa_device_ops ops={alloc,release,map,unmap,submit,wait,release};
 struct remesa_device dev={.ops=&ops,.offline_test_backend=true};struct pipe_screen *screen=remesa_screen_create(&dev);assert(screen);
 struct pipe_context *ctx=screen->context_create(screen,NULL,0);assert(ctx);
 struct pipe_shader_state state={.type=PIPE_SHADER_IR_NIR};state.ir.nir=vertex_fixture();ralloc_set_destructor(state.ir.nir,destroy_marker);
 unsigned before=destroyed;fail_next_calloc=true;assert(!ctx->create_vs_state(ctx,&state)&&remesa_context_error(ctx)==ENOMEM&&destroyed==before+1);cases++;
 state.ir.nir=vertex_fixture();state.stream_output.num_outputs=1;ralloc_set_destructor(state.ir.nir,destroy_marker);before=destroyed;
 assert(!ctx->create_vs_state(ctx,&state)&&remesa_context_error(ctx)==ENOTSUP&&destroyed==before+1);cases++;
 state.stream_output.num_outputs=0;state.ir.nir=vertex_fixture();ralloc_set_destructor(state.ir.nir,destroy_marker);before=destroyed;
 void *vs=ctx->create_vs_state(ctx,&state);assert(vs&&destroyed==before+1);ctx->bind_vs_state(ctx,vs);assert(!remesa_context_error(ctx));cases++;
 ctx->delete_vs_state(ctx,vs);ctx->delete_vs_state(ctx,vs);assert(remesa_context_error(ctx)==EINVAL);cases++;
 ctx->destroy(ctx);screen->destroy(screen);
 printf("PASS %u restricted vertex NIR admission/ownership cases; no GPU\n",cases);return 0;
}
