/* SPDX-License-Identifier: MIT
 * Original host registry prototype. Not kernel UAPI or an MMU implementation.
 * Every call (including inspection) requires one caller-owned registry lock.
 * No returned pointer may escape that lock: API returns snapshots only.
 */
#ifndef REMESA_REGISTRY_H
#define REMESA_REGISTRY_H
#include <stddef.h>
#include "remesa/resource.h"
struct remesa_token { size_t slot; uint64_t generation; };
struct remesa_slot { struct remesa_resource resource; uint64_t generation; unsigned live, holds; };
struct remesa_registry { struct remesa_slot *slots; size_t count; uint64_t next_generation; };
int remesa_registry_init(struct remesa_registry *, struct remesa_slot *, size_t);
int remesa_registry_allocate(struct remesa_registry *, uint64_t bytes, struct remesa_token *);
int remesa_registry_snapshot(struct remesa_registry *, struct remesa_token, struct remesa_resource *);
int remesa_registry_map(struct remesa_registry *, struct remesa_token, uint64_t va, uint64_t alignment);
int remesa_registry_begin(struct remesa_registry *, struct remesa_token, uint64_t operation);
int remesa_registry_retire(struct remesa_registry *, struct remesa_token, uint64_t operation);
int remesa_registry_publish(struct remesa_registry *, struct remesa_token, uint64_t operation);
/* Holds retain metadata/backing identity through provider job lifetime. */
int remesa_registry_hold(struct remesa_registry *, struct remesa_token);
int remesa_registry_drop(struct remesa_registry *, struct remesa_token);
int remesa_registry_remove(struct remesa_registry *, struct remesa_token);
#endif
