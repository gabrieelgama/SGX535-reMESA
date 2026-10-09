/* SPDX-License-Identifier: MIT; actual Mesa builder, no GPU or pixels. */
#include "remesa_nir.h"
#include "nir_builder.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef NDEBUG
#error Actual Mesa admission tests require assertions enabled
#endif
#define CHECK(condition) do { if(!(condition)) {fprintf(stderr,"check failed line %d\n",__LINE__);abort();} } while(0)
int main(void)
{
 static const nir_shader_compiler_options options={0};unsigned cases=0;glsl_type_singleton_init_or_ref();
 for(unsigned mode=0;mode<7;mode++) {
  nir_builder b=nir_builder_init_simple_shader(MESA_SHADER_VERTEX,&options,"reMESA identity position");b.shader->info.io_lowered=true;
  nir_def *zero=nir_imm_int(&b,0);
  nir_def *position=nir_load_input(&b,4,32,zero,.dest_type=nir_type_float32,.io_semantics={.location=VERT_ATTRIB_GENERIC0,.num_slots=1});
  if(mode==1)position=nir_fadd(&b,position,nir_imm_vec4(&b,0,0,0,0));
  nir_store_output(&b,position,zero,.write_mask=15,.src_type=nir_type_float32,.io_semantics={.location=VARYING_SLOT_POS,.num_slots=1});
  if(mode==2)b.shader->info.vs.window_space_position=true;
  if(mode==3)b.shader->info.num_textures=1;
  if(mode==4)b.shader->info.clip_distance_array_size=1;
  if(mode==5)b.shader->info.system_values_read[0]=1;
  if(mode==6)b.shader->info.outputs_written=BITFIELD64_BIT(VARYING_SLOT_POS);
  unsigned slot=99;int result=remesa_nir_position_vertex(b.shader,&slot);
  CHECK((mode==0||mode==6)?(!result&&slot==0):(result&&slot==99));ralloc_free(b.shader);cases++;
 }
 for(unsigned lowered=0;lowered<2;lowered++)for(unsigned mode=0;mode<8;mode++) {
  nir_builder b=nir_builder_init_simple_shader(MESA_SHADER_VERTEX,&options,"reMESA position copy");
  nir_def *position=NULL,*zero=NULL;nir_variable *output=NULL;
  b.shader->info.io_lowered=lowered;
  if(lowered) {
   zero=nir_imm_int(&b,0);
   position=nir_load_input(&b,4,32,zero,.dest_type=nir_type_float32,.io_semantics={.location=VERT_ATTRIB_GENERIC0,.num_slots=1});
  } else {
   nir_variable *input=nir_variable_create(b.shader,nir_var_shader_in,glsl_vec4_type(),"position");input->data.location=VERT_ATTRIB_GENERIC0;
   output=nir_variable_create(b.shader,nir_var_shader_out,glsl_vec4_type(),"out");output->data.location=VARYING_SLOT_POS;
   position=nir_load_deref(&b,nir_build_deref_var(&b,input));
  }
  nir_def *copy=nir_mov(&b,position);
  if(mode==1)nir_instr_as_alu(copy->parent_instr)->src[0].swizzle[0]=1;
  if(mode==2)nir_instr_as_alu(copy->parent_instr)->exact=true;
  if(mode==3)copy=nir_mov(&b,copy);
  if(mode==4)copy=position; /* dead copy must not broaden admission */
  if(mode==5)b.shader->info.num_ubos=1;
  if(mode==6)b.shader->info.vs.needs_edge_flag=true;
  if(mode==7)b.shader->info.inputs_read=BITFIELD64_BIT(VERT_ATTRIB_GENERIC1);
  if(lowered)nir_store_output(&b,copy,zero,.write_mask=15,.src_type=nir_type_float32,.io_semantics={.location=VARYING_SLOT_POS,.num_slots=1});
  else nir_store_deref(&b,nir_build_deref_var(&b,output),copy,15);
  unsigned slot=99;int result=remesa_nir_position_vertex(b.shader,&slot);
  CHECK(mode==0?(!result&&slot==0):(result&&slot==99));ralloc_free(b.shader);cases++;
 }
 glsl_type_singleton_decref();printf("PASS %u actual builder vertex cases; no GPU\n",cases);return 0;
}
