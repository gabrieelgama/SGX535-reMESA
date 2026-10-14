/* SPDX-License-Identifier: MIT
 * Original symbolic launch ownership tests. No hardware bytes or submissions.
 */
#include "remesa_pds.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static unsigned checks;
#define CHECK(x) do { assert(x); checks++; } while (0)

static struct remesa_usse_program program(void)
{
 struct remesa_usse_program p={.schema=REMESA_USSE_SCHEMA,
  .stage=REMESA_IR_VERTEX,.count=8};
 for(unsigned c=0;c<4;c++) {
  p.instructions[c]=(struct remesa_usse_instruction){
   .op=REMESA_USSE_INPUT,.dst=c,.src=REMESA_USSE_NO_VALUE,.channel=c};
  p.instructions[4+c]=(struct remesa_usse_instruction){
   .op=REMESA_USSE_EXPORT_POSITION,.dst=REMESA_USSE_NO_VALUE,.src=c,.channel=c};
 }
 return p;
}
static struct remesa_pds_request request(void)
{
 struct remesa_pds_request r={.operation=1,.generation=2,.owner_cookie=3,
  .code={.allocation_cookie=4,.owner_cookie=3,.mapping_generation=5,
   .address=UINT64_C(0x10000),.offset=64,.bytes=256,.allocation_bytes=4096},
  .data={.allocation_cookie=6,.owner_cookie=3,.mapping_generation=7,
   .address=UINT64_C(0x20000),.offset=128,.bytes=64,.allocation_bytes=4096}};
 return r;
}
static void rejected(const struct remesa_usse_program *p,
                     const struct remesa_pds_request *r,int error)
{
 struct remesa_pds_plan out,before;
 memset(&out,0xa5,sizeof(out));before=out;
 struct remesa_pds_request snapshot=*r;
 CHECK(remesa_pds_prepare(p,r,&out)==error);
 CHECK(!memcmp(&out,&before,sizeof(out)));
 CHECK(!memcmp(r,&snapshot,sizeof(snapshot)));
}
static void accepted(const struct remesa_usse_program *p,
                     const struct remesa_pds_request *r)
{
 struct remesa_pds_plan a,b;
 struct remesa_pds_request snapshot=*r;
 CHECK(!remesa_pds_prepare(p,r,&a));
 CHECK(!remesa_pds_prepare(p,r,&b));
 CHECK(!memcmp(&a,&b,sizeof(a)));
 CHECK(!memcmp(r,&snapshot,sizeof(snapshot)));
 CHECK(!remesa_pds_validate(p,&a));
 CHECK(!remesa_pds_validate_binding(p,&a,r));
 unsigned char bytes[32],before[32];size_t written=29;
 memset(bytes,0xa5,sizeof(bytes));memcpy(before,bytes,sizeof(bytes));
 CHECK(remesa_pds_encode(p,&a,bytes,sizeof(bytes),&written)==ENOTSUP);
 CHECK(written==29&&!memcmp(bytes,before,sizeof(bytes)));
}
static struct remesa_pds_block_request block_request(void)
{
 struct remesa_pds_request r=request();
 return (struct remesa_pds_block_request){.operation=r.operation,
  .generation=r.generation,.allocation=r.code,.data_bytes=13,
  .code_bytes=48,.code_alignment=16};
}
static void block_rejected(const struct remesa_pds_block_request *block,int error)
{
 struct remesa_pds_request out,before;
 struct remesa_pds_block_request snapshot=*block;
 memset(&out,0xa5,sizeof(out));before=out;
 CHECK(remesa_pds_request_from_block(block,&out)==error);
 CHECK(!memcmp(&out,&before,sizeof(out)));
 CHECK(!memcmp(block,&snapshot,sizeof(snapshot)));
}
static void block_layouts(const struct remesa_usse_program *p)
{
 /* Independent literal expectations check absolute address alignment and
  * window boundaries. The host padding is not a GPU fetch offset. */
 const struct { uint64_t base,offset,data,alignment,code_offset; } vectors[]={
  {0x10000,64,13,16,80}, {0x10003,64,13,16,77},
  {0x10000,1,1,1,2}, {0x10000,3,1,4,4},
  {0x10000,7,1,8,8}, {0x10000,0,32,32,32},
  {0x10000,63,2,64,128}, {0x10000,127,1,128,128}
 };
 for(unsigned n=0;n<sizeof(vectors)/sizeof(vectors[0]);n++) {
  struct remesa_pds_block_request block=block_request();
  block.allocation.address=vectors[n].base;
  block.allocation.offset=vectors[n].offset;block.allocation.bytes=256;
  block.data_bytes=vectors[n].data;block.code_alignment=vectors[n].alignment;
  struct remesa_pds_request r,a;
  struct remesa_pds_block_request snapshot=block;
  CHECK(!remesa_pds_request_from_block(&block,&r));
  CHECK(!remesa_pds_request_from_block(&block,&a));
  CHECK(!memcmp(&r,&a,sizeof(r)));
  CHECK(!memcmp(&block,&snapshot,sizeof(block)));
  CHECK(r.code.offset==vectors[n].code_offset);
  CHECK(r.data.offset==vectors[n].offset&&r.data.bytes==vectors[n].data);
  CHECK(r.code.bytes==48&&r.code.allocation_cookie==r.data.allocation_cookie);
  CHECK(r.code.mapping_generation==r.data.mapping_generation);
  CHECK(r.code.address==r.data.address&&r.code.allocation_bytes==r.data.allocation_bytes);
  accepted(p,&r);
 }
 struct remesa_pds_block_request block=block_request();
 block.allocation.bytes=64;block.data_bytes=16;block.code_bytes=48;
 struct remesa_pds_request r;
 CHECK(!remesa_pds_request_from_block(&block,&r));accepted(p,&r);
 block.code_bytes++;block_rejected(&block,EOVERFLOW);
 block=block_request();block.allocation.bytes=60;block_rejected(&block,EOVERFLOW);
 block=block_request();block.data_bytes=257;block_rejected(&block,EOVERFLOW);
 block=block_request();block.code_bytes=UINT64_MAX;block_rejected(&block,EOVERFLOW);
 block=block_request();block.code_alignment=UINT64_C(1)<<63;block_rejected(&block,EOVERFLOW);
 block=block_request();block.allocation.address=UINT64_MAX-4096;
 block.allocation.offset=4000;block.allocation.bytes=96;block.data_bytes=80;
 block.code_bytes=1;block.code_alignment=256;block_rejected(&block,EOVERFLOW);
 for(unsigned n=0;n<8;n++) {
  block=block_request();
  if(n==0)block.operation=0;
  if(n==1)block.generation=0;
  if(n==2)block.allocation.owner_cookie=0;
  if(n==3)block.data_bytes=0;
  if(n==4)block.code_bytes=0;
  if(n==5)block.code_alignment=0;
  if(n==6)block.code_alignment=3;
  if(n==7)block.allocation.allocation_cookie=0;
  block_rejected(&block,EINVAL);
 }
 block=block_request();struct remesa_pds_block_request saved=block;
 CHECK(remesa_pds_request_from_block(&block,(struct remesa_pds_request *)&block)==EINVAL);
 CHECK(!memcmp(&block,&saved,sizeof(block)));
 CHECK(remesa_pds_request_from_block(NULL,&r)==EINVAL);
 CHECK(remesa_pds_request_from_block(&block,NULL)==EINVAL);
 /* Fresh ownership metadata must still equal the partition plan exactly. */
 CHECK(!remesa_pds_request_from_block(&block,&r));
 struct remesa_pds_plan plan;
 CHECK(!remesa_pds_prepare(p,&r,&plan));
 r.code.mapping_generation++;r.data.mapping_generation++;
 CHECK(remesa_pds_validate_binding(p,&plan,&r)==ESTALE);
}
static void link_rejected(const struct remesa_pds_plan plans[2],
 const struct remesa_pds_binding programs[2],const uint32_t banks[2],
 const uint64_t offsets[2],int error)
{
 struct remesa_pds_use_link out[2],before[2];memset(out,0xa5,sizeof(out));
 memcpy(before,out,sizeof(out));
 CHECK(remesa_pds_prepare_use_links(plans,programs,banks,offsets,out)==error);
 CHECK(!memcmp(out,before,sizeof(out)));
}
static void use_links(void)
{
 struct remesa_usse_program p[2]={program(),program()};p[1].stage=REMESA_IR_FRAGMENT;
 for(unsigned c=0;c<4;c++) {
  p[1].instructions[c].op=REMESA_USSE_CONSTANT;p[1].instructions[c].channel=0;
  p[1].instructions[c].bits=c==1||c==3?UINT32_C(0x3f800000):0;
  p[1].instructions[4+c].op=REMESA_USSE_EXPORT_COLOR;
 }
 struct remesa_pds_request requests[2];struct remesa_pds_plan plans[2];
 for(unsigned i=0;i<2;i++) {
  requests[i]=request();requests[i].code.address=0x20000000+4096*i;
  requests[i].code.allocation_cookie=64+i;requests[i].code.bytes=32;
  requests[i].data=requests[i].code;requests[i].data.offset=0;requests[i].data.bytes=32;
  CHECK(!remesa_pds_prepare(&p[i],&requests[i],&plans[i]));
 }
 struct remesa_pds_binding programs[2]={
  {.allocation_cookie=32,.owner_cookie=3,.mapping_generation=8,.address=0x10000000,
   .bytes=32,.allocation_bytes=4096},
  {.allocation_cookie=32,.owner_cookie=3,.mapping_generation=8,.address=0x10000000,
   .offset=32,.bytes=8,.allocation_bytes=4096}
 };
 uint32_t banks[2]={3,4};uint64_t offsets[2]={0,28};
 struct remesa_pds_use_link links[2],copy[2];
 CHECK(!remesa_pds_prepare_use_links(plans,programs,banks,offsets,links));
 CHECK(links[0].entry.entry_address==0x10000000&&links[0].entry.program_end==0x10000020);
 CHECK(links[1].entry.entry_address==0x10000020&&links[1].entry.program_end==0x10000028);
 CHECK(links[0].data_address==0x20000000&&links[1].data_address==0x2000101c);
 CHECK(links[0].address_bits==3&&links[1].address_bits==4+(32/16)*256);
 CHECK(!remesa_pds_validate_use_links(plans,programs,links));
 for(unsigned i=0;i<2;i++) {
  struct remesa_use_entry_fixup_fields decoded;
  CHECK(!remesa_use_entry_decode_fixup(links[i].address_bits,&decoded));
  CHECK(links[i].entry.base_address+decoded.entry_byte_offset==
        programs[i].address+programs[i].offset&&decoded.bank==banks[i]);
 }
 for(unsigned n=0;n<12;n++) {
  struct remesa_pds_binding bad[2]={programs[0],programs[1]};int error=EINVAL;
  if(n==0)bad[1].owner_cookie++;
  if(n==1)bad[1].bytes=0;
  if(n==2)bad[1].offset=4096,error=EOVERFLOW;
  if(n==3)bad[1].address=UINT64_MAX,error=EOVERFLOW;
  if(n==4)bad[1].mapping_generation++,error=ESTALE;
  if(n==5)bad[1].allocation_cookie++,error=EINVAL;
  if(n==6)bad[0].address=bad[1].address=plans[1].request.code.address;
  if(n==7)bad[1].offset=0;
  if(n==8)bad[1].allocation_bytes=2048,error=ESTALE;
  if(n==9)bad[0].allocation_cookie=bad[1].allocation_cookie=plans[0].request.code.allocation_cookie,
           error=ESTALE;
  if(n==10)bad[0].address=bad[1].address=0x10000001;
  if(n==11)bad[1].allocation_bytes=UINT64_MAX,error=EOVERFLOW;
  link_rejected(plans,bad,banks,offsets,error);
 }
 const uint64_t bad_offsets[]={1,29,32,UINT64_MAX};
 for(unsigned n=0;n<4;n++) {
  uint64_t bad[2]={0,bad_offsets[n]};link_rejected(plans,programs,banks,bad,EINVAL);
 }
 banks[1]=banks[0];link_rejected(plans,programs,banks,offsets,EBUSY);
 banks[1]=16;link_rejected(plans,programs,banks,offsets,ERANGE);banks[1]=4;
 for(unsigned n=0;n<10;n++) {
  memcpy(copy,links,sizeof(copy));
  if(n==0)copy[1].address_bits|=1u<<19;
  if(n==1)copy[1].data_address++;
  if(n==2)copy[1].entry.program_end++;
  if(n==3)copy[1].request.data.mapping_generation++;
  if(n==4)copy[1].program.mapping_generation++;
  if(n==5)copy[1].program_identity++;
  if(n==6)copy[1].stage=REMESA_IR_VERTEX;
  if(n==7)copy[1].data_byte_offset-=4;
  if(n==8)copy[1].request.operation++;
  if(n==9)copy[1].entry.register_word^=1u<<25;
  CHECK(remesa_pds_validate_use_links(plans,programs,copy)==ESTALE);
 }
 struct remesa_pds_plan changed[2]={plans[0],plans[1]};
 changed[1].request.operation++;
 link_rejected(changed,programs,banks,offsets,ESTALE);
 changed[1]=plans[0];link_rejected(changed,programs,banks,offsets,EINVAL);
 CHECK(remesa_pds_prepare_use_links(plans,programs,banks,offsets,(void *)plans)==EINVAL);
 union {struct remesa_pds_binding program[2];struct remesa_pds_use_link link[2];} alias,saved_alias;
 memset(&alias,0,sizeof(alias));memcpy(alias.program,programs,sizeof(programs));saved_alias=alias;
 CHECK(remesa_pds_prepare_use_links(plans,alias.program,banks,offsets,alias.link)==EINVAL);
 CHECK(!memcmp(&alias,&saved_alias,sizeof(alias)));
 CHECK(remesa_pds_prepare_use_links((void *)(UINTPTR_MAX-15),programs,banks,offsets,copy)==EINVAL);
 CHECK(remesa_pds_prepare_use_links(plans,programs,banks,offsets,(void *)(UINTPTR_MAX-15))==EINVAL);
 CHECK(remesa_pds_prepare_use_links(NULL,programs,banks,offsets,copy)==EINVAL);
 CHECK(remesa_pds_validate_use_links(plans,programs,NULL)==EINVAL);
 CHECK(!remesa_pds_validate_use_links(plans,programs,links));
 struct remesa_pds_encoding_status status;memset(&status,0xa5,sizeof(status));
 remesa_pds_encoding_status(&status);
 CHECK(status.opcode==REMESA_ENCODING_UNKNOWN&&status.task_control==REMESA_ENCODING_UNKNOWN&&
       status.launch_semantics==REMESA_ENCODING_UNKNOWN);
}

int main(void)
{
 struct remesa_usse_program p=program();
 use_links();
 CHECK(!remesa_usse_validate(&p));
 block_layouts(&p);
 struct remesa_pds_request r=request();accepted(&p,&r);

 /* Distinct retained allocation identities cannot cover the same address
  * extent even when the two selected storage windows are disjoint. */
 for(unsigned mode=0;mode<5;mode++) {
  r=request();r.data.address=r.code.address;r.data.offset=1024;
  if(mode==1)r.data.address+=2048;
  if(mode==2)r.data.address-=2048;
  if(mode==3)r.data.allocation_bytes=8192;
  if(mode==4)r.data.address+=4095;
  rejected(&p,&r,EINVAL);
 }
 /* Adjacent full allocations do not alias, irrespective of selected offset. */
 r=request();r.data.address=r.code.address+r.code.allocation_bytes;accepted(&p,&r);
 r=request();r.code.address=r.data.address+r.data.allocation_bytes;accepted(&p,&r);

 /* Two roles can share one retained allocation with identical mapping
  * metadata and disjoint storage, including immediately adjacent windows. */
 r=request();r.data=r.code;r.data.offset=r.code.offset+r.code.bytes;
 r.data.bytes=64;accepted(&p,&r);
 r.data.offset--;rejected(&p,&r,EINVAL);
 r.data.offset=0;r.data.bytes=r.code.offset;accepted(&p,&r);
 r.data.bytes++;rejected(&p,&r,EINVAL);
 for(unsigned mode=0;mode<3;mode++) {
  r=request();r.data=r.code;r.data.offset=1024;
  if(mode==0)r.data.mapping_generation++;
  if(mode==1)r.data.address+=4096;
  if(mode==2)r.data.allocation_bytes+=4096;
  rejected(&p,&r,ESTALE);
 }
 /* No allocation is acquired here; absent caller-owned allocation metadata
  * and invalid extents must fail before changing an output or input. */
 r=request();r.code.allocation_cookie=0;rejected(&p,&r,EINVAL);
 r=request();r.data.allocation_bytes=0;rejected(&p,&r,EINVAL);
 r=request();r.data.address=UINT64_MAX-64;rejected(&p,&r,EOVERFLOW);
 r=request();r.data.offset=4096;rejected(&p,&r,EOVERFLOW);
 r=request();r.data.bytes=UINT64_MAX;rejected(&p,&r,EOVERFLOW);
 /* Valid near-limit allocations preserve the half-open endpoint contract. */
 r=request();r.data.address=UINT64_MAX-4096;r.data.offset=4095;
 r.data.bytes=1;accepted(&p,&r);

 /* A stored plan and fresh caller metadata are checked for the same aliases,
  * not merely initial construction. A forged matching relocation cannot
  * make a conflicting declared ownership identity valid. */
 r=request();struct remesa_pds_plan plan,before;
 CHECK(!remesa_pds_prepare(&p,&r,&plan));
 r.data.address=r.code.address;r.data.offset=1024;
 CHECK(remesa_pds_validate_binding(&p,&plan,&r)==EINVAL);
 plan.request=r;
 plan.relocations[1].address=r.data.address+r.data.offset;
 before=plan;
 CHECK(remesa_pds_validate(&p,&plan)==EINVAL);
 CHECK(!memcmp(&plan,&before,sizeof(plan)));
 unsigned char bytes[32],saved[32];size_t written=17;
 memset(bytes,0xa5,sizeof(bytes));memcpy(saved,bytes,sizeof(bytes));
 CHECK(remesa_pds_encode(&p,&plan,bytes,sizeof(bytes),&written)==EINVAL);
 CHECK(written==17&&!memcmp(bytes,saved,sizeof(bytes)));
 printf("PASS %u symbolic launch ownership checks; zero hardware execution\n",checks);
 return 0;
}
