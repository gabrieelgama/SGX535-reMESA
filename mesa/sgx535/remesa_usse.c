/* SPDX-License-Identifier: MIT; independently authored host backend schema. */
#include "remesa_usse.h"
#include "remesa_nir.h"
#include <errno.h>
#include <string.h>
static int overlaps(const void *a,size_t na,const void *b,size_t nb)
{
 if(!na||!nb)return 0;
 uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
 return x<=y?y-x<na:x-y<nb;
}
struct scalar { uint64_t used;uint32_t input,channel,bits; };
static int zero_storage(const void *p,size_t n)
{const unsigned char *s=p;for(size_t k=0;k<n;k++)if(s[k])return 0;return 1;}
int remesa_usse_validate(const struct remesa_usse_program *p)
{
 struct scalar values[REMESA_USSE_VALUES]={0};uint32_t defined=0,exports=0;
 uint64_t used=0;uint32_t colors[4]={0};
 if(!p)return EINVAL;
 if(p->schema!=REMESA_USSE_SCHEMA||(p->stage!=REMESA_IR_VERTEX&&p->stage!=REMESA_IR_FRAGMENT)||
    !p->count||p->count>REMESA_USSE_INSTRUCTIONS)return ENOTSUP;
 for(uint32_t n=0;n<p->count;n++) {
  const struct remesa_usse_instruction *i=&p->instructions[n];struct scalar v={0};
  if(i->reserved)return ENOTSUP;
  if(i->op==REMESA_USSE_EXPORT_POSITION||i->op==REMESA_USSE_EXPORT_COLOR) {
   if(i->dst!=REMESA_USSE_NO_VALUE||i->src>=defined||i->bits||i->channel!=exports||exports>=4)return ENOTSUP;
   v=values[i->src];used|=v.used;
   if(i->op==REMESA_USSE_EXPORT_POSITION) {
    if(p->stage!=REMESA_IR_VERTEX||!v.input||v.channel!=exports)return ENOTSUP;
   } else {
    if(p->stage!=REMESA_IR_FRAGMENT||v.input)return ENOTSUP;
    colors[exports]=v.bits;
   }
   exports++;continue;
  }
  if(exports||defined>=REMESA_USSE_VALUES||i->dst!=defined)return ENOTSUP;
  v.used=UINT64_C(1)<<defined;
  switch(i->op) {
  case REMESA_USSE_INPUT:
   if(p->stage!=REMESA_IR_VERTEX||defined>=4||i->channel!=defined||i->src!=REMESA_USSE_NO_VALUE||i->bits)return ENOTSUP;
   v.input=1;v.channel=i->channel;break;
  case REMESA_USSE_CONSTANT:
   if(p->stage!=REMESA_IR_FRAGMENT||i->channel||i->src!=REMESA_USSE_NO_VALUE||
      (i->bits&&i->bits!=UINT32_C(0x3f800000)))return ENOTSUP;
   v.bits=i->bits;break;
  case REMESA_USSE_MOVE:
   if(i->src>=defined||i->channel||i->bits)return ENOTSUP;
   v=values[i->src];v.used|=UINT64_C(1)<<defined;break;
  default:return ENOTSUP;
  }
  values[defined++]=v;
 }
 if(exports!=4||!defined||used!=(defined==64?UINT64_MAX:((UINT64_C(1)<<defined)-1)))return ENOTSUP;
 if(p->stage==REMESA_IR_FRAGMENT) {
  uint32_t color;int ret=remesa_constant_rgba32(colors,&color);if(ret)return ret;
 }
 if(!zero_storage(&p->instructions[p->count],
                  (REMESA_USSE_INSTRUCTIONS-p->count)*sizeof(p->instructions[0])))return ENOTSUP;
 return 0;
}
int remesa_usse_lower(const struct remesa_shader_ir *ir,struct remesa_usse_program *out)
{
 struct remesa_usse_program p={.schema=REMESA_USSE_SCHEMA};
 uint32_t map[REMESA_SHADER_IR_VALUES][4]={{0}},defined=0;
 if(!out||!ir||overlaps(out,sizeof(*out),ir,sizeof(*ir)))return EINVAL;
 int ret=remesa_shader_ir_validate(ir);if(ret)return ret;
 p.stage=ir->stage;
 for(uint32_t n=0;n<ir->count;n++) {
  const struct remesa_ir_instruction *i=&ir->instructions[n];
  /* COPY/COMPOSE4 are pure float32 channel aliases in the admitted IR.
   * Propagate them before allocating scalar values. This is an original
   * semantic optimization, not a claim about hardware registers or MOV bits.
   * Validation above rejects side effects, cycles and forward references. */
  if(i->op==REMESA_IR_COPY||i->op==REMESA_IR_COMPOSE4) {
   for(uint32_t c=0;c<i->components;c++)
    map[i->dst][c]=map[i->src[c].reg][i->src[c].channel];
   continue;
  }
  for(uint32_t c=0;c<i->components;c++) {
   if(p.count>=REMESA_USSE_INSTRUCTIONS)return EOVERFLOW;
   struct remesa_usse_instruction *u=&p.instructions[p.count++];
   u->dst=REMESA_USSE_NO_VALUE;u->src=REMESA_USSE_NO_VALUE;
   if(i->op==REMESA_IR_POSITION||i->op==REMESA_IR_COLOR) {
    u->op=i->op==REMESA_IR_POSITION?REMESA_USSE_EXPORT_POSITION:REMESA_USSE_EXPORT_COLOR;
    u->src=map[i->src[0].reg][c];u->channel=c;
   } else {
    if(defined>=REMESA_USSE_VALUES)return EOVERFLOW;
    u->dst=defined;map[i->dst][c]=defined++;
    if(i->op==REMESA_IR_INPUT) {u->op=REMESA_USSE_INPUT;u->channel=c;}
    else if(i->op==REMESA_IR_CONSTANT) {u->op=REMESA_USSE_CONSTANT;u->bits=i->bits[c];}
    else return ENOTSUP;
   }
  }
 }
 ret=remesa_usse_validate(&p);if(ret)return ret;
 *out=p;return 0;
}
static void put32(unsigned char *p,uint32_t n)
{for(unsigned k=0;k<4;k++)p[k]=(unsigned char)(n>>(k*8));}
static uint32_t get32(const unsigned char *p)
{uint32_t n=0;for(unsigned k=0;k<4;k++)n|=(uint32_t)p[k]<<(k*8);return n;}
int remesa_usse_serialize_host(const struct remesa_usse_program *p,void *out,size_t cap,size_t *written)
{
 if(!written||(!out&&cap))return EINVAL;
 int ret=remesa_usse_validate(p);if(ret)return ret;
 size_t bytes=REMESA_USSE_HOST_HEADER_BYTES+p->count*REMESA_USSE_HOST_INSTRUCTION_BYTES;
 if(cap<bytes)return ENOSPC;
 if(overlaps(out,bytes,p,sizeof(*p))||overlaps(written,sizeof(*written),p,sizeof(*p))||
    overlaps(written,sizeof(*written),out,bytes))return EINVAL;
 unsigned char *s=out;
 /* RUSI is original host identification, NOT hardware program magic. */
 put32(s,UINT32_C(0x49535552));put32(s+4,p->schema);put32(s+8,p->stage);put32(s+12,p->count);
 for(uint32_t n=0;n<p->count;n++) {
  const struct remesa_usse_instruction *i=&p->instructions[n];unsigned char *d=s+16+n*24;
  put32(d,i->op);put32(d+4,i->dst);put32(d+8,i->src);put32(d+12,i->channel);put32(d+16,i->bits);put32(d+20,i->reserved);
 }
 *written=bytes;return 0;
}
int remesa_usse_deserialize_host(const void *data,size_t bytes,struct remesa_usse_program *out)
{
 if(!data||!out||overlaps(data,bytes,out,sizeof(*out)))return EINVAL;
 if(bytes<REMESA_USSE_HOST_HEADER_BYTES)return EINVAL;
 const unsigned char *s=data;struct remesa_usse_program p={0};
 if(get32(s)!=UINT32_C(0x49535552))return ENOTSUP;
 p.schema=get32(s+4);p.stage=(enum remesa_ir_stage)get32(s+8);p.count=get32(s+12);
 if(!p.count||p.count>REMESA_USSE_INSTRUCTIONS||bytes!=16+p.count*24u)return EINVAL;
 for(uint32_t n=0;n<p.count;n++) {
  struct remesa_usse_instruction *i=&p.instructions[n];const unsigned char *d=s+16+n*24;
  i->op=get32(d);i->dst=get32(d+4);i->src=get32(d+8);i->channel=get32(d+12);i->bits=get32(d+16);i->reserved=get32(d+20);
 }
 int ret=remesa_usse_validate(&p);if(ret)return ret;
 *out=p;return 0;
}
int remesa_usse_identity(const struct remesa_usse_program *p,uint64_t *identity)
{
 unsigned char bytes[16+REMESA_USSE_INSTRUCTIONS*24];size_t written=0;
 if(!identity||!p||overlaps(identity,sizeof(*identity),p,sizeof(*p)))return EINVAL;
 int ret=remesa_usse_serialize_host(p,bytes,sizeof(bytes),&written);if(ret)return ret;
 /* Original host checksum only. Not hardware evidence or cryptography. */
 uint64_t h=0;
 for(size_t n=0;n<written;n++){h=(h<<7)|(h>>57);h^=(uint64_t)bytes[n]+n;}
 *identity=h;return 0;
}
void remesa_usse_encoding_status(struct remesa_usse_encoding_status *status)
{if(status)memset(status,0,sizeof(*status));}
int remesa_usse_encode(const struct remesa_usse_program *p,void *out,size_t cap,size_t *written)
{
 if(!written||(!out&&cap))return EINVAL;
 int ret=remesa_usse_validate(p);return ret?ret:ENOTSUP;
}
