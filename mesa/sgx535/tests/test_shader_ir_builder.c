/* SPDX-License-Identifier: MIT; actual licensed Mesa NIR builder API. */
#include "remesa_shader_ir.h"
#include "nir_builder.h"
#include "util/ralloc.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
int main(void)
{
 const nir_shader_compiler_options options={0};unsigned cases=0;glsl_type_singleton_init_or_ref();
 for(unsigned stage=0;stage<2;stage++)for(unsigned lowered=0;lowered<2;lowered++)for(unsigned mode=0;mode<10;mode++) {
  nir_builder b=nir_builder_init_simple_shader(stage?MESA_SHADER_FRAGMENT:MESA_SHADER_VERTEX,&options,"original semantic IR");
  nir_def *v=NULL,*zero=NULL;nir_variable *out=NULL;
  if(lowered){b.shader->info.io_lowered=true;zero=nir_imm_int(&b,0);}
  if(stage)v=nir_imm_vec4(&b,1,0,1,1);
  else if(lowered)v=nir_load_input(&b,4,32,zero,.dest_type=nir_type_float32,.io_semantics={.location=VERT_ATTRIB_GENERIC0,.num_slots=1});
  else {nir_variable *in=nir_variable_create(b.shader,nir_var_shader_in,glsl_vec4_type(),"position");in->data.location=VERT_ATTRIB_GENERIC0;v=nir_load_deref(&b,nir_build_deref_var(&b,in));}
  if(mode==1)v=nir_mov(&b,v);
  if(mode==2)v=nir_fadd(&b,v,v);
  if(mode==3)(void)nir_mov(&b,v);
  if(mode==4)v=nir_fneg(&b,v);
  if(mode==5)b.shader->info.stage=MESA_SHADER_COMPUTE;
  if(mode==6)b.shader->info.num_textures=1;
  if(mode==7)b.shader->info.writes_memory=true;
  if(mode==8)b.shader->num_uniforms=1;
  if(mode==9)b.shader->global_mem_size=4;
  unsigned location=stage?FRAG_RESULT_COLOR:VARYING_SLOT_POS;
  if(lowered)nir_store_output(&b,v,zero,.write_mask=15,.src_type=nir_type_float32,.io_semantics={.location=location,.num_slots=1});
  else {out=nir_variable_create(b.shader,nir_var_shader_out,glsl_vec4_type(),"output");out->data.location=location;nir_store_deref(&b,nir_build_deref_var(&b,out),v,15);}
  struct remesa_shader_ir p,original;memset(&p,0xa5,sizeof(p));original=p;
  int error=remesa_shader_ir_lower(b.shader,&p);
  if(mode<2){assert(!error&&!remesa_shader_ir_validate(&p));assert(p.stage==(stage?REMESA_IR_FRAGMENT:REMESA_IR_VERTEX));
   unsigned char bytes[8]={0};size_t written=999;assert(remesa_shader_ir_encode_sgx535(&p,bytes,sizeof(bytes),&written)==ENOTSUP&&written==999);
  }else assert(error&&!memcmp(&p,&original,sizeof(p)));
  cases++;ralloc_free(b.shader);
 }
 glsl_type_singleton_decref();printf("PASS %u actual Mesa NIR lowering cases; no GPU\n",cases);return 0;
}
