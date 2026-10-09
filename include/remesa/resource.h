/* SPDX-License-Identifier: MIT
 * Original host-side ownership model. Not a stable kernel UAPI. */
#ifndef REMESA_RESOURCE_H
#define REMESA_RESOURCE_H
#include <stdint.h>
enum remesa_status { REMESA_OK, REMESA_INVALID, REMESA_BUSY,
                     REMESA_UNSUPPORTED, REMESA_UNPUBLISHED, REMESA_AMBIGUOUS,
                     REMESA_RETRY };
enum remesa_resource_state { REMESA_NEW, REMESA_MAPPED, REMESA_IN_FLIGHT,
                             REMESA_RETIRED, REMESA_QUARANTINED };
struct remesa_resource {
    uint64_t handle, bytes, gpu_va, operation;
    enum remesa_resource_state state;
    unsigned published;
};
int remesa_resource_init(struct remesa_resource *, uint64_t handle, uint64_t bytes);
int remesa_resource_map(struct remesa_resource *, uint64_t va, uint64_t alignment);
int remesa_resource_begin(struct remesa_resource *, uint64_t operation);
/* Caller must supply an independently qualified, matching retirement receipt.
 * This model validates identity, not the authenticity of that receipt. */
int remesa_resource_retire(struct remesa_resource *, uint64_t operation);
/* Executed only after a separately established device publication contract. */
int remesa_resource_publish(struct remesa_resource *, uint64_t operation);
int remesa_resource_readable(const struct remesa_resource *, uint64_t offset, uint64_t bytes);
int remesa_resource_unmap(struct remesa_resource *);
void remesa_resource_quarantine(struct remesa_resource *);
#endif
