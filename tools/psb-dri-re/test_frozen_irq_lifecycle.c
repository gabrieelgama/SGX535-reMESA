/* Hardware-free harness for actual extracted gma500 lifecycle code.
 * Kernel boundary doubles enforce object validity and IRQ quiescence order. */
#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#define PAGE_SHIFT 12
#include "extracted-irq-definitions.inc"
struct drm_device;
struct pci_dev { int irq; struct drm_device *driver_data; };
struct pci_driver { void (*remove)(struct pci_dev *); };
#define __exit
struct drm_crtc { bool enabled; };
struct psb_gtt { int sem; unsigned int mmu_gatt_start; };
struct psb_ops {
    void (*chip_teardown)(struct drm_device *);
    void (*hotplug_enable)(struct drm_device *, bool);
};
struct drm_psb_private {
    struct psb_ops *ops;
    struct psb_gtt gtt;
    void *backlight_device, *pf_pd, *mmu, *scratch_page;
    void *vdc_reg, *sgx_reg, *aux_reg, *aux_pdev, *lpc_pdev;
    unsigned int vram_stolen_size;
    bool modeset;
    unsigned int vdc_irq_mask;
    int irqmask_lock;
};
struct drm_device {
    struct drm_psb_private *dev_private;
    struct pci_dev *pdev;
    bool irq_enabled;
    unsigned int num_crtcs;
    struct drm_crtc crtcs[2];
    struct { bool enabled; } vblank[2];
};
#define drm_for_each_crtc(crtc, dev) \
    for ((crtc) = (dev)->crtcs; (crtc) < (dev)->crtcs + (dev)->num_crtcs; ++(crtc))
static int event, poll_event, irq_event, modeset_event, unmap_event, free_event;
static int irq_calls, vblank_calls, install_error, next_phase_seen;
static bool initial_irq;
static bool observe_original;
static int unregister_event, final_put_event;
static struct pci_dev *bound_pci;
static bool callback_locked;
static unsigned int irq_mask_register, irq_enable_register, identity_register;
static unsigned int callback_reads, callback_writes;
#define spin_lock_irqsave(lock, flags) do { (void)(lock); (flags) = 0; \
    assert(!callback_locked); callback_locked = true; } while (0)
#define spin_unlock_irqrestore(lock, flags) do { (void)(lock); (void)(flags); \
    assert(callback_locked); callback_locked = false; } while (0)
static void test_register_write(unsigned int value, unsigned int reg)
{
    assert(callback_locked); ++callback_writes;
    switch (reg) {
    case PSB_HWSTAM: assert(value == 0xffffffffU); break;
    case PSB_INT_MASK_R: irq_mask_register = value; break;
    case PSB_INT_ENABLE_R: irq_enable_register = value; break;
    case PSB_INT_IDENTITY_R: assert(value == identity_register); identity_register = 0; break;
    default: abort();
    }
}
static unsigned int test_register_read(unsigned int reg)
{ assert(callback_locked && reg == PSB_INT_IDENTITY_R); ++callback_reads; return identity_register; }
#define PSB_WVDC32(value, reg) test_register_write((value), (reg))
#define PSB_RVDC32(reg) test_register_read(reg)
static void wmb(void) { assert(callback_writes >= 3); }
static void psb_disable_pipestat(struct drm_psb_private *priv, unsigned int i, unsigned int mask)
{ (void)priv; assert(callback_locked && i < 2 && mask == PIPE_VBLANK_INTERRUPT_ENABLE); }
#include "extracted-irq-callback.inc"

static void valid_release(void)
{ assert(observe_original || !initial_irq || irq_calls == 1); }
static void drm_kms_helper_poll_disable(struct drm_device *dev)
{ assert(dev->dev_private->modeset); poll_event = ++event; }
static void drm_crtc_vblank_off(struct drm_crtc *crtc)
{ assert(poll_event && !irq_event); crtc->enabled = false; ++vblank_calls; ++event; }
static int drm_irq_uninstall(struct drm_device *dev)
{
    unsigned int i;
    assert(dev->irq_enabled && dev->dev_private && dev->dev_private->sgx_reg);
    for (i = 0; i < dev->num_crtcs; ++i) assert(!dev->crtcs[i].enabled);
    dev->irq_enabled = false; /* Exact DRM core order before driver callback. */
    psb_irq_uninstall(dev);
    assert(irq_mask_register == 0xffffffffU && irq_enable_register == 0);
    assert(callback_reads == 1 && !callback_locked);
    ++irq_calls; irq_event = ++event; return 0;
}
static int drm_irq_install(struct drm_device *dev, int irq)
{ assert(irq == 16); dev->irq_enabled = !install_error; return install_error; }
static void gma_backlight_exit(struct drm_device *dev) { (void)dev; }
static void psb_modeset_cleanup(struct drm_device *dev)
{ assert(observe_original || !dev->irq_enabled); valid_release(); modeset_event = ++event; }
static void chip_teardown(struct drm_device *dev)
{ (void)dev; valid_release(); ++event; }
static void psb_intel_opregion_fini(struct drm_device *dev)
{ (void)dev; valid_release(); }
static void psb_mmu_free_pagedir(void *ptr) { (void)ptr; valid_release(); }
static void down_read(int *sem) { (void)sem; }
static void up_read(int *sem) { (void)sem; }
static void *psb_mmu_get_default_pd(void *ptr) { return ptr; }
static void psb_mmu_remove_pfn_sequence(void *p, unsigned int va, unsigned int size)
{ (void)p; (void)va; (void)size; valid_release(); }
static void psb_mmu_driver_takedown(void *p) { (void)p; valid_release(); }
static void psb_gtt_takedown(struct drm_device *dev) { (void)dev; valid_release(); }
static void set_pages_wb(void *p, int n) { (void)p; (void)n; }
static void __free_page(void *p) { (void)p; valid_release(); }
static void iounmap(void *p) { (void)p; valid_release(); if (!unmap_event) unmap_event = ++event; }
static void pci_dev_put(void *p) { (void)p; }
static void psb_intel_destroy_bios(struct drm_device *dev) { (void)dev; }
static void kfree(void *p) { (void)p; valid_release(); free_event = ++event; }
static void gma_power_uninit(struct drm_device *dev) { (void)dev; valid_release(); }
static struct drm_device *pci_get_drvdata(struct pci_dev *pdev)
{ assert(pdev == bound_pci); return pdev->driver_data; }
static void drm_dev_unregister(struct drm_device *dev)
{
    /* Exact MODESET/GEM unregister does not own this ordinary IRQ allocation;
     * its driver has no drm_driver.unload hook. Device/private state is alive. */
    assert(dev->dev_private && !free_event && !unmap_event);
    unregister_event = ++event;
}
static void drm_dev_put(struct drm_device *dev)
{
    assert(unregister_event && !dev->dev_private && free_event);
    assert(observe_original || (!dev->irq_enabled && irq_calls == 1));
    final_put_event = ++event;
}
static void pci_unregister_driver(struct pci_driver *driver)
{ assert(bound_pci && driver->remove); driver->remove(bound_pci); }

/* The test runner inserts the actual patched psb_driver_unload definition and
 * IRQ-install control-flow statements here; the harness supplies no substitute. */
#include "extracted-lifecycle.inc"

int main(int argc, char **argv)
{
    static struct pci_dev pci = { .irq = 16 };
    static struct psb_ops ops = {chip_teardown, NULL};
    static struct drm_psb_private priv;
    static struct drm_device dev;
    int scenario;
    assert(argc == 2); scenario = atoi(argv[1]);
    priv.ops = &ops; priv.pf_pd = priv.mmu = priv.scratch_page = (void *)1;
    priv.vdc_reg = priv.sgx_reg = priv.aux_reg = (void *)1;
    priv.modeset = scenario == 0 || scenario >= 6;
    dev.dev_private = &priv; dev.pdev = &pci; dev.num_crtcs = 2;
    pci.driver_data = &dev; bound_pci = &pci;
    dev.irq_enabled = initial_irq = scenario == 0 || scenario == 2 || scenario >= 6;
    dev.crtcs[0].enabled = dev.crtcs[1].enabled = priv.modeset;
    priv.vdc_irq_mask = _PSB_IRQ_SGX_FLAG | _PSB_IRQ_MSVDX_FLAG |
                        _LNC_IRQ_TOPAZ_FLAG | 0x80U;
    identity_register = 0x123U;
    if (scenario >= 10) {
        if (scenario == 10) psb_pci_remove(&pci);
        else if (scenario == 11) psb_exit();
        else {
            assert(scenario == 12); observe_original = true;
            original_pci_remove(&pci);
        }
        assert(!dev.dev_private && unregister_event && final_put_event);
        if (observe_original) {
            assert(dev.irq_enabled && irq_calls == 0);
            assert(unregister_event < free_event && free_event < final_put_event);
        } else {
            assert(irq_calls == 1 && !dev.irq_enabled && vblank_calls == 2);
            assert(unregister_event < poll_event && poll_event < irq_event);
            assert(irq_event < modeset_event && irq_event < unmap_event);
            assert(unmap_event < free_event && free_event < final_put_event);
        }
        return 0;
    }
    if (scenario >= 8) {
        unsigned int preserved = _PSB_IRQ_SGX_FLAG | _PSB_IRQ_MSVDX_FLAG | _LNC_IRQ_TOPAZ_FLAG;
        dev.irq_enabled = scenario == 9;
        psb_irq_uninstall(&dev);
        assert(priv.vdc_irq_mask == (scenario == 9 ? preserved : 0));
        assert(irq_enable_register == priv.vdc_irq_mask);
        assert(irq_mask_register == ~priv.vdc_irq_mask);
        assert(callback_reads == 1 && !callback_locked);
        assert(!irq_calls && !free_event && !unmap_event);
        return 0;
    }
    if (scenario == 3) dev.dev_private = NULL;
    if (scenario >= 6) {
        int error = scenario == 6 ? -12 : 0;
        assert(test_actual_late_init(&dev, error) == error);
        assert(next_phase_seen == !error);
        if (error) assert(!dev.dev_private && !dev.irq_enabled && irq_calls == 1);
        else { psb_driver_unload(&dev); assert(!dev.dev_private); }
        return 0;
    }
    if (scenario >= 4) {
        int ret;
        initial_irq = false; install_error = scenario == 4 ? -16 : 0;
        ret = test_actual_install(&dev);
        assert(ret == install_error);
        assert(next_phase_seen == !install_error);
        if (!install_error) { initial_irq = true; psb_driver_unload(&dev); }
        return 0;
    }
    psb_driver_unload(&dev);
    assert(!dev.dev_private);
    assert(irq_calls == (initial_irq ? 1 : 0));
    if (scenario == 0) {
        assert(vblank_calls == 2);
        assert(poll_event < irq_event && irq_event < modeset_event);
        assert(irq_event < unmap_event && irq_event < free_event);
    } else assert(vblank_calls == 0 && !poll_event);
    return 0;
}
