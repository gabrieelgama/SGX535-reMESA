/* SPDX-License-Identifier: MIT */
/* Entirely synthetic fixtures, never hardware observations. */
#include "remesa/retained_evidence.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static struct remesa_evidence_expected e;
static struct remesa_evidence_admission a;
static struct remesa_evidence_response r;
static struct remesa_evidence_observer o;
static struct remesa_evidence_ownership g;
static unsigned tests;
static void reset(void)
{
 memset(&e,0,sizeof e); memset(&a,0,sizeof a); memset(&r,0,sizeof r);
 memset(&o,0,sizeof o); memset(&g,0,sizeof g);
 e.id=(struct remesa_evidence_id){1,2,3,4,5};
 memset(e.boot,1,16); memset(e.source,2,32); memset(e.request_sha256,3,32);
 e.request_bytes=264; e.resources=10;
 memcpy(a.boot,e.boot,16); memcpy(a.source,e.source,32);
 a.records=a.source_valid=a.capsule_unused=a.native_unused=a.owner_valid=1;
 a.mappings_valid=a.pfn_valid=a.commands_retained=1;
 r.id=g.id=e.id; r.request_bytes=o.request_bytes=e.request_bytes;
 r.response_bytes=o.response_bytes=4096;
 memcpy(r.request_sha256,e.request_sha256,32); memcpy(o.request_sha256,e.request_sha256,32);
 memset(r.response_sha256,4,32); memcpy(o.response_sha256,r.response_sha256,32);
 memcpy(o.boot,e.boot,16); memcpy(o.source,e.source,32);
 r.records=r.attempted=r.ta_complete=r.end_render=r.memory_free=r.retired=1;
 r.visible=r.acknowledged=1;
 o.native_records=o.capsule_records=o.source_records=1;
 o.request_present=o.response_present=o.closed=o.syscall_exit_seen=o.source_valid=1;
 o.ta_complete=o.end_render=o.memory_free=o.retired=1;
 g.records=g.pfn_valid=g.mappings_valid=g.held_through_completion=1;
 g.visibility_ordered=g.detached=g.released=1; g.resources=e.resources;
}
#define BAD(change,expect) do { reset(); change; assert(remesa_retained_completion_verify(&e,&r,&o,&g)==expect);tests++;}while(0)
#define ADMIT_BAD(change,expect) do { reset(); change; assert(remesa_retained_admission_verify(&e,&a)==expect);tests++;}while(0)
int main(void)
{
 reset(); assert(remesa_retained_admission_verify(&e,&a)==REMESA_EVIDENCE_OK);tests++;
 assert(remesa_retained_completion_verify(&e,&r,&o,&g)==REMESA_EVIDENCE_OK);tests++;
 BAD(r.id.context++,REMESA_EVIDENCE_IDENTITY);
 BAD(r.id.operation++,REMESA_EVIDENCE_IDENTITY);
 BAD(r.id.serial++,REMESA_EVIDENCE_IDENTITY);
 BAD(r.id.slot++,REMESA_EVIDENCE_IDENTITY);
 BAD(r.id.generation++,REMESA_EVIDENCE_IDENTITY);
 BAD(g.id.generation++,REMESA_EVIDENCE_IDENTITY);
 BAD(e.id.generation=0,REMESA_EVIDENCE_MALFORMED);
 BAD(e.id.slot=8,REMESA_EVIDENCE_MALFORMED);
 BAD(r.records=2,REMESA_EVIDENCE_MALFORMED);
 BAD(o.native_records=0,REMESA_EVIDENCE_MALFORMED);
 BAD(o.capsule_records=2,REMESA_EVIDENCE_MALFORMED);
 BAD(o.source_records=0,REMESA_EVIDENCE_MALFORMED);
 BAD(g.records=2,REMESA_EVIDENCE_MALFORMED);
 BAD(o.boot[0]++,REMESA_EVIDENCE_BINDING);
 BAD(o.source[0]++,REMESA_EVIDENCE_BINDING);
 BAD(r.request_sha256[0]++,REMESA_EVIDENCE_BINDING);
 BAD(o.request_sha256[0]++,REMESA_EVIDENCE_BINDING);
 BAD(o.response_sha256[31]++,REMESA_EVIDENCE_BINDING);
 BAD(o.request_bytes--,REMESA_EVIDENCE_BINDING);
 BAD(o.response_bytes--,REMESA_EVIDENCE_BINDING);
 BAD(o.closed=0,REMESA_EVIDENCE_BINDING);
 BAD(o.syscall_exit_seen=0,REMESA_EVIDENCE_BINDING);
 BAD(o.syscall_result=-1,REMESA_EVIDENCE_BINDING);
 BAD(o.invalid=1,REMESA_EVIDENCE_BINDING);
 BAD(o.capsule_invalid=1,REMESA_EVIDENCE_BINDING);
 BAD(o.source_valid=2,REMESA_EVIDENCE_BINDING);
 BAD(r.operation_errno=-5,REMESA_EVIDENCE_LIFECYCLE);
 BAD(r.attempted=0,REMESA_EVIDENCE_LIFECYCLE);
 BAD(r.ta_complete=0,REMESA_EVIDENCE_LIFECYCLE);
 BAD(o.end_render=0,REMESA_EVIDENCE_LIFECYCLE);
 BAD(r.memory_free=0,REMESA_EVIDENCE_LIFECYCLE);
 BAD(o.retired=0,REMESA_EVIDENCE_LIFECYCLE);
 BAD(r.retired=0,REMESA_EVIDENCE_LIFECYCLE);
 BAD(r.visible=0,REMESA_EVIDENCE_LIFECYCLE);
 BAD(r.acknowledged=0,REMESA_EVIDENCE_LIFECYCLE);
 BAD(r.quarantined=1,REMESA_EVIDENCE_LIFECYCLE);
 BAD(g.pfn_valid=0,REMESA_EVIDENCE_OWNERSHIP);
 BAD(g.mappings_valid=0,REMESA_EVIDENCE_OWNERSHIP);
 BAD(g.held_through_completion=0,REMESA_EVIDENCE_OWNERSHIP);
 BAD(g.visibility_ordered=0,REMESA_EVIDENCE_OWNERSHIP);
 BAD(g.detached=0,REMESA_EVIDENCE_OWNERSHIP);
 BAD(g.released=0,REMESA_EVIDENCE_OWNERSHIP);
 BAD(g.resources--,REMESA_EVIDENCE_OWNERSHIP);
 BAD(g.quarantined=1,REMESA_EVIDENCE_OWNERSHIP);
 ADMIT_BAD(a.capsule_unused=0,REMESA_EVIDENCE_ADMISSION);
 ADMIT_BAD(a.native_unused=0,REMESA_EVIDENCE_ADMISSION);
 ADMIT_BAD(a.pfn_valid=0,REMESA_EVIDENCE_ADMISSION);
 ADMIT_BAD(a.commands_retained=0,REMESA_EVIDENCE_ADMISSION);
 ADMIT_BAD(a.records=2,REMESA_EVIDENCE_MALFORMED);
 ADMIT_BAD(a.source[0]++,REMESA_EVIDENCE_BINDING);
 reset(); assert(remesa_retained_completion_verify(0,&r,&o,&g)==REMESA_EVIDENCE_MALFORMED);tests++;
 printf("PASS %u synthetic evidence cases; no hardware observations\n",tests);
 return 0;
}
