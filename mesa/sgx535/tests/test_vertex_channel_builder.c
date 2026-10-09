/* SPDX-License-Identifier: MIT; real Mesa NIR builder, no GPU execution. */
#include "remesa_nir.h"
#include "nir_builder.h"
#include "util/ralloc.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#ifdef NDEBUG
#error Shader admission tests require assertions
#endif
int main(void)
{
 static const nir_shader_compiler_options options={0};unsigned cases=0;
 glsl_type_singleton_init_or_ref();
 for(unsigned lowered=0;lowered<2;lowered++)for(unsigned mode=0;mode<6;mode++) {
  nir_builder b=nir_builder_init_simple_shader(MESA_SHADER_VERTEX,&options,"reMESA channel identity");
  nir_def *input=NULL,*zero=NULL;nir_variable *out=NULL;
  if(lowered) {
   b.shader->info.io_lowered=true;zero=nir_imm_int(&b,0);
   input=nir_load_input(&b,4,32,zero,.dest_type=nir_type_float32,.io_semantics={.location=VERT_ATTRIB_GENERIC0,.num_slots=1});
  } else {
   nir_variable *in=nir_variable_create(b.shader,nir_var_shader_in,glsl_vec4_type(),"position");in->data.location=VERT_ATTRIB_GENERIC0;
   input=nir_load_deref(&b,nir_build_deref_var(&b,in));
   out=nir_variable_create(b.shader,nir_var_shader_out,glsl_vec4_type(),"output");out->data.location=VARYING_SLOT_POS;
  }
  nir_def *channel[4];for(unsigned i=0;i<4;i++)channel[i]=nir_channel(&b,input,i);
  nir_def *value=nir_vec4(&b,channel[0],channel[1],channel[2],channel[3]);
  if(mode==1)value=nir_mov(&b,value);
  if(mode==2)value=nir_vec4(&b,channel[1],channel[0],channel[2],channel[3]);
  if(mode==3)value=nir_fneg(&b,value);
  if(mode==4)(void)nir_mov(&b,input);
  if(mode==5)value=nir_vec4(&b,channel[0],channel[1],channel[2],nir_imm_float(&b,1));
  if(lowered)nir_store_output(&b,value,zero,.write_mask=15,.src_type=nir_type_float32,.io_semantics={.location=VARYING_SLOT_POS,.num_slots=1});
  else nir_store_deref(&b,nir_build_deref_var(&b,out),value,15);
  unsigned slot=99;int error=remesa_nir_position_vertex(b.shader,&slot);
  assert(mode<2?(!error&&slot==0):(error&&slot==99));cases++;ralloc_free(b.shader);
 }
 glsl_type_singleton_decref();printf("PASS %u actual Mesa scalarized vertex builder cases; no GPU\n",cases);return 0;
}
