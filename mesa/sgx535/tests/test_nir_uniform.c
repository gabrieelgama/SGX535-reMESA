/* SPDX-License-Identifier: MIT
 * Actual Mesa 25.2 structures, manually constructed offline NIR fixtures.
 * No GPU instruction generation, completion or rasterization. */
#include "remesa_nir_uniform.h"
#include "nir.h"
#include "util/ralloc.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef NDEBUG
#error Assertions must remain enabled
#endif
#define CHECK(x) do {if(!(x)){fprintf(stderr,"check failed: %d\n",__LINE__);abort();}} while(0)
#include "uniform_nir_fixture.h"
int main(void)
{
 unsigned cases=0;struct remesa_uniform_color sentinel={7,123,9},out=sentinel;
 CHECK(remesa_nir_uniform_fragment(NULL,&out)==EINVAL&&!memcmp(&out,&sentinel,sizeof(out)));cases++;
 const unsigned offsets[]={0,16,32,256,1024,2048,4064,4080};
 for(unsigned i=0;i<sizeof(offsets)/sizeof(offsets[0]);i++) {
  unsigned offset=offsets[i];struct fixture f=make(offset);CHECK(!remesa_nir_uniform_fragment(f.s,&out)&&out.slot==0&&out.byte_offset==offset&&out.byte_size==16);ralloc_free(f.s);cases++;
 }
 for(unsigned mode=0;mode<40;mode++) {
  struct fixture f=make(16);out=sentinel;
  switch(mode) {
   case 0:f.s->info.stage=MESA_SHADER_VERTEX;break;
   case 1:f.s->info.io_lowered=false;break;
   case 2:f.s->info.num_ubos=2;break;
   case 3:f.s->info.num_textures=1;break;
   case 4:f.s->info.writes_memory=true;break;
   case 5:f.s->info.fs.uses_discard=true;break;
   case 6:f.s->info.inputs_read=1;break;
   case 7:f.s->info.outputs_written=BITFIELD64_BIT(FRAG_RESULT_DEPTH);break;
   case 8:f.index->value[0].u32=1;break;
   case 9:f.offset->value[0].u32=4;break;
   case 10:f.offset->value[0].u32=4096;break;
   case 11:f.load->def.bit_size=16;break;
   case 12:f.load->num_components=3;break;
   case 13:nir_intrinsic_set_align_mul(f.load,4);break;
   case 14:nir_intrinsic_set_align_offset(f.load,4);break;
   case 15:nir_intrinsic_set_range(f.load,32);break;
   case 16:nir_intrinsic_set_range_base(f.load,0);break;
   case 17:nir_intrinsic_set_access(f.load,ACCESS_VOLATILE);break;
   case 18:nir_intrinsic_set_write_mask(f.store,7);break;
   case 19:nir_intrinsic_set_src_type(f.store,nir_type_uint32);break;
   case 20:nir_intrinsic_set_io_semantics(f.store,(nir_io_semantics){.location=FRAG_RESULT_DEPTH,.num_slots=1});break;
   case 21:f.store->src[0].ssa=&f.offset->def;break;
   case 22:f.load->src[1].ssa=NULL;break;
   case 23:f.store->src[1].ssa=&f.offset->def;break;
   case 24:f.load->intrinsic=nir_intrinsic_load_ssbo;break;
   case 25:f.s->info.system_values_read[0]=1;break;
   case 26:imm(&f,99);break;
   case 27:f.s->scratch_size=16;break;
   case 28:f.s->info.num_ubos=0;break;
   case 29:f.s->num_uniforms=1;break;
   case 30:f.s->info.fs.uses_sample_shading=true;break;
   case 31:f.s->info.num_ssbos=1;break;
   case 32:f.load->def.num_components=3;break;
   case 33:f.offset->def.bit_size=64;break;
   case 34:f.store->num_components=3;break;
   case 35:nir_intrinsic_set_component(f.store,1);break;
   case 36:nir_intrinsic_set_io_semantics(f.store,(nir_io_semantics){.location=FRAG_RESULT_DATA0,.num_slots=2});break;
   case 37:f.load->src[0].ssa=&f.load->def;break;
   case 38:f.block->cf_node.type=nir_cf_node_if;break;
   case 39:f.s->info.xfb_stride[0]=4;break;
  }
  CHECK(remesa_nir_uniform_fragment(f.s,&out)!=0&&!memcmp(&out,&sentinel,sizeof(out)));ralloc_free(f.s);cases++;
 }
 unsigned char buffer[4097]={0};uint32_t rgba[4]={0x3f800000,0,0x3f800000,0x3f800000},argb=123;
 out=(struct remesa_uniform_color){0,16,16};memcpy(buffer+17,rgba,16);
 CHECK(!remesa_uniform_color_resolve(&out,buffer+1,32,&argb)&&argb==0xffff00ff);cases++;
 rgba[0]=0;rgba[1]=0x3f800000;rgba[2]=0;memcpy(buffer+17,rgba,16);
 CHECK(!remesa_uniform_color_resolve(&out,buffer+1,32,&argb)&&argb==0xff00ff00);cases++;
 rgba[0]=0x3f800000;memcpy(buffer+17,rgba,16);
 CHECK(!remesa_uniform_color_resolve(&out,buffer+1,32,&argb)&&argb==0xffffff00);cases++;
 for(unsigned mode=0;mode<10;mode++) {
  out=(struct remesa_uniform_color){0,16,16};size_t bytes=32;const void *data=buffer+1;argb=123;
  if(mode==0)bytes=31;if(mode==1)bytes=0;if(mode==2)bytes=4097;if(mode==3)out.slot=1;
  if(mode==4)out.byte_offset=17;if(mode==5)out.byte_size=12;if(mode==6)out.byte_offset=UINT32_MAX;
  if(mode==7)data=NULL;if(mode==8){rgba[0]=0x3f000000;memcpy(buffer+17,rgba,16);}
  if(mode==9){rgba[0]=0x3f800000;rgba[3]=0;memcpy(buffer+17,rgba,16);}
  CHECK(remesa_uniform_color_resolve(&out,data,bytes,&argb)!=0&&argb==123);cases++;
 }
 printf("PASS %u static UBO profile admission/CPU specialization cases; no GPU\n",cases);return 0;
}
