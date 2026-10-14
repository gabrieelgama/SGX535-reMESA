/* SPDX-License-Identifier: MIT */
#ifndef REMESA_SHADER_UAPI_H
#define REMESA_SHADER_UAPI_H

#include <linux/types.h>

/* Original-program staging contract. No DRM ioctl number is assigned and no
 * runtime kernel endpoint installs this contract. It is separate from the
 * diagnostic geometry/color ABI. It contains CPU bytes, never GPU addresses.
 * Host validation/staging does not qualify an instruction or permit execution.
 */
#define REMESA_SHADER_ABI_VERSION 1u
#define REMESA_SHADER_RESOURCE_COUNT 4u
#define REMESA_SHADER_DATA_BYTES 4096u
#define REMESA_SHADER_RESOURCE_BYTES 2048u
#define REMESA_SHADER_MAX_ALIGNMENT 4096u
#define REMESA_SHADER_REQUEST_BYTES 4288u

enum remesa_shader_action {
    REMESA_SHADER_STAGE = 1,
    REMESA_SHADER_EXECUTE = 2
};

/* Fixed slot order; neither naming nor storage establishes hardware semantics. */
enum remesa_shader_resource_kind {
    REMESA_SHADER_VERTEX_USSE = 1,
    REMESA_SHADER_FRAGMENT_USSE,
    REMESA_SHADER_VERTEX_PDS,
    REMESA_SHADER_FRAGMENT_PDS
};

enum remesa_shader_resource_state {
    /* CPU-format research body: this state never authorizes an execution
     * mapping. An explicit FS-only laboratory may copy a canonically checked
     * body into a separately admitted exact retained profile; that is not a
     * promotion of this state, the generic EXECUTE action, VS or PDS. */
    REMESA_SHADER_CPU_CANDIDATE = 1,
    /* Successfully encoded by an original encoder; hardware qualification is
     * still absent. This ABI has no state that grants execution permission. */
    REMESA_SHADER_ENCODED_UNQUALIFIED = 2
};

struct remesa_shader_resource {
    __u32 kind;
    __u32 state;
    __u32 offset;     /* byte offset within request.data, not an address */
    __u32 length;     /* exact byte length, not an instruction count */
    __u32 alignment;  /* host storage requirement only */
    __u32 reserved[3];
};

struct remesa_shader_request {
    __u32 version;
    __u32 struct_bytes;
    __u32 action;
    __u32 flags;
    __u64 owner;       /* caller ownership binding; not authentication */
    __u64 generation;  /* immutable snapshot identity within that owner */
    __u32 byte_count;
    __u32 resource_mask;
    __u32 reserved[6];
    struct remesa_shader_resource resources[REMESA_SHADER_RESOURCE_COUNT];
    __u8 data[REMESA_SHADER_DATA_BYTES];
};

/* Unused descriptors, reserved fields, inter-resource padding and unused data
 * must be zero. No pointers, instruction PCs, relocations or hardware-register
 * definitions are smuggled through reserved space. owner/generation must match
 * the receiving software object's independently established lifetime binding.
 */
#endif
