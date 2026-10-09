/* SPDX-License-Identifier: MIT */
#ifndef REMESA_RETAINED_READBACK_H
#define REMESA_RETAINED_READBACK_H
#include <stdint.h>
#include <stddef.h>
#include "remesa_persistent.h"
#include "remesa/retained_evidence.h"
struct pipe_context;
struct pipe_resource;
struct pipe_fence_handle;
#define REMESA_RETAINED_COLOR_BYTES 4096u
/* Internal readback seam. Receipt MUST come from the authenticated transport;
 * comparing identities is not authentication of an arbitrary caller record.
 * Gallium's writer fence and map gates provide independent access ordering.
 * External serialization must prevent a new writer during this snapshot.
 * No GPU cache/TLB operation, rasterization or submission occurs here. */
/* CPU preparation only. Gallium/provider unmap and submit remain responsible
 * for qualified sync_for_device; this helper does not flush caches or execute. */
int remesa_retained_upload(struct pipe_context *,struct pipe_resource *,size_t,
 const void *,size_t);
int remesa_retained_readback(struct pipe_context *,struct pipe_resource *,
 struct pipe_fence_handle *,uint64_t,uint64_t,const struct remesa_gpu_receipt *,
 uint64_t,void *,size_t);
/* Strict variant ties access to complete normalized kernel/observer/ownership
 * evidence. Raw originals and trusted capture remain required for a physical
 * claim; synthetic normalized records alone cannot establish GPU execution. */
int remesa_retained_readback_verified(struct pipe_context *,struct pipe_resource *,
 struct pipe_fence_handle *,const struct remesa_evidence_expected *,
 const struct remesa_evidence_response *,const struct remesa_evidence_observer *,
 const struct remesa_evidence_ownership *,uint64_t,void *,size_t);
/* Compare original preserved bytes; never generate a reference image. */
int remesa_retained_compare(const void *,size_t,const void *,size_t,unsigned *);
#endif
