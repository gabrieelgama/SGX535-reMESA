#ifndef SGX535_GMA500_FIXED_ENTRY_H
#define SGX535_GMA500_FIXED_ENTRY_H

#include <linux/types.h>
#include "frozen_kernel_contract.h"

struct drm_device;
struct drm_file;

enum sgx535_gma500_fixed_outcome {
    SGX535_FIXED_REJECTED = 1,
    SGX535_FIXED_COMPLETED,
    SGX535_FIXED_HELD
};

struct sgx535_gma500_fixed_result {
    u32 outcome;
    u32 phase;
    u32 observed_events;
    u32 color_observed;
    u32 diagnostic_stage_reached;
    u32 diagnostic_failure_stage;
    u32 diagnostic_observation_stage;
    u32 diagnostic_failure_source;
    s32 diagnostic_raw_result;
    u32 diagnostic_load_flags;
    u32 diagnostic_status1;
    u32 diagnostic_status2;
    u32 diagnostic_initend;
    struct sgx535_frozen_color_summary color;
    u8 color_bytes[4096];
};

/* Internal one-shot entry. The separate ioctl patch is not authorization to
 * install or invoke it; every target action requires exact Gate B review. */
int sgx535_gma500_fixed_attempt(struct drm_device *dev,
                                 struct sgx535_gma500_fixed_result *result);
int sgx535_gma500_fixed_ioctl(struct drm_device *dev, void *data,
                              struct drm_file *file);

#endif
