/* SPDX-License-Identifier: MIT */
#ifndef REMESA_SCREEN_H
#define REMESA_SCREEN_H
#include <stdint.h>
#include <stdbool.h>
/* Opaque factory ABI also used by the standalone ownership-model tests.
 * The Gallium adapter qualification compiles against pinned Mesa headers. */
struct pipe_screen;
struct remesa_device;
struct pipe_context; struct pipe_resource; struct pipe_fence_handle;
/* Poll/wait the existing resource writer; preserves exact backend errors.
 * Does not issue a draw or manufacture completion. */
int remesa_resource_wait_writer(struct pipe_context *,struct pipe_resource *,uint64_t timeout);
/* Internal serialized import. Backend owns both BO and completion evidence. */
int remesa_resource_attach_writer(struct pipe_context *,struct pipe_resource *,void *,struct pipe_fence_handle **);
struct pipe_screen *remesa_screen_create(struct remesa_device *device);
/* Consumes device only on success. Release runs after the final context,
 * resource and fence disappear. Without release, backend ownership is borrowed
 * and its caller must keep it alive for all children. Release must return a
 * nonzero error without destroying pending/quarantined backing; EBUSY/EAGAIN
 * mean side-effect-free retry. On error the closing screen retains ownership.
 * Not an OpenGL factory. */
struct pipe_screen *remesa_screen_create_owned(struct remesa_device *,int (*release)(struct remesa_device *));
/* For a closing owner retained because release failed. EBUSY/EAGAIN retry
 * collection only; terminal errors retain quarantine. Success consumes screen.
 * Caller must not invoke after a successful destroy or collection. */
int remesa_screen_collect(struct pipe_screen *);
/* Explicit attribution: completed CPU clears, never SGX clear submissions. */
/* Normal rendering requests target preservation. false explicitly permits
 * diagnostic discard; it does not establish a kernel load/store capability. */
int remesa_context_set_framebuffer_load(struct pipe_context *,bool preserve);
uint64_t remesa_context_cpu_clear_count(const struct pipe_context *);
/* Completed CPU byte-copy callbacks, never SGX draw/blit submissions. */
uint64_t remesa_context_cpu_copy_count(const struct pipe_context *);
const char *remesa_screen_unavailable_reason(void);
#endif
