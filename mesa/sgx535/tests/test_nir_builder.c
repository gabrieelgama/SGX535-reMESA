/* SPDX-License-Identifier: MIT: actual Mesa NIR builder, offline only. */
#include "remesa_nir.h"
#include "nir_builder.h"
#include "util/ralloc.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#ifdef NDEBUG
#error Actual Mesa admission tests require assertions enabled
#endif
int main(void)
{
 static const nir_shader_compiler_options options={0};unsigned cases=0;
 glsl_type_singleton_init_or_ref();
 for(unsigned lowered=0;lowered<2;lowered++)for(unsigned color=0;color<3;color++) {
  nir_builder b=nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,&options,"reMESA constant offline");
  float r=color==1?0:1,g=color==0?0:1,bl=color==0?1:0;uint32_t out=0;
  nir_def *value=nir_imm_vec4(&b,r,g,bl,1);
  /* A component-preserving NIR mov is semantically the same constant
   * profile and must not force a new hardware program. */
  if(lowered && color==2)value=nir_mov(&b,value);
  if(lowered){b.shader->info.io_lowered=true;nir_store_output(&b,value,nir_imm_int(&b,0),.write_mask=15,.src_type=nir_type_float32,.io_semantics={.location=FRAG_RESULT_DATA0,.num_slots=1});}
  else {nir_variable *v=nir_variable_create(b.shader,nir_var_shader_out,glsl_vec4_type(),"color");v->data.location=FRAG_RESULT_DATA0;nir_store_deref(&b,nir_build_deref_var(&b,v),value,15);}
  assert(!remesa_nir_constant_fragment(b.shader,&out));assert(out==(color==0?0xffff00ff:color==1?0xff00ff00:0xffffff00));cases++;
  b.shader->info.fs.uses_discard=true;assert(remesa_nir_constant_fragment(b.shader,&out)==ENOTSUP);cases++;ralloc_free(b.shader);
 }
 /* Arithmetic must stay rejected: no SGX ALU encoding is admitted by this
  * semantic-only profile translator. */
 nir_builder reject=nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,&options,"reMESA arithmetic reject");
 nir_def *lhs=nir_imm_vec4(&reject,1,0,1,1);
 nir_def *rhs=nir_imm_vec4(&reject,0,0,0,0);
 nir_variable *rv=nir_variable_create(reject.shader,nir_var_shader_out,glsl_vec4_type(),"color");
 rv->data.location=FRAG_RESULT_DATA0;
 nir_store_deref(&reject,nir_build_deref_var(&reject,rv),nir_fadd(&reject,lhs,rhs),15);
 uint32_t rejected=0;assert(remesa_nir_constant_fragment(reject.shader,&rejected)==ENOTSUP);cases++;
 ralloc_free(reject.shader);
 /* Unused/multiple copies are outside the deliberately finite profile. */
 nir_builder extra=nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,&options,"unused mov reject");
 nir_def *cv=nir_imm_vec4(&extra,1,0,1,1);
 (void)nir_mov(&extra,cv);
 nir_variable *ev=nir_variable_create(extra.shader,nir_var_shader_out,glsl_vec4_type(),"color");ev->data.location=FRAG_RESULT_DATA0;
 nir_store_deref(&extra,nir_build_deref_var(&extra,ev),cv,15);
 assert(remesa_nir_constant_fragment(extra.shader,&rejected)==ENOTSUP);cases++;
 ralloc_free(extra.shader);
 /* Metadata must not admit unused resource contracts or decorated outputs
  * just because their constant color happens to match the known profile. */
 for(unsigned mode=0;mode<10;mode++) {
  nir_builder b=nir_builder_init_simple_shader(MESA_SHADER_FRAGMENT,&options,"metadata rejection");
  nir_def *value=nir_imm_vec4(&b,1,0,1,1);
  nir_variable *v=nir_variable_create(b.shader,nir_var_shader_out,glsl_vec4_type(),"color");v->data.location=FRAG_RESULT_DATA0;
  nir_store_deref(&b,nir_build_deref_var(&b,v),value,15);
  if(mode==0)b.shader->info.num_textures=1;
  if(mode==1)b.shader->info.num_ubos=1;
  if(mode==2)b.shader->info.num_ssbos=1;
  if(mode==3)b.shader->info.num_images=1;
  if(mode==4)b.shader->info.outputs_read=BITFIELD64_BIT(FRAG_RESULT_DATA0);
  if(mode==5)b.shader->info.system_values_read[0]=1;
  if(mode==6)b.shader->info.xfb_stride[0]=4;
  if(mode==7)v->data.bindless=true;
  if(mode==8)v->data.explicit_binding=true;
  if(mode==9)v->data.is_xfb=true;
  uint32_t unchanged=0x12345678;
  assert(remesa_nir_constant_fragment(b.shader,&unchanged)==ENOTSUP);
  assert(unchanged==0x12345678);cases++;ralloc_free(b.shader);
 }
 glsl_type_singleton_decref();printf("PASS %u actual Mesa NIR builder cases (NO GPU)\n",cases);return 0;
}
