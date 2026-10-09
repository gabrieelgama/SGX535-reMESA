/* SPDX-License-Identifier: MIT; manually constructed Mesa NIR, offline only. */
#include "nir.h"
#include "util/ralloc.h"
#include <assert.h>
static nir_shader *vertex_fixture(void)
{
 nir_shader *s=rzalloc(NULL,nir_shader);assert(s);
 exec_list_make_empty(&s->variables);exec_list_make_empty(&s->functions);
 s->info.stage=MESA_SHADER_VERTEX;s->info.io_lowered=true;
 nir_function *fn=rzalloc(s,nir_function);fn->shader=s;fn->is_entrypoint=true;exec_list_push_tail(&s->functions,&fn->node);
 nir_function_impl *impl=rzalloc(s,nir_function_impl);fn->impl=impl;impl->function=fn;impl->structured=true;
 exec_list_make_empty(&impl->body);exec_list_make_empty(&impl->locals);
 impl->end_block=rzalloc(s,nir_block);exec_list_make_empty(&impl->end_block->instr_list);
 nir_block *b=rzalloc(s,nir_block);b->cf_node.type=nir_cf_node_block;b->cf_node.parent=&impl->cf_node;
 exec_list_push_tail(&impl->body,&b->cf_node.node);exec_list_make_empty(&b->instr_list);
 nir_load_const_instr *zero=rzalloc_size(s,sizeof(*zero)+sizeof(nir_const_value));zero->instr.type=nir_instr_type_load_const;zero->instr.block=b;zero->def.parent_instr=&zero->instr;zero->def.bit_size=32;zero->def.num_components=1;exec_list_push_tail(&b->instr_list,&zero->instr.node);
 nir_intrinsic_instr *input=rzalloc_size(s,sizeof(*input)+sizeof(nir_src));input->instr.type=nir_instr_type_intrinsic;input->instr.block=b;input->intrinsic=nir_intrinsic_load_input;input->num_components=4;input->src[0].ssa=&zero->def;input->def.parent_instr=&input->instr;input->def.bit_size=32;input->def.num_components=4;
 nir_intrinsic_set_dest_type(input,nir_type_float32);nir_intrinsic_set_io_semantics(input,(nir_io_semantics){.location=VERT_ATTRIB_GENERIC0,.num_slots=1});exec_list_push_tail(&b->instr_list,&input->instr.node);
 nir_intrinsic_instr *store=rzalloc_size(s,sizeof(*store)+2*sizeof(nir_src));store->instr.type=nir_instr_type_intrinsic;store->instr.block=b;store->intrinsic=nir_intrinsic_store_output;store->num_components=4;store->src[0].ssa=&input->def;store->src[1].ssa=&zero->def;
 nir_intrinsic_set_write_mask(store,15);nir_intrinsic_set_src_type(store,nir_type_float32);nir_intrinsic_set_io_semantics(store,(nir_io_semantics){.location=VARYING_SLOT_POS,.num_slots=1});exec_list_push_tail(&b->instr_list,&store->instr.node);return s;
}
