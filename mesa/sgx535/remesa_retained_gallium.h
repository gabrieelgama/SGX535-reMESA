/* SPDX-License-Identifier: MIT */
#ifndef REMESA_RETAINED_GALLIUM_H
#define REMESA_RETAINED_GALLIUM_H
#include "remesa_linux_memory.h"
#include "remesa/retained_uapi.h"
#include "remesa_retained_readback.h"
/* Borrowed mapping: never allocates/copies COLOR or owns its kernel backing. */
int remesa_retained_gallium_read(struct remesa_linux_memory *,const struct remesa_retained_request *,uint32_t,uint64_t,
 const struct remesa_evidence_expected *,const struct remesa_evidence_response *,
 const struct remesa_evidence_observer *,const struct remesa_evidence_ownership *,
 void *,size_t);
#endif
