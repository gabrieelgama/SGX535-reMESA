/* SPDX-License-Identifier: MIT; no independently established PDS bitfields. */
#include "remesa_pds.h"
#include <errno.h>
#include <string.h>
static int overlaps(const void *a,size_t na,const void *b,size_t nb)
{
 if(!na||!nb)return 0;
 uintptr_t x=(uintptr_t)a,y=(uintptr_t)b;
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
 if(r->code.allocation_cookie==r->data.allocation_cookie&&
    (r->code.mapping_generation!=r->data.mapping_generation||
     r->code.address!=r->data.address||r->code.allocation_bytes!=r->data.allocation_bytes))return ESTALE;
 uint64_t c=r->code.address+r->code.offset,d=r->data.address+r->data.offset;
 if(c<d+r->data.bytes&&d<c+r->code.bytes)return EINVAL;
 return 0;
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
void remesa_pds_encoding_status(struct remesa_pds_encoding_status *status)
{if(status)memset(status,0,sizeof(*status));}
int remesa_pds_encode(const struct remesa_usse_program *program,const struct remesa_pds_plan *plan,void *out,size_t capacity,size_t *written)
{
 if(!written||(!out&&capacity))return EINVAL;
 int ret=remesa_pds_validate(program,plan);return ret?ret:ENOTSUP;
}
