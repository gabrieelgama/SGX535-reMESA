/* SPDX-License-Identifier: MIT */
#include "remesa_nir_uniform.h"
#include "remesa_nir.h"
#include "nir.h"
#include <errno.h>
#include <string.h>
struct value { nir_def *def;uint32_t bits; };
static int constant(const struct value *v,unsigned n,const nir_def *def,uint32_t *bits,unsigned *used)
{
 for(unsigned i=0;i<n;i++)if(v[i].def==def){*bits=v[i].bits;*used|=1u<<i;return 0;}
 return ENOTSUP;
}
int remesa_nir_uniform_fragment(const nir_shader *s,struct remesa_uniform_color *out)
{
 nir_function *entry=NULL;nir_intrinsic_instr *load=NULL;struct value values[3]={0};
 struct remesa_uniform_color result={.byte_size=16};unsigned functions=0,blocks=0,instructions=0,count=0,stores=0,used=0,loc=0;
 if(!s||!out)return EINVAL;
 if(s->info.stage!=MESA_SHADER_FRAGMENT||!s->info.io_lowered||!exec_list_is_empty(&s->variables)||
    s->num_inputs||s->num_outputs>1||s->num_uniforms||s->global_mem_size||s->scratch_size||s->constant_data_size||s->xfb_info||
    s->info.num_ubos!=1||s->info.num_textures||s->info.num_abos||s->info.num_ssbos||s->info.num_images||
    s->info.inputs_read||s->info.outputs_read||s->info.writes_memory||s->info.dual_slot_inputs||
    s->info.per_primitive_inputs||s->info.per_primitive_outputs||s->info.clip_distance_array_size||s->info.cull_distance_array_size||
    s->info.fs.uses_discard||s->info.fs.uses_fbfetch_output||s->info.fs.color_is_dual_source||
    s->info.fs.uses_sample_shading||s->info.fs.uses_sample_qualifier||s->info.fs.require_full_quads||s->info.fs.quad_derivatives||
    s->info.fs.early_fragment_tests||s->info.fs.post_depth_coverage||s->info.fs.inner_coverage||s->info.fs.origin_upper_left||
    s->info.fs.pixel_center_integer||s->info.fs.needs_coarse_quad_helper_invocations||s->info.fs.needs_full_quad_helper_invocations)return ENOTSUP;
 for(unsigned i=0;i<ARRAY_SIZE(s->info.system_values_read);i++)if(s->info.system_values_read[i])return ENOTSUP;
 for(unsigned i=0;i<ARRAY_SIZE(s->info.xfb_stride);i++)if(s->info.xfb_stride[i])return ENOTSUP;
 nir_foreach_function(fn,s) {
  if(++functions>1||!fn->is_entrypoint||fn->is_preamble||fn->is_exported||fn->shader!=s||fn->num_params||!fn->impl)return ENOTSUP;
  entry=fn;
 }
 if(!entry)return EINVAL;
 nir_function_impl *impl=entry->impl;
 if(impl->function!=entry||impl->preamble||!impl->end_block||!exec_list_is_empty(&impl->end_block->instr_list)||!impl->structured||!exec_list_is_empty(&impl->locals))return ENOTSUP;
 foreach_list_typed(nir_cf_node,node,node,&impl->body) {
  if(++blocks>1||node->type!=nir_cf_node_block||node->parent!=&impl->cf_node)return ENOTSUP;
  nir_block *block=nir_cf_node_as_block(node);
  nir_foreach_instr(instr,block) {
   if(++instructions>5||instr->block!=block)return EINVAL;
   if(instr->type==nir_instr_type_load_const) {
    if(count>=3||stores)return ENOTSUP;
    nir_load_const_instr *c=nir_instr_as_load_const(instr);
    if(c->def.parent_instr!=instr||c->def.num_components!=1||c->def.bit_size!=32)return ENOTSUP;
    values[count++]=(struct value){&c->def,c->value[0].u32};continue;
   }
   if(instr->type!=nir_instr_type_intrinsic)return ENOTSUP;
   nir_intrinsic_instr *io=nir_instr_as_intrinsic(instr);
   if(io->intrinsic==nir_intrinsic_load_ubo) {
    if(load||stores||io->num_components!=4||io->def.parent_instr!=instr||io->def.num_components!=4||io->def.bit_size!=32||
       nir_intrinsic_access(io)||nir_intrinsic_align_mul(io)!=16||nir_intrinsic_align_offset(io))return ENOTSUP;
    if(constant(values,count,io->src[0].ssa,&result.slot,&used)||constant(values,count,io->src[1].ssa,&result.byte_offset,&used)||
       result.slot||result.byte_offset%16||result.byte_offset>REMESA_UNIFORM_BYTES_MAX-16||
       nir_intrinsic_range_base(io)!=result.byte_offset||nir_intrinsic_range(io)!=16)return ENOTSUP;
    load=io;continue;
   }
   if(io->intrinsic!=nir_intrinsic_store_output)return ENOTSUP;
   nir_io_semantics sem=nir_intrinsic_io_semantics(io),expected={.location=sem.location,.num_slots=1};
   nir_io_xfb xfb=nir_intrinsic_io_xfb(io),xfb2=nir_intrinsic_io_xfb2(io),none={0};uint32_t offset=1;
   if(++stores>1||!load||io->src[0].ssa!=&load->def||constant(values,count,io->src[1].ssa,&offset,&used)||offset||
      io->num_components!=4||nir_intrinsic_write_mask(io)!=15||nir_intrinsic_src_type(io)!=nir_type_float32||
      nir_intrinsic_base(io)||nir_intrinsic_component(io)||nir_intrinsic_range(io)>1||
      (sem.location!=FRAG_RESULT_COLOR&&sem.location!=FRAG_RESULT_DATA0)||memcmp(&sem,&expected,sizeof(sem))||
      memcmp(&xfb,&none,sizeof(xfb))||memcmp(&xfb2,&none,sizeof(xfb2)))return ENOTSUP;
   loc=sem.location;
  }
 }
 if(blocks!=1||stores!=1||!load||!count||used!=((1u<<count)-1u)||
    (s->info.outputs_written&&s->info.outputs_written!=BITFIELD64_BIT(loc)))return ENOTSUP;
 *out=result;return 0;
}
int remesa_uniform_color_resolve(const struct remesa_uniform_color *desc,const void *buffer,size_t bytes,uint32_t *out)
{
 uint32_t rgba[4];
 if(!desc||!buffer||!out)return EINVAL;
 if(desc->slot||desc->byte_size!=16||desc->byte_offset%16||desc->byte_offset>REMESA_UNIFORM_BYTES_MAX-16)return EINVAL;
 if(bytes>REMESA_UNIFORM_BYTES_MAX||desc->byte_offset>bytes||bytes-desc->byte_offset<16)return EINVAL;
 memcpy(rgba,(const unsigned char *)buffer+desc->byte_offset,sizeof(rgba));
 return remesa_constant_rgba32(rgba,out);
}
