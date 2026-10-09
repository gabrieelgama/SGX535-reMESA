/* SPDX-License-Identifier: MIT
 * Portable specification prototype; not kernel MMU or DMA implementation. */
#include "remesa/resource.h"
int remesa_resource_init(struct remesa_resource *r, uint64_t h, uint64_t bytes)
{
    if (!r || !h || !bytes) return REMESA_INVALID;
    *r = (struct remesa_resource){ .handle=h, .bytes=bytes, .state=REMESA_NEW };
    return REMESA_OK;
}
int remesa_resource_map(struct remesa_resource *r, uint64_t va, uint64_t align)
{
    if (!r || !va || !align || (align & (align-1)) || (va & (align-1)) ||
        va > UINT64_MAX-r->bytes) return REMESA_INVALID;
    if (r->state != REMESA_NEW) return REMESA_BUSY;
    r->gpu_va=va; r->state=REMESA_MAPPED; return REMESA_OK;
}
int remesa_resource_begin(struct remesa_resource *r, uint64_t op)
{
    if (!r || !op) return REMESA_INVALID;
    if (r->state != REMESA_MAPPED) return REMESA_BUSY;
    r->operation=op; r->published=0; r->state=REMESA_IN_FLIGHT;
    return REMESA_OK;
}
int remesa_resource_retire(struct remesa_resource *r, uint64_t op)
{
    if (!r || !op || op != r->operation) return REMESA_INVALID;
    if (r->state != REMESA_IN_FLIGHT) return REMESA_BUSY;
    r->state=REMESA_RETIRED; return REMESA_OK;
}
int remesa_resource_publish(struct remesa_resource *r, uint64_t op)
{
    if (!r || !op || op != r->operation) return REMESA_INVALID;
    if (r->state != REMESA_RETIRED) return REMESA_BUSY;
    r->published=1; return REMESA_OK;
}
int remesa_resource_readable(const struct remesa_resource *r, uint64_t off, uint64_t n)
{
    if (!r || !n || off > r->bytes || n > r->bytes-off) return REMESA_INVALID;
    if (r->state != REMESA_RETIRED || !r->published) return REMESA_UNPUBLISHED;
    return REMESA_OK;
}
int remesa_resource_unmap(struct remesa_resource *r)
{
    if (!r) return REMESA_INVALID;
    if (r->state != REMESA_MAPPED && r->state != REMESA_RETIRED) return REMESA_BUSY;
    r->gpu_va=0; r->published=0;
    /* Terminal: reuse requires a new independently owned allocation, not rearm. */
    r->state=REMESA_QUARANTINED; return REMESA_OK;
}
void remesa_resource_quarantine(struct remesa_resource *r)
{
    if (r) { r->published=0; r->state=REMESA_QUARANTINED; }
}
