/* SPDX-License-Identifier: MIT */
#include "remesa/retained_evidence.h"
#include <string.h>
static int nonzero(const uint8_t *p, unsigned n)
{ unsigned i; for (i=0;i<n;i++) if(p[i]) return 1; return 0; }
static int valid(const struct remesa_evidence_expected *e)
{ return e && e->id.context && e->id.operation && e->id.serial &&
 e->id.generation && e->id.slot<8 && e->request_bytes && e->resources &&
 e->resources<=10 && nonzero(e->boot,16) && nonzero(e->source,32) &&
 nonzero(e->request_sha256,32); }
static int same(const struct remesa_evidence_id *a,const struct remesa_evidence_id *b)
{ return a->context==b->context && a->operation==b->operation &&
 a->serial==b->serial && a->slot==b->slot && a->generation==b->generation; }
enum remesa_evidence_status remesa_retained_admission_verify(
 const struct remesa_evidence_expected *e,const struct remesa_evidence_admission *a)
{
 if(!valid(e)||!a) return REMESA_EVIDENCE_MALFORMED;
 if(a->records!=1) return REMESA_EVIDENCE_MALFORMED;
 if(memcmp(e->boot,a->boot,16)||memcmp(e->source,a->source,32))
  return REMESA_EVIDENCE_BINDING;
 if(a->source_valid!=1 || a->capsule_unused!=1 || a->native_unused!=1 ||
 a->owner_valid!=1 || a->mappings_valid!=1 || a->pfn_valid!=1 ||
 a->commands_retained!=1) return REMESA_EVIDENCE_ADMISSION;
 return REMESA_EVIDENCE_OK;
}
enum remesa_evidence_status remesa_retained_completion_verify(
 const struct remesa_evidence_expected *e,const struct remesa_evidence_response *r,
 const struct remesa_evidence_observer *o,const struct remesa_evidence_ownership *g)
{
 if(!valid(e)||!r||!o||!g) return REMESA_EVIDENCE_MALFORMED;
 if(r->records!=1 || o->native_records!=1 || o->capsule_records!=1 ||
 o->source_records!=1 || g->records!=1 || !r->response_bytes ||
 !nonzero(r->response_sha256,32)) return REMESA_EVIDENCE_MALFORMED;
 if(!same(&e->id,&r->id)||!same(&e->id,&g->id)) return REMESA_EVIDENCE_IDENTITY;
 if(memcmp(e->boot,o->boot,16)||memcmp(e->source,o->source,32)||
 memcmp(e->request_sha256,r->request_sha256,32)||
 memcmp(e->request_sha256,o->request_sha256,32)||
 memcmp(r->response_sha256,o->response_sha256,32)||
 e->request_bytes!=r->request_bytes || e->request_bytes!=o->request_bytes ||
 r->response_bytes!=o->response_bytes) return REMESA_EVIDENCE_BINDING;
 if(o->request_present!=1 || o->response_present!=1 || o->closed!=1 ||
 o->syscall_exit_seen!=1 || o->invalid!=0 || o->capsule_invalid!=0 ||
 o->source_valid!=1 || o->ioctl_result!=r->ioctl_result ||
 o->syscall_result!=r->ioctl_result) return REMESA_EVIDENCE_BINDING;
 if(r->ioctl_result || r->operation_errno || r->attempted!=1 ||
 r->ta_complete!=1 || r->end_render!=1 || r->memory_free!=1 ||
 r->retired!=1 || r->visible!=1 || r->acknowledged!=1 || r->quarantined!=0 ||
 o->ta_complete!=1 || o->end_render!=1 || o->memory_free!=1 || o->retired!=1)
  return REMESA_EVIDENCE_LIFECYCLE;
 if(g->resources!=e->resources || g->pfn_valid!=1 || g->mappings_valid!=1 ||
 g->held_through_completion!=1 || g->visibility_ordered!=1 ||
 g->detached!=1 || g->released!=1 || g->quarantined!=0)
  return REMESA_EVIDENCE_OWNERSHIP;
 return REMESA_EVIDENCE_OK;
}
