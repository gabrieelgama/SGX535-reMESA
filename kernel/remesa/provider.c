/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "remesa/provider.h"
static struct remesa_job *resolve(struct remesa_provider *p, struct remesa_fence f)
{
    if (!p || f.context != p->context || f.slot >= REMESA_PROVIDER_MAX_JOBS || !f.generation) return NULL;
    struct remesa_job *j = &p->jobs[f.slot];
    return j->live && j->generation == f.generation ? j : NULL;
}
static struct remesa_resource *resource(struct remesa_provider *p, struct remesa_token t)
{
    struct remesa_registry *r = p->registry;
    if (!r || t.slot >= r->count || !t.generation) return NULL;
    struct remesa_slot *s = &r->slots[t.slot];
    return s->live && s->generation == t.generation ? &s->resource : NULL;
}
static void quarantine(struct remesa_provider *p, struct remesa_job *j)
{
    for (size_t i=0; i<j->count; ++i) {
        struct remesa_resource *r = resource(p,j->uses[i].token);
        if (r) remesa_resource_quarantine(r);
    }
    j->state = REMESA_FENCE_FAILED;
}
int remesa_provider_init(struct remesa_provider *p, struct remesa_registry *r,
                         uint64_t context, const struct remesa_backend *b)
{
    if (!p || !r || !r->slots || !r->count || !context || !b) return REMESA_INVALID;
    memset(p,0,sizeof(*p));
    p->registry=r; p->context=context; p->backend=*b; p->next_operation=1;
    return REMESA_OK;
}
int remesa_provider_submit(struct remesa_provider *p, const struct remesa_use *uses,
                           size_t count, struct remesa_fence *out)
{
    if (!p || !uses || !out || !count || count>REMESA_PROVIDER_MAX_USES) return REMESA_INVALID;
    if (!p->backend.dispatch) return REMESA_UNSUPPORTED;
    if (!p->next_operation || p->next_operation==UINT64_MAX) return REMESA_BUSY;
    size_t slot;
    for (slot=0; slot<REMESA_PROVIDER_MAX_JOBS && p->jobs[slot].live; ++slot) {}
    if (slot==REMESA_PROVIDER_MAX_JOBS) return REMESA_BUSY;
    struct remesa_resource previous[REMESA_PROVIDER_MAX_USES];
    for (size_t i=0; i<count; ++i) {
        struct remesa_resource *r=resource(p,uses[i].token);
        if (!r || !uses[i].bytes || uses[i].offset>r->bytes ||
            uses[i].bytes>r->bytes-uses[i].offset || !uses[i].access ||
            (uses[i].access & ~(REMESA_ACCESS_READ|REMESA_ACCESS_WRITE))) return REMESA_INVALID;
        if (r->state!=REMESA_MAPPED) return REMESA_BUSY;
        /* Require one consolidated range per object; aliases cannot receive
         * inconsistent access/lifetime transitions. */
        for (size_t n=0; n<i; ++n)
            if (uses[n].token.slot==uses[i].token.slot) return REMESA_INVALID;
        previous[i]=*r;
    }
    for (size_t i=0; i<count; ++i) {
        int s=remesa_registry_hold(p->registry,uses[i].token);
        if (s!=REMESA_OK) {
            for (size_t n=0;n<i;++n) remesa_registry_drop(p->registry,uses[n].token);
            return s;
        }
    }
    struct remesa_job *j=&p->jobs[slot];
    *j=(struct remesa_job){.context=p->context,.operation=p->next_operation,
        .generation=p->next_operation,.count=count,.state=REMESA_FENCE_PENDING,.live=1};
    ++p->next_operation;
    memcpy(j->uses,uses,count*sizeof(*uses));
    for (size_t i=0; i<count; ++i) {
        int s=remesa_resource_begin(resource(p,uses[i].token),j->operation);
        if (s!=REMESA_OK) { /* Cannot happen while caller lock is held. */
            for (size_t n=0;n<i;++n) *resource(p,uses[n].token)=previous[n];
            for (size_t n=0;n<count;++n) remesa_registry_drop(p->registry,uses[n].token);
            j->live=0; return s;
        }
    }
    enum remesa_dispatch result=p->backend.dispatch(p->backend.data,j);
    if (result==REMESA_NOT_ISSUED) {
        for (size_t i=0; i<count; ++i) *resource(p,uses[i].token)=previous[i];
        for (size_t i=0;i<count;++i) remesa_registry_drop(p->registry,uses[i].token);
        j->live=0; return REMESA_UNSUPPORTED;
    }
    *out=(struct remesa_fence){slot,j->generation,j->context};
    if (result!=REMESA_ISSUED) { quarantine(p,j); return REMESA_AMBIGUOUS; }
    return REMESA_OK;
}
int remesa_provider_query(struct remesa_provider *p, struct remesa_fence f,
                          struct remesa_job *out)
{
    struct remesa_job *j=resolve(p,f);
    if (!j || !out) return REMESA_INVALID;
    *out=*j; return REMESA_OK;
}
static int receipt(struct remesa_provider *p, struct remesa_fence f,
                    uint64_t context, uint64_t op, struct remesa_job **out)
{
    struct remesa_job *j=resolve(p,f);
    if (!j || j->context!=context || j->operation!=op) return REMESA_INVALID;
    *out=j; return REMESA_OK;
}
int remesa_provider_complete(struct remesa_provider *p, struct remesa_fence f,
                             uint64_t context, uint64_t op, int success)
{
    struct remesa_job *j; int s=receipt(p,f,context,op,&j);
    if (s!=REMESA_OK) return s;
    if (success!=0 && success!=1) return REMESA_INVALID;
    if (j->state!=REMESA_FENCE_PENDING) return REMESA_BUSY;
    if (!success) { quarantine(p,j); return REMESA_OK; }
    for (size_t i=0;i<j->count;++i) {
        struct remesa_resource *r=resource(p,j->uses[i].token);
        if (!r || r->state!=REMESA_IN_FLIGHT || r->operation!=op) {
            quarantine(p,j); return REMESA_INVALID;
        }
    }
    for (size_t i=0;i<j->count;++i) remesa_resource_retire(resource(p,j->uses[i].token),op);
    j->state=REMESA_FENCE_RETIRED; return REMESA_OK;
}
int remesa_provider_publish(struct remesa_provider *p, struct remesa_fence f,
                            uint64_t context, uint64_t op)
{
    struct remesa_job *j; int s=receipt(p,f,context,op,&j);
    if (s!=REMESA_OK) return s;
    if (j->state!=REMESA_FENCE_RETIRED) return REMESA_BUSY;
    for (size_t i=0;i<j->count;++i) {
        struct remesa_resource *r=resource(p,j->uses[i].token);
        if (!r || r->state!=REMESA_RETIRED || r->operation!=op) {
            quarantine(p,j); return REMESA_INVALID;
        }
    }
    for (size_t i=0;i<j->count;++i)
        if (j->uses[i].access & REMESA_ACCESS_WRITE)
            remesa_resource_publish(resource(p,j->uses[i].token),op);
    j->state=REMESA_FENCE_PUBLISHED; return REMESA_OK;
}
int remesa_provider_fault(struct remesa_provider *p, struct remesa_fence f,
                          uint64_t context, uint64_t op)
{
    struct remesa_job *j; int s=receipt(p,f,context,op,&j);
    if(s!=REMESA_OK) return s;
    quarantine(p,j); return REMESA_OK;
}
int remesa_provider_release(struct remesa_provider *p, struct remesa_fence f)
{
    struct remesa_job *j=resolve(p,f);
    if (!j) return REMESA_INVALID;
    if (j->state!=REMESA_FENCE_RETIRED && j->state!=REMESA_FENCE_PUBLISHED) return REMESA_BUSY;
    if (!p->backend.release) return REMESA_UNSUPPORTED;
    for (size_t i=0;i<j->count;++i) {
        struct remesa_resource *r=resource(p,j->uses[i].token);
        if (!r || r->state!=REMESA_RETIRED || r->operation!=j->operation) {
            quarantine(p,j); return REMESA_INVALID;
        }
    }
    int release_status=p->backend.release(p->backend.data,j);
    if (release_status==REMESA_RETRY) return REMESA_RETRY;
    if (release_status!=REMESA_OK) {
        quarantine(p,j); return REMESA_BUSY;
    }
    for (size_t i=0;i<j->count;++i) {
        struct remesa_resource *r=resource(p,j->uses[i].token);
        r->state=REMESA_MAPPED; r->operation=0; r->published=0;
        remesa_registry_drop(p->registry,j->uses[i].token);
    }
    j->live=0; return REMESA_OK;
}
