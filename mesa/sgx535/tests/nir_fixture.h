/* SPDX-License-Identifier: MIT; manually constructed actual Mesa NIR. */
#include "nir.h"
#include "util/ralloc.h"
#include <assert.h>
static nir_shader *fixture(uint32_t color)
{
 nir_shader *s=rzalloc(NULL,nir_shader);assert(s);
 exec_list_make_empty(&s->variables);exec_list_make_empty(&s->functions);
 s->info.stage=MESA_SHADER_FRAGMENT;s->info.io_lowered=true;
 nir_function *fn=rzalloc(s,nir_function);fn->shader=s;fn->is_entrypoint=true;exec_list_push_tail(&s->functions,&fn->node);
 nir_function_impl *impl=rzalloc(s,nir_function_impl);fn->impl=impl;impl->function=fn;impl->structured=true;
 exec_list_make_empty(&impl->body);exec_list_make_empty(&impl->locals);
 impl->end_block=rzalloc(s,nir_block);exec_list_make_empty(&impl->end_block->instr_list);
 nir_block *b=rzalloc(s,nir_block);b->cf_node.type=nir_cf_node_block;b->cf_node.parent=&impl->cf_node;
 exec_list_push_tail(&impl->body,&b->cf_node.node);exec_list_make_empty(&b->instr_list);
 nir_load_const_instr *c=rzalloc_size(s,sizeof(*c)+4*sizeof(nir_const_value));
 c->instr.type=nir_instr_type_load_const;c->instr.block=b;c->def.parent_instr=&c->instr;c->def.bit_size=32;c->def.num_components=4;
 c->value[0].u32=(color&0xff0000)?0x3f800000:0;c->value[1].u32=(color&0xff00)?0x3f800000:0;c->value[2].u32=(color&0xff)?0x3f800000:0;c->value[3].u32=(color&0xff000000)?0x3f800000:0;
 exec_list_push_tail(&b->instr_list,&c->instr.node);
 nir_load_const_instr *z=rzalloc_size(s,sizeof(*z)+sizeof(nir_const_value));z->instr.type=nir_instr_type_load_const;z->instr.block=b;z->def.parent_instr=&z->instr;z->def.bit_size=32;z->def.num_components=1;exec_list_push_tail(&b->instr_list,&z->instr.node);
 nir_intrinsic_instr *st=rzalloc_size(s,sizeof(*st)+2*sizeof(nir_src));st->instr.type=nir_instr_type_intrinsic;st->instr.block=b;st->intrinsic=nir_intrinsic_store_output;st->num_components=4;st->src[0].ssa=&c->def;st->src[1].ssa=&z->def;
 nir_intrinsic_set_write_mask(st,15);nir_intrinsic_set_src_type(st,nir_type_float32);nir_intrinsic_set_io_semantics(st,(nir_io_semantics){.location=FRAG_RESULT_DATA0,.num_slots=1});exec_list_push_tail(&b->instr_list,&st->instr.node);
 return s;
}
