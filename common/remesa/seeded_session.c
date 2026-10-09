/* SPDX-License-Identifier: MIT */
#include "remesa/seeded_session.h"
#include <errno.h>
#include <string.h>

/* Origin: independently authored host policy from the project's retained GEM
 * service lifecycle and exact FIRE3 observation interface. Numeric values here
 * are host record enums/counts, not SGX instructions or registers. */
static int same(const struct remesa_framebuffer_observation_identity *a,
                const struct remesa_framebuffer_observation_identity *b)
{
    return a->owner == b->owner && a->resource == b->resource &&
        a->resource_generation == b->resource_generation &&
        a->mapping_generation == b->mapping_generation &&
        a->operation == b->operation &&
        a->operation_generation == b->operation_generation &&
        a->content_epoch == b->content_epoch;
}
static int overlaps(const void *a, size_t an, const void *b, size_t bn)
{
    uintptr_t aa=(uintptr_t)a, bb=(uintptr_t)b;
    return aa<=bb ? bb-aa<an : aa-bb<bn;
}
int remesa_seeded_session_validate(
    const struct remesa_framebuffer_observation_identity *expected,
    const struct remesa_seeded_session_record *records, size_t count,
    const void *reference, size_t reference_bytes,
    remesa_seeded_record_verifier verifier, void *opaque,
    struct remesa_seeded_session_report *report)
{
    struct remesa_seeded_session_report result={0};
    uint64_t sequence=0, gpu_va, pfn;
    int error;
    if(!expected || !records || !reference || !verifier || !report ||
       count!=REMESA_SEEDED_SESSION_RECORDS || reference_bytes!=4096 ||
       !expected->owner || !expected->resource || !expected->resource_generation ||
       !expected->mapping_generation || !expected->operation ||
       !expected->operation_generation || !expected->content_epoch) return EINVAL;
    if(overlaps(report,sizeof(*report),records,count*sizeof(*records)) ||
       overlaps(report,sizeof(*report),expected,sizeof(*expected)) ||
       overlaps(report,sizeof(*report),reference,reference_bytes)) return EINVAL;
    gpu_va=records[0].gpu_va; pfn=records[0].pfn;
    if(!gpu_va || (gpu_va&4095u) || gpu_va>UINT32_MAX-4095u || !pfn) return EINVAL;
    for(size_t i=0;i<count;i++) {
        const struct remesa_seeded_session_record *r=records+i;
        unsigned event=(unsigned)r->event;
        int capture=event==REMESA_SEEDED_BEFORE ||
            event==REMESA_SEEDED_PRIMARY_CAPTURE ||
            event==REMESA_SEEDED_CORROBORATING_CAPTURE;
        int released=event>=REMESA_SEEDED_RELEASED;
        unsigned jobs=event>=REMESA_SEEDED_SUBMITTED &&
            event<=REMESA_SEEDED_ACKNOWLEDGED ? 1u:0u;
        if(event!=i+1 || !r->sequence || r->sequence<=sequence ||
           !r->provider || !r->record_id) return EINVAL;
        if(!same(expected,&r->identity)) return ESTALE;
        for(size_t j=0;j<i;j++)
            if(r->provider==records[j].provider &&
               r->record_id==records[j].record_id) return EALREADY;
        if(r->quarantined || r->live_resources!=(released?0u:10u) ||
           r->mapped_pages!=(released?0u:1u) ||
           r->verified_pages!=(released?0u:1u) ||
           r->pins!=(released?0u:1u) ||
           r->cpu_leases!=(capture?1u:0u) || r->held_jobs!=jobs ||
           r->submissions!=(event>=REMESA_SEEDED_SUBMITTED?1u:0u) ||
           r->gpu_va!=(released?0:gpu_va) || r->pfn!=(released?0:pfn))
            return EBUSY;
        if(capture) {
            if(!r->capture_id || !r->payload || r->payload_bytes!=4096 ||
               overlaps(report,sizeof(*report),r->payload,r->payload_bytes))
                return EINVAL;
        } else if(r->capture_id || r->payload || r->payload_bytes) return EINVAL;
        error=verifier(opaque,r);
        if(error) return error;
        result.verified_records++;
        sequence=r->sequence;
    }
    const struct remesa_seeded_session_record *before=records+1;
    const struct remesa_seeded_session_record *primary=records+10;
    const struct remesa_seeded_session_record *independent=records+11;
    if(primary->provider==independent->provider ||
       primary->capture_id==independent->capture_id ||
       before->capture_id==primary->capture_id ||
       before->capture_id==independent->capture_id) return EINVAL;
    error=remesa_seeded_triangle_observe(expected,&primary->identity,
        before->payload,before->payload_bytes,reference,reference_bytes,
        primary->payload,primary->payload_bytes,
        independent->payload,independent->payload_bytes,&result.observation);
    if(error && error!=EILSEQ) return error;
    /* EILSEQ may reject a corrupt oracle before producing comparison metrics.
     * Do not publish an apparently valid zero-initialized report in that case. */
    if(error && !result.observation.corroboration_byte_mismatches) return error;
    result.primary_provider=primary->provider;
    result.corroborating_provider=independent->provider;
    *report=result;
    if(error) return error;
    if(result.observation.pixels.result==REMESA_FB_UNCHANGED) return ENODATA;
    if(result.observation.pixels.result==REMESA_FB_MISMATCH) return EILSEQ;
    return 0;
}
