// SPDX-License-Identifier: GPL-2.0-only
/* One frozen-scene attempt only. A separate opt-in patch installs its ioctl. */
#include <linux/errno.h>
#include <linux/jiffies.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/pci.h>
#include <linux/slab.h>

#include "psb_drv.h"
#include "psb_reg.h"
#include "power.h"
#include "gma500_fixed_backend.h"
#include "gma500_fixed_entry.h"
#include "gma500_fixed_uapi.h"

struct sgx535_fixed_attempt_state {
    struct sgx535_gma500_owner owner;
    struct sgx535_gma500_fixed_backend backend;
};

static DEFINE_MUTEX(fixed_attempt_mutex);
static bool fixed_attempt_used;
/* A held scene is intentionally never freed by an error path. */
static struct sgx535_fixed_attempt_state *fixed_held_attempt;

int sgx535_gma500_fixed_attempt(struct drm_device *dev,
                                 struct sgx535_gma500_fixed_result *result)
{
    struct sgx535_fixed_attempt_state *attempt;
    struct drm_psb_private *dev_priv;
    u32 core_id, core_revision;
    int ret;

    if (!dev || !dev->pdev || !result ||
        dev->pdev->vendor != 0x8086 || dev->pdev->device != 0x8108)
        return -ENODEV;
    result->outcome = SGX535_FIXED_REJECTED;
    result->phase = SGX535_PHASE_EMPTY;
    result->observed_events = 0;
    result->color_observed = 0;
    mutex_lock(&fixed_attempt_mutex);
    if (fixed_attempt_used) {
        ret = -EALREADY;
        goto unlock;
    }
    dev_priv = dev->dev_private;
    if (!dev_priv || !dev_priv->sgx_reg ||
        !gma_power_begin(dev, false)) {
        ret = -ENODEV;
        goto unlock;
    }
    /* These two fixed reads would be part of the separately reviewed action.
     * They are not performed merely by compiling this dormant function. */
    core_id = PSB_RSGX32(PSB_CR_CORE_ID);
    core_revision = PSB_RSGX32(PSB_CR_CORE_REVISION);
    gma_power_end(dev);
    if (core_id != 0x01130000U || core_revision != 0x00010201U) {
        ret = -ENODEV;
        goto unlock;
    }
    fixed_attempt_used = true;
    attempt = kzalloc(sizeof(*attempt), GFP_KERNEL);
    if (!attempt) {
        ret = -ENOMEM;
        goto unlock;
    }
    if (!try_module_get(THIS_MODULE)) {
        ret = -ENODEV;
        goto free_attempt;
    }
    ret = sgx535_gma500_owner_construct(dev, &attempt->owner);
    if (ret)
        goto put_module;
    ret = sgx535_gma500_fixed_backend_begin(&attempt->backend,
                                             &attempt->owner);
    if (ret)
        goto release_owner;
    ret = sgx535_fixed_service_run_once(&attempt->owner.fixed_service,
             &sgx535_gma500_fixed_backend_ops,
             &sgx535_gma500_fixed_backend_status_source,
             &attempt->backend, core_revision, 1U, 5U * HZ, 300000U);
    result->phase = attempt->owner.session.phase;
    result->observed_events = attempt->owner.session.observed_events;
    if (!sgx535_fixed_service_may_release(&attempt->owner.fixed_service)) {
        fixed_held_attempt = attempt;
        result->outcome = SGX535_FIXED_HELD;
        if (!ret)
            ret = -EIO;
        goto unlock;
    }
    if (!ret) {
        result->outcome = SGX535_FIXED_COMPLETED;
        if (!sgx535_gma500_owner_capture_color(&attempt->owner,
                                                 &result->color,
                                                 result->color_bytes))
            result->color_observed = 1;
        else
            ret = -EIO;
    }
    if (sgx535_gma500_fixed_backend_end(&attempt->backend)) {
        fixed_held_attempt = attempt;
        result->outcome = SGX535_FIXED_HELD;
        ret = -EIO;
        goto unlock;
    }
release_owner:
    if (sgx535_gma500_owner_release(&attempt->owner)) {
        fixed_held_attempt = attempt;
        result->outcome = SGX535_FIXED_HELD;
        ret = -EIO;
        goto unlock;
    }
put_module:
    module_put(THIS_MODULE);
free_attempt:
    kfree(attempt);
unlock:
    mutex_unlock(&fixed_attempt_mutex);
    return ret;
}

int sgx535_gma500_fixed_ioctl(struct drm_device *dev, void *data,
                              struct drm_file *file)
{
    struct sgx535_fixed_ioctl *io = data;
    struct sgx535_gma500_fixed_result *result;
    int ret;
    size_t row;

    (void)file;
    if (!io || sgx535_frozen_validate_request(io,
            sizeof(struct sgx535_frozen_request)))
        return -EINVAL;
    result = kzalloc(sizeof(*result), GFP_KERNEL);
    if (!result)
        return -ENOMEM;
    /* The four request words are the only accepted input. The remaining
     * words are overwritten before returning, including on rejection. */
    ret = sgx535_gma500_fixed_attempt(dev, result);
    io->operation_errno = ret;
    io->outcome = result->outcome;
    io->phase = result->phase;
    io->observed_events = result->observed_events;
    io->color_observed = result->color_observed;
    io->color_fnv1a = result->color.fnv1a;
    io->color_nonzero_pixels = result->color.nonzero_pixels;
    for (row = 0; row < 32; row++)
        io->color_row_nonzero[row] = result->color.row_nonzero[row];
    memcpy(io->color_bytes, result->color_bytes,
           sizeof(io->color_bytes));
    kfree(result);
    /* Return zero so DRM copies the bounded outcome back even when the
     * attempt failed after FIRE_POSSIBLE. The outcome is not success. */
    return 0;
}
