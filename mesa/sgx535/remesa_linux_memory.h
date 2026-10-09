/* SPDX-License-Identifier: MIT */
#ifndef REMESA_LINUX_MEMORY_H
#define REMESA_LINUX_MEMORY_H
#include "remesa_persistent.h"
#include "remesa/memory_uapi.h"
/* Borrow fd/context already bound by the future kernel per-file integration.
 * Never discovers/installs a driver or opens a live node. Current stock/native
 * modules do not expose the memory ioctl and must fail ENOTTY. No draw support.
 * Externally serialized, close only after persistent resources are destroyed. */
struct remesa_linux_memory;
/* Begin a kernel-issued per-file resource session; no GPU submission. */
struct remesa_memory_recorder {
 int (*record)(void *,const struct remesa_memory_request *,const struct remesa_memory_request *,int,int,bool);
 void *priv;
};
int remesa_linux_memory_begin_recorded(int,struct remesa_linux_memory **,const struct remesa_memory_recorder *);
int remesa_linux_memory_begin(int,struct remesa_linux_memory **);
int remesa_linux_memory_attach(int,uint64_t,struct remesa_linux_memory **);
int remesa_linux_memory_info(struct remesa_linux_memory *,uint32_t,struct remesa_memory_info *);
/* Exact retained ID/size/VA/pin/PTE ownership admission for a future executor.
 * Performs INFO only, never submission. This proves CPU PTE ownership, not
 * GPU TLB visibility/completion. An uncertain response poisons the object. */
int remesa_linux_memory_validate_backing(struct remesa_linux_memory *,uint32_t,
                                        uint64_t,size_t);
uint64_t remesa_linux_memory_context(const struct remesa_linux_memory *);
/* Drop ONLY host staging/metadata after all consumers stop. No kernel cleanup,
 * no ioctl, no claim of retirement; caller still owns and must close the fd. */
void remesa_linux_memory_forget(struct remesa_linux_memory *);
int remesa_linux_memory_detach(struct remesa_linux_memory *);
const struct remesa_persistent_ops *remesa_linux_memory_ops(void);
#endif
