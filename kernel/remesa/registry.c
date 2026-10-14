/* SPDX-License-Identifier: MIT */
#include <string.h>
#include "remesa/registry.h"
static struct remesa_slot *resolve(struct remesa_registry *r, struct remesa_token t)
{
    if (!r || !r->slots || t.slot >= r->count || !t.generation) return NULL;
    struct remesa_slot *s=&r->slots[t.slot];
    return s->live && s->generation==t.generation ? s : NULL;
}
int remesa_registry_init(struct remesa_registry *r, struct remesa_slot *slots, size_t count)
{
    if (!r || !slots || !count || count>SIZE_MAX/sizeof(*slots)) return REMESA_INVALID;
    memset(slots,0,count*sizeof(*slots));
    *r=(struct remesa_registry){slots,count,1};
    return REMESA_OK;
}
int remesa_registry_allocate(struct remesa_registry *r, uint64_t bytes, struct remesa_token *out)
{
    if (!r || !r->slots || !bytes || !out) return REMESA_INVALID;
    /* Never wrap identity. Reinitializing a registry requires destruction of
     * its entire namespace and all external tokens; it is not a reset API. */
    if (!r->next_generation || r->next_generation==UINT64_MAX) return REMESA_BUSY;
    for (size_t i=0;i<r->count;i++) if (!r->slots[i].live) {
        struct remesa_slot *s=&r->slots[i];
        s->generation=r->next_generation++;
        remesa_resource_init(&s->resource,s->generation,bytes);
        s->live=1;
        *out=(struct remesa_token){i,s->generation};
        return REMESA_OK;
    }
    return REMESA_BUSY;
}
int remesa_registry_snapshot(struct remesa_registry *r, struct remesa_token t, struct remesa_resource *out)
{
    struct remesa_slot *s=resolve(r,t);
    if (!s || !out) return REMESA_INVALID;
    *out=s->resource; return REMESA_OK;
}
int remesa_registry_map(struct remesa_registry *r, struct remesa_token t, uint64_t va, uint64_t align)
{
    struct remesa_slot *s=resolve(r,t);
    if (!s) return REMESA_INVALID;
    /* Validate on a copy so a rejected overlap cannot partially map. */
    struct remesa_resource candidate=s->resource;
    int status=remesa_resource_map(&candidate,va,align);
    if (status!=REMESA_OK) return status;
    for (size_t i=0;i<r->count;i++) {
        struct remesa_slot *other=&r->slots[i];
        if (other==s || !other->live || !other->resource.gpu_va) continue;
        uint64_t b=other->resource.gpu_va, n=other->resource.bytes;
        if (va < b+n && b < va+candidate.bytes) return REMESA_BUSY;
    }
    s->resource=candidate; return REMESA_OK;
}
#define TRANSITION(name) \
int remesa_registry_##name(struct remesa_registry *r, struct remesa_token t, uint64_t op) \
{ struct remesa_slot *s=resolve(r,t); return s ? remesa_resource_##name(&s->resource,op) : REMESA_INVALID; }
TRANSITION(begin)
TRANSITION(retire)
TRANSITION(publish)
int remesa_registry_hold(struct remesa_registry *r, struct remesa_token t)
{
    struct remesa_slot *s=resolve(r,t);
    if (!s) return REMESA_INVALID;
    if (s->holds==UINT32_MAX) return REMESA_BUSY;
    ++s->holds; return REMESA_OK;
}
int remesa_registry_drop(struct remesa_registry *r, struct remesa_token t)
{
    struct remesa_slot *s=resolve(r,t);
    if (!s || !s->holds) return REMESA_INVALID;
    --s->holds; return REMESA_OK;
}
int remesa_registry_remove(struct remesa_registry *r, struct remesa_token t)
{
    struct remesa_slot *s=resolve(r,t);
    if (!s) return REMESA_INVALID;
    /* Quarantine may still contain an ambiguous GPU writer: retain backing. */
    if (s->holds || s->resource.state==REMESA_IN_FLIGHT || s->resource.state==REMESA_QUARANTINED) return REMESA_BUSY;
    remesa_resource_quarantine(&s->resource);
    s->live=0; return REMESA_OK;
}
