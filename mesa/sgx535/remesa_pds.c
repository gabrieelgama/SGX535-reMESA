/* SPDX-License-Identifier: MIT; no independently established PDS bitfields. */
#include "remesa_pds.h"
#include "remesa_pds_candidate.h"
#include <errno.h>
#include <string.h>
static int overlaps(const void *a,size_t na,const void *b,size_t nb)
{
 if(!na||!nb)return 0;
 uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
 if(na>UINTPTR_MAX-x||nb>UINTPTR_MAX-y)return 1;
 return x<=y?y-x<na:x-y<nb;
}
static int binding_valid(const struct remesa_pds_binding *b,uint64_t owner)
{
 if(!b->allocation_cookie||b->owner_cookie!=owner||!b->mapping_generation||
    !b->address||!b->bytes||!b->allocation_bytes)return EINVAL;
 if(b->offset>=b->allocation_bytes||b->bytes>b->allocation_bytes-b->offset||
    b->allocation_bytes>UINT64_MAX-b->address)return EOVERFLOW;
 return 0;
}
static int request_valid(const struct remesa_pds_request *r)
{
 if(!r||!r->operation||!r->generation||!r->owner_cookie)return EINVAL;
 int ret=binding_valid(&r->code,r->owner_cookie);if(ret)return ret;
 ret=binding_valid(&r->data,r->owner_cookie);if(ret)return ret;
 /* One retained allocation cannot carry contradictory mapping identities.
  * Overlap is rejected so a data update cannot corrupt symbolic code storage.
  * Distinct allocations still cannot alias declared address ranges. */
 if(r->code.allocation_cookie==r->data.allocation_cookie) {
  if(r->code.mapping_generation!=r->data.mapping_generation||
     r->code.address!=r->data.address||r->code.allocation_bytes!=r->data.allocation_bytes)return ESTALE;
 } else {
  /* Distinct retained ownership identities must describe disjoint complete
   * allocations in this request's address space. Selected windows alone do
   * not establish that a data allocation cannot alias code backing. Both
   * endpoints are representable after binding_valid above. */
  if(r->code.address<r->data.address+r->data.allocation_bytes&&
     r->data.address<r->code.address+r->code.allocation_bytes)return EINVAL;
 }
 uint64_t c=r->code.address+r->code.offset,d=r->data.address+r->data.offset;
 if(c<d+r->data.bytes&&d<c+r->code.bytes)return EINVAL;
 return 0;
}
int remesa_pds_request_from_block(const struct remesa_pds_block_request *block,
                                 struct remesa_pds_request *out)
{
 if(!block||!out||overlaps(block,sizeof(*block),out,sizeof(*out)))return EINVAL;
 if(!block->operation||!block->generation||!block->allocation.owner_cookie||
    !block->data_bytes||!block->code_bytes)return EINVAL;
 uint64_t alignment=block->code_alignment;
 if(!alignment||(alignment&(alignment-1)))return EINVAL;
 int ret=binding_valid(&block->allocation,block->allocation.owner_cookie);if(ret)return ret;
 const struct remesa_pds_binding *b=&block->allocation;
 if(block->data_bytes>b->bytes)return EOVERFLOW;
 /* Binding bounds make start + data_bytes representable. Padding is checked
  * separately so alignment cannot wrap into another allocation/window. */
 uint64_t start=b->address+b->offset,after_data=start+block->data_bytes;
 uint64_t padding=(alignment-(after_data&(alignment-1)))&(alignment-1);
 if(padding>UINT64_MAX-after_data)return EOVERFLOW;
 uint64_t code_address=after_data+padding,used=block->data_bytes;
 if(padding>b->bytes-used)return EOVERFLOW;
 used+=padding;
 if(block->code_bytes>b->bytes-used)return EOVERFLOW;
 struct remesa_pds_request r={.operation=block->operation,
  .generation=block->generation,.owner_cookie=b->owner_cookie,
  .code=*b,.data=*b};
 r.data.bytes=block->data_bytes;
 r.code.offset=code_address-b->address;r.code.bytes=block->code_bytes;
 ret=request_valid(&r);if(ret)return ret;
 *out=r;return 0;
}
int remesa_pds_prepare(const struct remesa_usse_program *program,const struct remesa_pds_request *request,struct remesa_pds_plan *out)
{
 if(!out||!program||!request||overlaps(out,sizeof(*out),program,sizeof(*program))||
    overlaps(out,sizeof(*out),request,sizeof(*request)))return EINVAL;
 uint64_t identity;int ret=remesa_usse_identity(program,&identity);if(ret)return ret;
 ret=request_valid(request);if(ret)return ret;
 struct remesa_pds_plan p={.schema=REMESA_PDS_SCHEMA,.stage=program->stage,
  .program_identity=identity,.program=*program,.request=*request,.relocation_count=REMESA_PDS_RELOCATIONS};
 p.relocations[0].role=REMESA_PDS_CODE;p.relocations[1].role=REMESA_PDS_DATA;
 p.relocations[0].kind=p.relocations[1].kind=REMESA_PDS_SYMBOLIC_ADDRESS;
 p.relocations[0].address=request->code.address+request->code.offset;
 p.relocations[0].bytes=request->code.bytes;
 p.relocations[1].address=request->data.address+request->data.offset;
 p.relocations[1].bytes=request->data.bytes;
 *out=p;return 0;
}
int remesa_pds_validate(const struct remesa_usse_program *program,const struct remesa_pds_plan *plan)
{
 if(!plan)return EINVAL;
 uint64_t identity;int ret=remesa_usse_identity(program,&identity);if(ret)return ret;
 if(plan->schema!=REMESA_PDS_SCHEMA||plan->reserved||
    plan->relocation_count!=REMESA_PDS_RELOCATIONS)return ENOTSUP;
 ret=remesa_usse_validate(&plan->program);if(ret)return ret;
 if(plan->program_identity!=identity||plan->stage!=program->stage||
    plan->program.schema!=program->schema||plan->program.stage!=program->stage||
    plan->program.count!=program->count||
    memcmp(plan->program.instructions,program->instructions,sizeof(program->instructions)))return ESTALE;
 ret=request_valid(&plan->request);if(ret)return ret;
 for(unsigned n=0;n<REMESA_PDS_RELOCATIONS;n++) {
  const struct remesa_pds_relocation *r=&plan->relocations[n];
  const struct remesa_pds_binding *b=n?&plan->request.data:&plan->request.code;
  if(r->role!=(n?REMESA_PDS_DATA:REMESA_PDS_CODE)||r->kind!=REMESA_PDS_SYMBOLIC_ADDRESS||
     r->address!=b->address+b->offset||r->bytes!=b->bytes)return EINVAL;
 }
 return 0;
}
static int binding_equal(const struct remesa_pds_binding *a,const struct remesa_pds_binding *b)
{
 return a->allocation_cookie==b->allocation_cookie&&a->owner_cookie==b->owner_cookie&&
  a->mapping_generation==b->mapping_generation&&a->address==b->address&&
  a->offset==b->offset&&a->bytes==b->bytes&&a->allocation_bytes==b->allocation_bytes;
}
int remesa_pds_validate_binding(const struct remesa_usse_program *program,const struct remesa_pds_plan *plan,const struct remesa_pds_request *current)
{
 int ret=remesa_pds_validate(program,plan);if(ret)return ret;
 ret=request_valid(current);if(ret)return ret;
 const struct remesa_pds_request *old=&plan->request;
 if(old->operation!=current->operation||old->generation!=current->generation||
    old->owner_cookie!=current->owner_cookie||!binding_equal(&old->code,&current->code)||
    !binding_equal(&old->data,&current->data))return ESTALE;
 return 0;
}
/* Call only after both bindings have passed overflow/bounds validation. */
static int storage_disjoint(const struct remesa_pds_binding *a,
                            const struct remesa_pds_binding *b)
{
 if(a->allocation_cookie==b->allocation_cookie) {
  if(a->address!=b->address||a->allocation_bytes!=b->allocation_bytes||
     a->mapping_generation!=b->mapping_generation)return ESTALE;
 } else if(a->address<b->address+b->allocation_bytes&&
           b->address<a->address+a->allocation_bytes)return EINVAL;
 uint64_t x=a->address+a->offset,y=b->address+b->offset;
 return x<y+b->bytes&&y<x+a->bytes?EINVAL:0;
}
int remesa_pds_prepare_use_links(const struct remesa_pds_plan plans[2],
 const struct remesa_pds_binding programs[2],const uint32_t banks[2],
 const uint64_t offsets[2],struct remesa_pds_use_link links[2])
{
 if(!plans||!programs||!banks||!offsets||!links||
    overlaps(links,sizeof(*links)*2,plans,sizeof(*plans)*2)||
    overlaps(links,sizeof(*links)*2,programs,sizeof(*programs)*2)||
    overlaps(links,sizeof(*links)*2,banks,sizeof(*banks)*2)||
    overlaps(links,sizeof(*links)*2,offsets,sizeof(*offsets)*2))return EINVAL;
 struct remesa_use_entry_request requests[2]={{0}};
 struct remesa_pds_use_link result[2]={0};
 const enum remesa_ir_stage stages[2]={REMESA_IR_VERTEX,REMESA_IR_FRAGMENT};
 for(unsigned i=0;i<2;i++) {
  const struct remesa_pds_request *r=&plans[i].request;
  int error=remesa_pds_validate(&plans[i].program,&plans[i]);if(error)return error;
  if(plans[i].stage!=stages[i])return EINVAL;
  if(r->owner_cookie!=plans[0].request.owner_cookie||
     r->operation!=plans[0].request.operation||
     r->generation!=plans[0].request.generation)return ESTALE;
  error=binding_valid(&programs[i],r->owner_cookie);if(error)return error;
  if(offsets[i]%4||offsets[i]>r->data.bytes||4>r->data.bytes-offsets[i])return EINVAL;
  uint64_t address=r->data.address+r->data.offset+offsets[i];
  if(address%4)return EINVAL;
  requests[i]=(struct remesa_use_entry_request){
   .allocation_address=programs[i].address,.allocation_bytes=programs[i].allocation_bytes,
   .entry_offset=programs[i].offset,.program_bytes=programs[i].bytes,
   .bank=banks[i],.data_master=i==0?REMESA_USE_VERTEX:REMESA_USE_PIXEL
  };
  result[i].request=*r;result[i].program=programs[i];result[i].stage=stages[i];
  result[i].program_identity=plans[i].program_identity;
  result[i].data_byte_offset=offsets[i];result[i].data_address=address;
 }
 /* Matching source handles alone cannot prove disjointness from either PDS
  * allocation. Check complete extents and all selected windows pairwise. */
 const struct remesa_pds_binding *regions[6]={&programs[0],&programs[1],
  &plans[0].request.code,&plans[0].request.data,
  &plans[1].request.code,&plans[1].request.data};
 for(unsigned i=0;i<6;i++)for(unsigned j=i+1;j<6;j++) {
  int error=storage_disjoint(regions[i],regions[j]);if(error)return error;
 }
 struct remesa_use_entry_plan entries[2];
 int error=remesa_use_entry_pair_prepare(requests,entries);if(error)return error;
 for(unsigned i=0;i<2;i++) {
  result[i].entry=entries[i];
  error=remesa_use_entry_preview_fixup(&entries[i],&result[i].address_bits);
  if(error)return error;
 }
 links[0]=result[0];links[1]=result[1];return 0;
}
static int entry_equal(const struct remesa_use_entry_plan *a,
                       const struct remesa_use_entry_plan *b)
{
 return a->base_address==b->base_address&&a->entry_address==b->entry_address&&
  a->program_end==b->program_end&&a->bank==b->bank&&
  a->register_offset==b->register_offset&&a->register_word==b->register_word&&
  a->entry_byte_offset==b->entry_byte_offset;
}
int remesa_pds_validate_use_links(const struct remesa_pds_plan plans[2],
 const struct remesa_pds_binding programs[2],const struct remesa_pds_use_link links[2])
{
 if(!links||sizeof(*links)*2>UINTPTR_MAX-(uintptr_t)links)return EINVAL;
 const uint32_t banks[2]={links[0].entry.bank,links[1].entry.bank};
 const uint64_t offsets[2]={links[0].data_byte_offset,links[1].data_byte_offset};
 struct remesa_pds_use_link current[2];
 int error=remesa_pds_prepare_use_links(plans,programs,banks,offsets,current);if(error)return error;
 for(unsigned i=0;i<2;i++) {
  const struct remesa_pds_use_link *a=&links[i],*b=&current[i];
  if(a->stage!=b->stage||a->program_identity!=b->program_identity||
     a->data_address!=b->data_address||a->address_bits!=b->address_bits||
     !entry_equal(&a->entry,&b->entry)||!binding_equal(&a->program,&b->program)||
     a->request.owner_cookie!=b->request.owner_cookie||
     a->request.operation!=b->request.operation||a->request.generation!=b->request.generation||
     !binding_equal(&a->request.code,&b->request.code)||
     !binding_equal(&a->request.data,&b->request.data))return ESTALE;
 }
 return 0;
}
int remesa_pds_preview_record_pair(const struct remesa_pds_plan *plans,
 const struct remesa_pds_binding *programs,const struct remesa_pds_use_link *links,
 struct remesa_pds_record_preview *out)
{
 struct remesa_pds_record_preview result[2]={{0}};
 if(!plans||!programs||!links||!out||
    overlaps(out,sizeof(result),plans,2*sizeof(*plans))||
    overlaps(out,sizeof(result),programs,2*sizeof(*programs))||
    overlaps(out,sizeof(result),links,2*sizeof(*links))||
    sizeof(result)>UINTPTR_MAX-(uintptr_t)out)return EINVAL;
 int error=remesa_pds_validate_use_links(plans,programs,links);if(error)return error;
 for(unsigned i=0;i<2;i++) {
  const struct remesa_pds_request *r=&plans[i].request;
  const struct remesa_pds_binding *d=&r->data,*c=&r->code;
  /* This CPU projection is only defined for one contiguous retained block.
   * Other symbolic PDS layouts may be valid, but have no admitted projection. */
  if(d->allocation_cookie!=c->allocation_cookie)return ENOTSUP;
  uint64_t b=d->address+d->offset,code=c->address+c->offset;
  if(b>UINT32_MAX||code>UINT32_MAX||c->bytes>UINT32_MAX-code)return ERANGE;
  if(code<b||code-b<d->bytes)return EINVAL;
  uint64_t q=code-b;
  if((b&15)||(q&15))return EINVAL;
  if(!q||q>=0x400)return ERANGE;
  struct remesa_pds_record_preview *p=&result[i];
  p->operation=r->operation;p->generation=r->generation;p->owner_cookie=r->owner_cookie;
  p->allocation_cookie=d->allocation_cookie;p->mapping_generation=d->mapping_generation;
  p->block_address=b;p->prefix_bytes=q;p->code_window_address=code;p->code_bytes=c->bytes;
  p->use_entry_address=links[i].entry.entry_address;p->use_program_bytes=programs[i].bytes;
  p->data_fixup_address=links[i].data_address;p->use_address_bits=links[i].address_bits;
  p->address_prefix_known_mask=UINT32_C(0xfcffffff);
  p->address_prefix_bits=((uint32_t)(b>>4)&UINT32_C(0x00ffffff))|((uint32_t)q<<22);
  p->high_address_region=(uint32_t)b&UINT32_C(0xf0000000);
 }
 out[0]=result[0];out[1]=result[1];return 0;
}
static int host_entry_targets(const struct remesa_pds_plan *plans,
 const struct remesa_pds_binding *programs,const struct remesa_pds_use_link *links,
 const struct remesa_pds_host_data *data,unsigned char *target[2])
{
 if(!data||2*sizeof(*data)>UINTPTR_MAX-(uintptr_t)data)return EINVAL;
 /* Entry fixups require validated USE links and bounded DATA, not a T+8
  * projection. CODE/DATA can be separate symbolic allocations; no receiver
  * framing or first-PC choice is needed to patch the owned host bytes. */
 int error=remesa_pds_validate_use_links(plans,programs,links);if(error)return error;
 for(unsigned i=0;i<2;i++) {
  const struct remesa_pds_host_data *d=&data[i];
  if(!binding_equal(&d->binding,&plans[i].request.data))return ESTALE;
  if(!d->bytes||d->byte_length!=d->binding.bytes)return EINVAL;
  if(overlaps(d->bytes,d->byte_length,plans,2*sizeof(*plans))||
     overlaps(d->bytes,d->byte_length,programs,2*sizeof(*programs))||
     overlaps(d->bytes,d->byte_length,links,2*sizeof(*links))||
     overlaps(d->bytes,d->byte_length,data,2*sizeof(*data)))return EINVAL;
  if(i&&overlaps(d->bytes,d->byte_length,data[0].bytes,data[0].byte_length))return EINVAL;
  uint64_t offset=links[i].data_byte_offset;
  /* Revalidation already bounds this four-byte location; retain a local
   * subtraction check so publication cannot depend on unchecked arithmetic. */
  if(offset>d->byte_length||4>d->byte_length-offset)return ERANGE;
  target[i]=(unsigned char *)d->bytes+(size_t)offset;
 }
 return 0;
}
int remesa_pds_validate_host_entry_fixups(const struct remesa_pds_plan *plans,
 const struct remesa_pds_binding *programs,const struct remesa_pds_use_link *links,
 const struct remesa_pds_host_data *data)
{
 unsigned char *target[2];
 int error=host_entry_targets(plans,programs,links,data,target);if(error)return error;
 for(unsigned i=0;i<2;i++) {
  uint32_t word=(uint32_t)target[i][0]|((uint32_t)target[i][1]<<8)|
   ((uint32_t)target[i][2]<<16)|((uint32_t)target[i][3]<<24);
  if((word&UINT32_C(0x7ffff))!=links[i].address_bits)return EPROTO;
 }
 return 0;
}
int remesa_pds_apply_host_entry_fixups(const struct remesa_pds_plan *plans,
 const struct remesa_pds_binding *programs,const struct remesa_pds_use_link *links,
 const struct remesa_pds_host_data *data)
{
 unsigned char *target[2];uint32_t words[2];
 int error=host_entry_targets(plans,programs,links,data,target);if(error)return error;
 for(unsigned i=0;i<2;i++) {
  uint32_t previous=(uint32_t)target[i][0]|((uint32_t)target[i][1]<<8)|
   ((uint32_t)target[i][2]<<16)|((uint32_t)target[i][3]<<24);
  words[i]=(previous&~UINT32_C(0x7ffff))|links[i].address_bits;
 }
 /* No fallible operation remains. This is transactional on errors, not a
  * concurrency primitive or CPU/GPU cache-publication operation. */
 for(unsigned i=0;i<2;i++)for(unsigned byte=0;byte<4;byte++)
  target[i][byte]=(unsigned char)(words[i]>>(8*byte));
 return 0;
}
void remesa_pds_encoding_status(struct remesa_pds_encoding_status *status)
{if(status)memset(status,0,sizeof(*status));}
int remesa_pds_encode(const struct remesa_usse_program *program,const struct remesa_pds_plan *plan,void *out,size_t capacity,size_t *written)
{
 if(!written||(!out&&capacity))return EINVAL;
 int ret=remesa_pds_validate(program,plan);return ret?ret:ENOTSUP;
}

int remesa_pds_candidate_decode(uint32_t word,struct remesa_pds_candidate_request *out)
{return remesa_pds_candidate_decode_condition(word,7,out);}
int remesa_pds_candidate_decode_condition(uint32_t word,uint32_t condition,
 struct remesa_pds_candidate_request *out)
{
 if(!out)return EINVAL;
 if(condition!=3&&condition!=7)return ENOTSUP;
 if(word&UINT32_C(0x10))return EPROTO;
 if((word>>27)||((word>>24)&7)!=condition||(word&15)!=5)return ENOTSUP;
 struct remesa_pds_candidate_request r={.policy=REMESA_PDS_CANDIDATE_ANALYSIS_ONLY,
  .source={(word>>18)&63,(word>>13)&31}};
 for(unsigned n=0;n<4;n++)r.selector[n]=(word>>(11-2*n))&3;
 *out=r;return 0;
}
int remesa_pds_candidate_preview(const struct remesa_pds_candidate_request *r,
                               struct remesa_pds_candidate_preview *out)
{return remesa_pds_candidate_preview_condition(r,7,out);}
int remesa_pds_candidate_preview_condition(const struct remesa_pds_candidate_request *r,
 uint32_t condition,struct remesa_pds_candidate_preview *out)
{
 if(!r||!out||overlaps(r,sizeof(*r),out,sizeof(*out)))return EINVAL;
 if(r->policy!=REMESA_PDS_CANDIDATE_ANALYSIS_ONLY||
    (condition!=3&&condition!=7))return ENOTSUP;
 if(r->source[0]>63||r->source[1]>31)return ERANGE;
 struct remesa_pds_candidate_preview p={0};
 /* Original packing from the supplied candidate partition, not a template
  * extracted from a driver or a fragment program. No sequence is emitted. */
 p.word=(condition<<24)|(r->source[0]<<18)|(r->source[1]<<13)|5u;
 for(unsigned n=0;n<4;n++) {
  uint32_t selector=r->selector[n];if(selector>3)return ERANGE;
  p.word|=selector<<(11-2*n);
  uint32_t bank=selector>>1,half=selector&1;
  p.pair[n]=(struct remesa_pds_candidate_operand){bank,2*r->source[bank]+half};
  p.scalar[n]=(struct remesa_pds_candidate_operand){bank,r->source[bank]+half};
 }
 int error=remesa_pds_candidate_decode_condition(p.word,condition,&p.decoded);if(error)return error;
 *out=p;return 0;
}
