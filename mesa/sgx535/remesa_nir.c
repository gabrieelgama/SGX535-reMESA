/* SPDX-License-Identifier: MIT */
#include "remesa_nir.h"
#include <errno.h>
#include <string.h>
#include "nir.h"
int remesa_constant_rgba32(const uint32_t rgba[4],uint32_t *out)
{
 uint32_t bytes[4],argb;
 if(!rgba||!out)return EINVAL;
 for(unsigned i=0;i<4;i++) {
  if(rgba[i]!=0 && rgba[i]!=0x3f800000)return ENOTSUP;
  bytes[i]=rgba[i] ? 255 : 0;
 }
 argb=(bytes[3]<<24)|(bytes[0]<<16)|(bytes[1]<<8)|bytes[2];
 /* These exact opaque outputs were physically observed. Zero background
  * pixels are not evidence for a transparent fragment export. */
 if(argb!=0xffff00ff && argb!=0xff00ff00 && argb!=0xffffff00)return ENOTSUP;
 *out=argb;return 0;
}
static bool location(unsigned n) {return n==FRAG_RESULT_COLOR||n==FRAG_RESULT_DATA0;}
static bool vector4(const struct glsl_type *t)
{return t&&t->base_type==GLSL_TYPE_FLOAT&&t->vector_elements==4&&t->matrix_columns==1;}
/* Track only bit-preserving SSA channel routing from the sole POSITION input.
 * Scalarization and vector reconstruction do not require new SGX instructions:
 * the accepted final value is provably the same float4 position profile. */
#define REMESA_POSITION_VALUES 16
struct position_value {
 nir_def *def;
 unsigned components,channel[4];
 uint32_t dependencies;
};
static int position_index(const struct position_value values[],unsigned count,
                          const nir_def *def)
{
 for(unsigned i=0;i<count;i++)if(values[i].def==def)return (int)i;
 return -1;
}
static void position_input(struct position_value values[],unsigned *count,nir_def *def)
{
 values[0]=(struct position_value){.def=def,.components=4,.channel={0,1,2,3},.dependencies=1};
 *count=1;
}
static int position_copy(struct position_value values[],unsigned *count,nir_alu_instr *alu)
{
 struct position_value value={0};
 if(!*count||*count>=REMESA_POSITION_VALUES||alu->exact||
    alu->def.parent_instr!=&alu->instr||alu->def.bit_size!=32||
    !alu->def.num_components||alu->def.num_components>4)return ENOTSUP;
 value.def=&alu->def;value.components=alu->def.num_components;
 if(alu->op==nir_op_mov) {
  int src=position_index(values,*count,alu->src[0].src.ssa);
  if(src<0)return ENOTSUP;
  for(unsigned i=0;i<value.components;i++) {
   unsigned channel=alu->src[0].swizzle[i];
   if(channel>=values[src].components)return ENOTSUP;
   value.channel[i]=values[src].channel[channel];
  }
  value.dependencies=values[src].dependencies;
 } else if(alu->op==nir_op_vec4&&value.components==4) {
  for(unsigned i=0;i<4;i++) {
   int src=position_index(values,*count,alu->src[i].src.ssa);
   unsigned channel=alu->src[i].swizzle[0];
   if(src<0||channel>=values[src].components)return ENOTSUP;
   value.channel[i]=values[src].channel[channel];
   value.dependencies|=values[src].dependencies;
  }
 } else return ENOTSUP;
 value.dependencies|=1u<<*count;values[(*count)++]=value;return 0;
}
static bool position_identity(const struct position_value values[],unsigned count,nir_def *def)
{
 int index=position_index(values,count,def);
 if(index<0||values[index].components!=4||values[index].dependencies!=((1u<<count)-1u))return false;
 for(unsigned i=0;i<4;i++)if(values[index].channel[i]!=i)return false;
 return true;
}
/* Compile-time construction of a constant does not imply native ALU support.
 * Every admitted value is defined earlier in this one block, and every value
 * must contribute to the sole export (or its zero offset).  The finite table
 * rejects cycles, forward references, dead instructions and unbounded chains.
 * No floating-point arithmetic or conversion is performed here. */
#define REMESA_CONSTANT_VALUES 16
struct constant_value {
 nir_def *def;
 uint32_t bits[4];
 uint32_t dependencies;
 unsigned components;
};
static int constant_index(const struct constant_value values[],unsigned count,
                          const nir_def *def)
{
 for(unsigned i=0;i<count;i++)if(values[i].def==def)return (int)i;
 return -1;
}
static int append_constant(struct constant_value values[],unsigned *count,
                           nir_instr *instr)
{
 struct constant_value value={0};
 if(*count>=REMESA_CONSTANT_VALUES)return ENOTSUP;
 if(instr->type==nir_instr_type_load_const) {
  nir_load_const_instr *load=nir_instr_as_load_const(instr);
  if(load->def.parent_instr!=instr||load->def.bit_size!=32||
     (load->def.num_components!=1&&load->def.num_components!=4))return ENOTSUP;
  value.def=&load->def;value.components=load->def.num_components;
  for(unsigned i=0;i<value.components;i++)value.bits[i]=load->value[i].u32;
 } else {
  nir_alu_instr *alu=nir_instr_as_alu(instr);
  if(alu->def.parent_instr!=instr||alu->def.bit_size!=32||alu->exact)return ENOTSUP;
  value.def=&alu->def;value.components=alu->def.num_components;
  if(alu->op==nir_op_mov) {
   int index=constant_index(values,*count,alu->src[0].src.ssa);
   if(index<0||value.components!=values[index].components)return ENOTSUP;
   for(unsigned i=0;i<value.components;i++) {
    if(alu->src[0].swizzle[i]!=i)return ENOTSUP;
    value.bits[i]=values[index].bits[i];
   }
   value.dependencies=values[index].dependencies;
  } else if(alu->op==nir_op_vec4&&value.components==4) {
   for(unsigned i=0;i<4;i++) {
    int index=constant_index(values,*count,alu->src[i].src.ssa);
    if(index<0||values[index].components!=1||alu->src[i].swizzle[0])return ENOTSUP;
    value.bits[i]=values[index].bits[0];
    value.dependencies|=values[index].dependencies;
   }
  } else return ENOTSUP;
 }
 value.dependencies|=1u<<*count;
 values[(*count)++]=value;return 0;
}
int remesa_nir_constant_fragment(const nir_shader *shader,uint32_t *out)
{
 nir_function *entry=NULL;nir_variable *output=NULL;unsigned functions=0,variables=0,blocks=0,instructions=0,stores=0,values_count=0,derefs=0,loc=0;
 struct constant_value values[REMESA_CONSTANT_VALUES]={0};
 nir_deref_instr *deref=NULL;uint32_t used=0,bits[4]={0};
 if(!shader||!out)return EINVAL;
 if(shader->info.stage!=MESA_SHADER_FRAGMENT)return ENOTSUP;
 if(shader->num_inputs||shader->num_outputs>1||shader->num_uniforms||shader->global_mem_size||shader->scratch_size||shader->constant_data_size||shader->xfb_info||
    shader->info.num_textures||shader->info.num_ubos||shader->info.num_abos||shader->info.num_ssbos||shader->info.num_images||shader->info.outputs_read||
    shader->info.inputs_read||shader->info.writes_memory||shader->info.fs.uses_discard||shader->info.fs.uses_fbfetch_output||
    shader->info.fs.color_is_dual_source||shader->info.fs.uses_sample_shading||shader->info.fs.uses_sample_qualifier||
    shader->info.fs.require_full_quads||shader->info.fs.quad_derivatives||shader->info.fs.early_fragment_tests||
    shader->info.fs.post_depth_coverage||shader->info.fs.inner_coverage||shader->info.fs.origin_upper_left||
    shader->info.fs.pixel_center_integer||shader->info.fs.needs_coarse_quad_helper_invocations||shader->info.fs.needs_full_quad_helper_invocations)return ENOTSUP;
 for(unsigned i=0;i<ARRAY_SIZE(shader->info.system_values_read);i++)if(shader->info.system_values_read[i])return ENOTSUP;
 for(unsigned i=0;i<ARRAY_SIZE(shader->info.xfb_stride);i++)if(shader->info.xfb_stride[i])return ENOTSUP;
 nir_foreach_variable_in_shader(var,shader) {
  if(++variables>1||var->data.mode!=nir_var_shader_out||!location(var->data.location)||
     var->data.index||var->data.driver_location||var->data.location_frac||var->data.patch||var->data.per_view||var->data.per_primitive||var->data.compact||var->data.centroid||var->data.sample||
     var->data.interpolation||var->data.fb_fetch_output||var->num_members||var->members||var->interface_type||
     var->num_state_slots||var->state_slots||var->constant_initializer||var->pointer_initializer||
     var->data.access||var->data.bindless||var->data.binding||var->data.descriptor_set||var->data.explicit_binding||
     var->data.is_xfb||var->data.is_xfb_only||var->data.stream||var->data.read_only||
     var->data.explicit_xfb_buffer||var->data.explicit_xfb_stride||var->data.explicit_offset||
     var->data.matrix_layout||var->data.from_named_ifc_block||var->data.from_ssbo_unsized_array||
     var->data.offset||var->data.ray_query||var->data.has_initializer||var->data.is_implicit_initializer||
     (var->data.precision!=GLSL_PRECISION_NONE&&var->data.precision!=GLSL_PRECISION_HIGH)||!vector4(var->type))return ENOTSUP;
  output=var;
 }
 nir_foreach_function(fn,shader) {
  if(++functions>1||!fn->is_entrypoint||fn->is_preamble||fn->is_exported||fn->shader!=shader||fn->num_params||!fn->impl)return ENOTSUP;
  entry=fn;
 }
 if(!entry)return EINVAL;
 nir_function_impl *impl=entry->impl;
 if(impl->function!=entry||impl->preamble||!impl->end_block||!exec_list_is_empty(&impl->end_block->instr_list)||!impl->structured||!exec_list_is_empty(&impl->locals))return ENOTSUP;
 foreach_list_typed(nir_cf_node,node,node,&impl->body) {
  if(++blocks>1||node->type!=nir_cf_node_block||node->parent!=&impl->cf_node)return ENOTSUP;
  nir_block *block=nir_cf_node_as_block(node);
  nir_foreach_instr(instr,block) {
   if(++instructions>REMESA_CONSTANT_VALUES+2||instr->block!=block)return EINVAL;
   if(instr->type==nir_instr_type_load_const||instr->type==nir_instr_type_alu) {
    if(stores)return ENOTSUP;
    int error=append_constant(values,&values_count,instr);
    if(error)return error;
    continue;
   }
   if(instr->type==nir_instr_type_deref) {
    deref=nir_instr_as_deref(instr);
    if(++derefs>1||!output||deref->deref_type!=nir_deref_type_var||deref->var!=output||
       deref->modes!=nir_var_shader_out||deref->type!=output->type||deref->def.parent_instr!=instr)return ENOTSUP;
    continue;
   }
   if(instr->type!=nir_instr_type_intrinsic)return ENOTSUP;
   nir_intrinsic_instr *store=nir_instr_as_intrinsic(instr);nir_def *value=NULL,*offset=NULL;
   if(store->intrinsic==nir_intrinsic_store_deref) {
    if(shader->info.io_lowered||!output||!deref||store->src[0].ssa!=&deref->def||nir_intrinsic_access(store))return ENOTSUP;
    value=store->src[1].ssa;loc=output->data.location;
   } else if(store->intrinsic==nir_intrinsic_store_output) {
    nir_io_semantics sem=nir_intrinsic_io_semantics(store),canonical={.location=sem.location,.num_slots=1};
    nir_io_xfb xfb=nir_intrinsic_io_xfb(store),xfb2=nir_intrinsic_io_xfb2(store),none={0};
    if(!shader->info.io_lowered||variables||derefs||!location(sem.location)||sem.num_slots!=1||
       sem.dual_source_blend_index||sem.medium_precision||memcmp(&sem,&canonical,sizeof(sem))||memcmp(&xfb,&none,sizeof(xfb))||memcmp(&xfb2,&none,sizeof(xfb2))||
       nir_intrinsic_range(store)>1||nir_intrinsic_base(store)||nir_intrinsic_component(store)||
       nir_intrinsic_src_type(store)!=nir_type_float32)return ENOTSUP;
    loc=sem.location;value=store->src[0].ssa;offset=store->src[1].ssa;
   } else return ENOTSUP;
   if(++stores>1||store->num_components!=4||nir_intrinsic_write_mask(store)!=15||!value)return ENOTSUP;
   int color=constant_index(values,values_count,value);
   if(color<0||values[color].components!=4)return ENOTSUP;
   used|=values[color].dependencies;
   if(offset) {
    int zero=constant_index(values,values_count,offset);
    if(zero<0||values[zero].components!=1||values[zero].bits[0])return ENOTSUP;
    used|=values[zero].dependencies;
   }
   for(unsigned i=0;i<4;i++)bits[i]=values[color].bits[i];
  }
 }
 if(blocks!=1||stores!=1||!values_count||used!=((1u<<values_count)-1u)||
    derefs!=(shader->info.io_lowered?0u:1u))return ENOTSUP;
 if(shader->info.outputs_written && shader->info.outputs_written!=BITFIELD64_BIT(loc))return ENOTSUP;
 return remesa_constant_rgba32(bits,out);
}
static bool position_attribute(unsigned location)
{
 return location==VERT_ATTRIB_GENERIC0||location==VERT_ATTRIB_POS;
}
/* Variable IO representation of exactly POSITION = one position attribute.
 * Mesa compatibility gl_Vertex and programmable generic0 have the same
 * independently recognized float4 passthrough semantics; both use input slot0.
 * NIR itself is
 * inspected without mutation; this produces the same restricted descriptor as
 * lowered IO. It does not encode or claim a new SGX shader program. */
static int position_vertex_deref(const nir_shader *shader,unsigned *input_slot)
{
 nir_variable *input_var=NULL,*output_var=NULL;nir_function *entry=NULL;
 nir_deref_instr *input_deref=NULL,*output_deref=NULL;nir_intrinsic_instr *load=NULL;
 struct position_value values[REMESA_POSITION_VALUES]={0};unsigned values_count=0;
 unsigned variables=0,functions=0,blocks=0,instructions=0,stores=0;
 nir_foreach_variable_in_shader(var,shader) {
  if(++variables>2 || !vector4(var->type) || var->num_members || var->members ||
     var->interface_type || var->num_state_slots || var->state_slots ||
     var->constant_initializer || var->pointer_initializer ||
     var->data.index || var->data.driver_location || var->data.location_frac ||
     var->data.patch || var->data.per_view || var->data.per_primitive ||
     var->data.compact || var->data.centroid || var->data.sample ||
     (var->data.interpolation && !(var->data.mode==nir_var_shader_out && var->data.interpolation==INTERP_MODE_SMOOTH)) || var->data.fb_fetch_output || var->data.is_xfb ||
     var->data.is_xfb_only || var->data.stream || var->data.access ||
     var->data.bindless || var->data.explicit_binding || var->data.binding || var->data.descriptor_set ||
     var->data.explicit_xfb_buffer || var->data.explicit_xfb_stride || var->data.explicit_offset ||
     var->data.matrix_layout || var->data.from_named_ifc_block || var->data.from_ssbo_unsized_array ||
     var->data.offset || var->data.ray_query ||
     var->data.has_initializer || var->data.is_implicit_initializer ||
     (var->data.precision!=GLSL_PRECISION_NONE&&var->data.precision!=GLSL_PRECISION_HIGH))return ENOTSUP;
  if(var->data.mode==nir_var_shader_in) {
   if(input_var||!position_attribute(var->data.location))return ENOTSUP;
   input_var=var;
  } else if(var->data.mode==nir_var_shader_out) {
   if(output_var||var->data.location!=VARYING_SLOT_POS||var->data.read_only)return ENOTSUP;
   output_var=var;
  } else return ENOTSUP;
 }
 if(variables!=2||!input_var||!output_var)return ENOTSUP;
 if(shader->info.inputs_read&&shader->info.inputs_read!=BITFIELD64_BIT(input_var->data.location))return ENOTSUP;
 nir_foreach_function(fn,shader) {
  if(++functions>1||!fn->is_entrypoint||fn->is_preamble||fn->is_exported||fn->shader!=shader||fn->num_params||!fn->impl)return ENOTSUP;
  entry=fn;
 }
 if(!entry)return EINVAL;
 nir_function_impl *impl=entry->impl;
 if(impl->function!=entry||impl->preamble||!impl->end_block||!exec_list_is_empty(&impl->end_block->instr_list)||!impl->structured||!exec_list_is_empty(&impl->locals))return ENOTSUP;
 foreach_list_typed(nir_cf_node,node,node,&impl->body) {
  if(++blocks>1||node->type!=nir_cf_node_block||node->parent!=&impl->cf_node)return ENOTSUP;
  nir_block *block=nir_cf_node_as_block(node);
  nir_foreach_instr(instr,block) {
   if(++instructions>REMESA_POSITION_VALUES+3||instr->block!=block)return EINVAL;
   if(instr->type==nir_instr_type_deref) {
    nir_deref_instr *d=nir_instr_as_deref(instr);
    if(d->deref_type!=nir_deref_type_var||d->def.parent_instr!=instr||
       d->def.num_components!=1||d->def.bit_size!=32)return ENOTSUP;
    if(d->var==input_var) {
     if(input_deref||d->modes!=nir_var_shader_in||d->type!=input_var->type)return ENOTSUP;
     input_deref=d;
    } else if(d->var==output_var) {
     if(output_deref||d->modes!=nir_var_shader_out||d->type!=output_var->type)return ENOTSUP;
     output_deref=d;
    } else return ENOTSUP;
    continue;
   }
   if(instr->type==nir_instr_type_alu) {
    if(stores||!load||position_copy(values,&values_count,nir_instr_as_alu(instr)))return ENOTSUP;
    continue;
   }
   if(instr->type!=nir_instr_type_intrinsic)return ENOTSUP;
   nir_intrinsic_instr *io=nir_instr_as_intrinsic(instr);
   if(io->intrinsic==nir_intrinsic_load_deref) {
    if(load||stores||!input_deref||io->src[0].ssa!=&input_deref->def||
       io->num_components!=4||io->def.parent_instr!=instr||io->def.num_components!=4||
       io->def.bit_size!=32||nir_intrinsic_access(io))return ENOTSUP;
    load=io;position_input(values,&values_count,&load->def);continue;
   }
   if(io->intrinsic!=nir_intrinsic_store_deref||++stores>1||!output_deref||!load||
      io->src[0].ssa!=&output_deref->def||!position_identity(values,values_count,io->src[1].ssa)||
      io->num_components!=4||nir_intrinsic_write_mask(io)!=15||nir_intrinsic_access(io))return ENOTSUP;
  }
 }
 if(blocks!=1||instructions!=values_count+3||stores!=1||!load||!input_deref||!output_deref)return ENOTSUP;
 *input_slot=0;return 0;
}
/* A description of an identity position shader, not a general NIR compiler.
 * The caller separately validates vertex layout and clip-to-window preparation.
 * Exact lowered or variable IO: no uniforms, transforms, texture/memory or sysvalues. */
int remesa_nir_position_vertex(const nir_shader *shader,unsigned *input_slot)
{
 nir_function *entry=NULL;nir_load_const_instr *zero=NULL;nir_intrinsic_instr *input=NULL;
 struct position_value values[REMESA_POSITION_VALUES]={0};unsigned values_count=0;
 unsigned functions=0,blocks=0,instructions=0,stores=0;
 if(!shader||!input_slot)return EINVAL;
 if(shader->info.stage!=MESA_SHADER_VERTEX||
    (shader->info.io_lowered&&!exec_list_is_empty(&shader->variables))||shader->num_inputs>1||shader->num_outputs>1||shader->num_uniforms||
    shader->global_mem_size||shader->scratch_size||shader->constant_data_size||shader->xfb_info||
    shader->info.writes_memory||shader->info.num_textures||shader->info.num_ubos||shader->info.num_abos||
    shader->info.num_ssbos||shader->info.num_images||shader->info.dual_slot_inputs||shader->info.outputs_read||
    shader->info.per_primitive_inputs||shader->info.per_primitive_outputs||shader->info.clip_distance_array_size||
    shader->info.cull_distance_array_size||shader->info.vs.double_inputs||shader->info.vs.blit_sgprs_amd||
    shader->info.vs.tes_agx||shader->info.vs.window_space_position||shader->info.vs.needs_edge_flag)return ENOTSUP;
 if((shader->info.inputs_read&&shader->info.inputs_read!=BITFIELD64_BIT(VERT_ATTRIB_GENERIC0)&&
     shader->info.inputs_read!=BITFIELD64_BIT(VERT_ATTRIB_POS))||
    (shader->info.outputs_written&&shader->info.outputs_written!=BITFIELD64_BIT(VARYING_SLOT_POS)))return ENOTSUP;
 for(unsigned i=0;i<ARRAY_SIZE(shader->info.system_values_read);i++)if(shader->info.system_values_read[i])return ENOTSUP;
 for(unsigned i=0;i<ARRAY_SIZE(shader->info.xfb_stride);i++)if(shader->info.xfb_stride[i])return ENOTSUP;
 if(!shader->info.io_lowered)return position_vertex_deref(shader,input_slot);
 nir_foreach_function(fn,shader) {
  if(++functions>1||!fn->is_entrypoint||fn->is_preamble||fn->is_exported||fn->shader!=shader||fn->num_params||!fn->impl)return ENOTSUP;
  entry=fn;
 }
 if(!entry)return EINVAL;
 nir_function_impl *impl=entry->impl;
 if(impl->function!=entry||impl->preamble||!impl->end_block||!exec_list_is_empty(&impl->end_block->instr_list)||!impl->structured||!exec_list_is_empty(&impl->locals))return ENOTSUP;
 foreach_list_typed(nir_cf_node,node,node,&impl->body) {
  if(++blocks>1||node->type!=nir_cf_node_block||node->parent!=&impl->cf_node)return ENOTSUP;
  nir_block *block=nir_cf_node_as_block(node);
  nir_foreach_instr(instr,block) {
   if(++instructions>REMESA_POSITION_VALUES+2||instr->block!=block)return EINVAL;
   if(instr->type==nir_instr_type_load_const) {
    if(zero)return ENOTSUP;
    zero=nir_instr_as_load_const(instr);
    if(zero->def.parent_instr!=instr||zero->def.bit_size!=32||zero->def.num_components!=1||zero->value[0].u32)return ENOTSUP;
    continue;
   }
   if(instr->type==nir_instr_type_alu) {
    if(stores||!input||position_copy(values,&values_count,nir_instr_as_alu(instr)))return ENOTSUP;
    continue;
   }
   if(instr->type!=nir_instr_type_intrinsic)return ENOTSUP;
   nir_intrinsic_instr *io=nir_instr_as_intrinsic(instr);
   if(io->intrinsic==nir_intrinsic_load_input) {
    nir_io_semantics sem=nir_intrinsic_io_semantics(io),expected={.location=sem.location,.num_slots=1};
    if(input||!zero||stores||io->src[0].ssa!=&zero->def||io->num_components!=4||
       !position_attribute(sem.location)||
       (shader->info.inputs_read&&shader->info.inputs_read!=BITFIELD64_BIT(sem.location))||
       io->def.parent_instr!=instr||io->def.bit_size!=32||io->def.num_components!=4||
       nir_intrinsic_dest_type(io)!=nir_type_float32||nir_intrinsic_base(io)||nir_intrinsic_component(io)||
       nir_intrinsic_range(io)>1||memcmp(&sem,&expected,sizeof(sem)))return ENOTSUP;
    input=io;position_input(values,&values_count,&input->def);continue;
   }
   if(io->intrinsic!=nir_intrinsic_store_output)return ENOTSUP;
   nir_io_semantics sem=nir_intrinsic_io_semantics(io),expected={.location=VARYING_SLOT_POS,.num_slots=1};
   nir_io_xfb xfb=nir_intrinsic_io_xfb(io),xfb2=nir_intrinsic_io_xfb2(io),none={0};
   if(++stores>1||!input||!position_identity(values,values_count,io->src[0].ssa)||io->src[1].ssa!=&zero->def||
      io->num_components!=4||nir_intrinsic_write_mask(io)!=15||nir_intrinsic_src_type(io)!=nir_type_float32||
      nir_intrinsic_base(io)||nir_intrinsic_component(io)||nir_intrinsic_range(io)>1||
      memcmp(&sem,&expected,sizeof(sem))||memcmp(&xfb,&none,sizeof(xfb))||memcmp(&xfb2,&none,sizeof(xfb2)))return ENOTSUP;
  }
 }
 if(blocks!=1||!zero||!input||stores!=1||instructions!=values_count+2)return ENOTSUP;
 *input_slot=0;return 0;
}
