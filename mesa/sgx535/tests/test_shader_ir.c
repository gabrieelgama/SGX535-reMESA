/* SPDX-License-Identifier: MIT; independently authored semantic IR tests.
 * Actual Mesa NIR structs in vertex fixture; no GPU or hardware encoding. */
#include "remesa_shader_ir.h"
#include "vertex_nir_fixture.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
static struct remesa_shader_ir vertex(void)
{
 struct remesa_shader_ir p={.schema=1,.stage=REMESA_IR_VERTEX,.count=2};
 p.instructions[0]=(struct remesa_ir_instruction){.op=REMESA_IR_INPUT,.type=REMESA_IR_FLOAT32,.components=4};
 p.instructions[1]=(struct remesa_ir_instruction){.op=REMESA_IR_POSITION,.type=REMESA_IR_FLOAT32,.components=4,.dst=REMESA_SHADER_IR_NO_REGISTER};
 return p;
}
int main(void)
{
 unsigned cases=0;struct remesa_shader_ir p=vertex();
 assert(!remesa_shader_ir_validate(&p));cases++;
 unsigned slot=99;uint32_t argb=123;
 assert(!remesa_shader_ir_vertex_input(&p,&slot)&&slot==0);cases++;
 assert(remesa_shader_ir_fragment_color(&p,&argb)==ENOTSUP&&argb==123);cases++;
 char dump[4096],other[4096];size_t needed=999,required=999;
 assert(!remesa_shader_ir_dump(&p,dump,sizeof(dump),&needed));
 assert(!remesa_shader_ir_dump(&p,other,sizeof(other),&required));
 assert(needed==strlen(dump)+1&&required==needed&&!strcmp(dump,other));
 assert(strstr(dump,"NOT SGX instructions"));cases++;
 memset(other,0x55,sizeof(other));assert(remesa_shader_ir_dump(&p,other,needed-1,&required)==ENOSPC);
 for(unsigned n=0;n<sizeof(other);n++)assert((unsigned char)other[n]==0x55);cases++;
 assert(remesa_shader_ir_dump(&p,NULL,0,&required)==ENOSPC&&required==needed);cases++;
 unsigned char bytes[16];memset(bytes,0xa5,sizeof(bytes));size_t written=99;
 assert(remesa_shader_ir_encode_sgx535(&p,bytes,sizeof(bytes),&written)==ENOTSUP&&written==99);
 for(unsigned n=0;n<sizeof(bytes);n++)assert(bytes[n]==0xa5);cases++;
 for(unsigned mode=0;mode<24;mode++) {
  p=vertex();switch(mode) {
  case 0:p.schema=0;break;case 1:p.stage=0;break;case 2:p.count=0;break;
  case 3:p.count=REMESA_SHADER_IR_INSTRUCTIONS+1;break;
  case 4:p.instructions[0].dst=1;break;case 5:p.instructions[0].components=3;break;
  case 6:p.instructions[0].type=0;break;case 7:p.instructions[0].input_slot=1;break;
  case 8:p.instructions[0].src[0].channel=1;break;case 9:p.instructions[0].bits[0]=1;break;
  case 10:p.instructions[1].dst=0;break;case 11:p.instructions[1].components=3;break;
  case 12:p.instructions[1].src[0].reg=1;break;case 13:p.instructions[1].src[0].channel=1;break;
  case 14:p.instructions[1].src[1].reg=1;break;case 15:p.instructions[1].op=REMESA_IR_COLOR;break;
  case 16:p.instructions[1].bits[0]=1;break;case 17:p.count=3;break;
  case 18:p.instructions[2].op=REMESA_IR_CONSTANT;break;
  case 19:p.instructions[1].op=999;break;case 20:p.instructions[0].op=REMESA_IR_COPY;break;
  case 21:p.stage=REMESA_IR_FRAGMENT;break;case 22:p.instructions[1].type=0;break;
  case 23:p.instructions[1].input_slot=1;break;
  }
  assert(remesa_shader_ir_validate(&p));required=999;memset(other,0x55,sizeof(other));
  assert(remesa_shader_ir_dump(&p,other,sizeof(other),&required)&&required==999&&other[0]==0x55);
  assert(remesa_shader_ir_encode_sgx535(&p,bytes,sizeof(bytes),&written)&&written==99);cases++;
 }
 for(unsigned stage=0;stage<2;stage++)for(unsigned mode=0;mode<10;mode++) {
  p=vertex();p.count=3;
  p.instructions[2]=p.instructions[1];p.instructions[2].src[0].reg=1;
  p.instructions[1]=(struct remesa_ir_instruction){.op=REMESA_IR_COPY,.dst=1,.type=REMESA_IR_FLOAT32,.components=4};
  for(unsigned c=0;c<4;c++)p.instructions[1].src[c].channel=c;
  if(stage) {
   p.stage=REMESA_IR_FRAGMENT;p.instructions[0].op=REMESA_IR_CONSTANT;
   p.instructions[0].bits[0]=p.instructions[0].bits[2]=p.instructions[0].bits[3]=0x3f800000;
   p.instructions[2].op=REMESA_IR_COLOR;
  }
  switch(mode) {
  case 1:p.instructions[1].op=REMESA_IR_COMPOSE4;break;
  case 2:p.instructions[1].src[0].reg=1;break;
  case 3:p.instructions[1].src[0].channel=4;break;
  case 4:p.instructions[1].components=0;break;
  case 5:p.instructions[1].components=5;break;
  case 6:p.instructions[1].src[1].channel=0;break;
  case 7:p.instructions[2].src[0].reg=0;break;
  case 8:p.instructions[1].bits[0]=1;break;
  case 9:p.instructions[1].dst=16;break;
  }
  int ret=remesa_shader_ir_validate(&p);
  assert(mode<2?ret==0:ret!=0);cases++;
 }
 for(unsigned mode=0;mode<7;mode++) {
  p=vertex();p.stage=REMESA_IR_FRAGMENT;p.instructions[0].op=REMESA_IR_CONSTANT;p.instructions[1].op=REMESA_IR_COLOR;
  p.instructions[0].bits[0]=p.instructions[0].bits[2]=p.instructions[0].bits[3]=0x3f800000;
  switch(mode) {
  case 1:p.instructions[0].bits[0]=p.instructions[0].bits[2]=0;p.instructions[0].bits[1]=0x3f800000;break;
  case 2:p.instructions[0].bits[0]=p.instructions[0].bits[1]=0x3f800000;p.instructions[0].bits[2]=0;break;
  case 3:p.instructions[0].bits[3]=0;break;case 4:p.instructions[0].bits[0]=0x3f000000;break;
  case 5:p.instructions[0].bits[0]=0x7fc00000;break;case 6:p.instructions[0].bits[1]=0x3f800000;break;
  }
  assert(mode<3?!remesa_shader_ir_validate(&p):remesa_shader_ir_validate(&p)!=0);cases++;
  slot=99;argb=123;assert(remesa_shader_ir_vertex_input(&p,&slot)&&slot==99);cases++;
  if(mode<3){assert(!remesa_shader_ir_fragment_color(&p,&argb));assert(argb==(mode==0?0xffff00ffu:mode==1?0xff00ff00u:0xffffff00u));}
  else assert(remesa_shader_ir_fragment_color(&p,&argb)&&argb==123);cases++;
 }
 for(unsigned count=16;count<18;count++) {
  p=vertex();p.count=count+1;
  for(unsigned n=1;n<count;n++) {
   p.instructions[n]=(struct remesa_ir_instruction){.op=REMESA_IR_COPY,.dst=n,.type=REMESA_IR_FLOAT32,.components=4};
   for(unsigned c=0;c<4;c++)p.instructions[n].src[c]=(struct remesa_ir_operand){.reg=n-1,.channel=c};
  }
  p.instructions[count]=(struct remesa_ir_instruction){.op=REMESA_IR_POSITION,.dst=REMESA_SHADER_IR_NO_REGISTER,.type=REMESA_IR_FLOAT32,.components=4,.src={{.reg=count-1}}};
  assert(count==16?!remesa_shader_ir_validate(&p):remesa_shader_ir_validate(&p)!=0);cases++;
 }
 p=vertex();written=99;
 assert(remesa_shader_ir_encode_sgx535(&p,NULL,0,&written)==ENOTSUP&&written==99);cases++;
 struct remesa_shader_ir unchanged;memset(&unchanged,0xa5,sizeof(unchanged));p=unchanged;
 assert(remesa_shader_ir_lower(NULL,&p)==EINVAL&&!memcmp(&p,&unchanged,sizeof(p)));cases++;
 nir_shader *s=vertex_fixture();assert(!remesa_shader_ir_lower(s,&p)&&p.stage==REMESA_IR_VERTEX&&p.count==2);cases++;
 s->info.stage=MESA_SHADER_COMPUTE;p=unchanged;assert(remesa_shader_ir_lower(s,&p)&&!memcmp(&p,&unchanged,sizeof(p)));ralloc_free(s);cases++;
 assert(remesa_shader_ir_validate(NULL)==EINVAL);cases++;
 assert(remesa_shader_ir_encode_sgx535(&p,bytes,sizeof(bytes),NULL)==EINVAL);cases++;
 printf("PASS %u semantic IR validation cases; zero hardware bytes generated\n",cases);return 0;
}
