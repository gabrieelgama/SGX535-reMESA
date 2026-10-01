static void psb_driver_unload(struct drm_device *dev)
{
	struct drm_psb_private *dev_priv = dev->dev_private;

	if (dev_priv) {
		/* Quiesce KMS users before unregistering the IRQ, while all
		 * private state and register mappings are still alive. */
		if (dev_priv->modeset) {
			struct drm_crtc *crtc;

			drm_kms_helper_poll_disable(dev);
			drm_for_each_crtc(crtc, dev)
				drm_crtc_vblank_off(crtc);
		}
		if (dev->irq_enabled)
			drm_irq_uninstall(dev);

		if (dev_priv->backlight_device)
			gma_backlight_exit(dev);
		psb_modeset_cleanup(dev);

		if (dev_priv->ops->chip_teardown)
			dev_priv->ops->chip_teardown(dev);

		psb_intel_opregion_fini(dev);

		if (dev_priv->pf_pd) {
			psb_mmu_free_pagedir(dev_priv->pf_pd);
			dev_priv->pf_pd = NULL;
		}
		if (dev_priv->mmu) {
			struct psb_gtt *pg = &dev_priv->gtt;

			down_read(&pg->sem);
			psb_mmu_remove_pfn_sequence(
				psb_mmu_get_default_pd
				(dev_priv->mmu),
				pg->mmu_gatt_start,
				dev_priv->vram_stolen_size >> PAGE_SHIFT);
			up_read(&pg->sem);
			psb_mmu_driver_takedown(dev_priv->mmu);
			dev_priv->mmu = NULL;
		}
		psb_gtt_takedown(dev);
		if (dev_priv->scratch_page) {
			set_pages_wb(dev_priv->scratch_page, 1);
			__free_page(dev_priv->scratch_page);
			dev_priv->scratch_page = NULL;
		}
		if (dev_priv->vdc_reg) {
			iounmap(dev_priv->vdc_reg);
			dev_priv->vdc_reg = NULL;
		}
		if (dev_priv->sgx_reg) {
			iounmap(dev_priv->sgx_reg);
			dev_priv->sgx_reg = NULL;
		}
		if (dev_priv->aux_reg) {
			iounmap(dev_priv->aux_reg);
			dev_priv->aux_reg = NULL;
		}
		pci_dev_put(dev_priv->aux_pdev);
		pci_dev_put(dev_priv->lpc_pdev);

		/* Destroy VBT data */
		psb_intel_destroy_bios(dev);

		kfree(dev_priv);
		dev->dev_private = NULL;
	}
	gma_power_uninit(dev);
}
static int test_actual_install(struct drm_device *dev) {
int ret = 0;
	ret = drm_irq_install(dev, dev->pdev->irq);
	if (ret)
		goto out_err;

next_phase_seen = 1; return 0;

out_err:
	psb_driver_unload(dev);
	return ret;

}
static int test_actual_late_init(struct drm_device *dev, int ret) {

	if (ret)
		goto out_err;
	next_phase_seen = 1; return 0;

out_err:
	psb_driver_unload(dev);
	return ret;

}
static void psb_pci_remove(struct pci_dev *pdev)
{
	struct drm_device *dev = pci_get_drvdata(pdev);

	drm_dev_unregister(dev);
	psb_driver_unload(dev);
	drm_dev_put(dev);
}
static struct pci_driver psb_pci_driver = { .remove = psb_pci_remove };
static void __exit psb_exit(void)
{
	pci_unregister_driver(&psb_pci_driver);
}
static void original_driver_unload(struct drm_device *dev)
{
	struct drm_psb_private *dev_priv = dev->dev_private;

	/* TODO: Kill vblank etc here */

	if (dev_priv) {
		if (dev_priv->backlight_device)
			gma_backlight_exit(dev);
		psb_modeset_cleanup(dev);

		if (dev_priv->ops->chip_teardown)
			dev_priv->ops->chip_teardown(dev);

		psb_intel_opregion_fini(dev);

		if (dev_priv->pf_pd) {
			psb_mmu_free_pagedir(dev_priv->pf_pd);
			dev_priv->pf_pd = NULL;
		}
		if (dev_priv->mmu) {
			struct psb_gtt *pg = &dev_priv->gtt;

			down_read(&pg->sem);
			psb_mmu_remove_pfn_sequence(
				psb_mmu_get_default_pd
				(dev_priv->mmu),
				pg->mmu_gatt_start,
				dev_priv->vram_stolen_size >> PAGE_SHIFT);
			up_read(&pg->sem);
			psb_mmu_driver_takedown(dev_priv->mmu);
			dev_priv->mmu = NULL;
		}
		psb_gtt_takedown(dev);
		if (dev_priv->scratch_page) {
			set_pages_wb(dev_priv->scratch_page, 1);
			__free_page(dev_priv->scratch_page);
			dev_priv->scratch_page = NULL;
		}
		if (dev_priv->vdc_reg) {
			iounmap(dev_priv->vdc_reg);
			dev_priv->vdc_reg = NULL;
		}
		if (dev_priv->sgx_reg) {
			iounmap(dev_priv->sgx_reg);
			dev_priv->sgx_reg = NULL;
		}
		if (dev_priv->aux_reg) {
			iounmap(dev_priv->aux_reg);
			dev_priv->aux_reg = NULL;
		}
		pci_dev_put(dev_priv->aux_pdev);
		pci_dev_put(dev_priv->lpc_pdev);

		/* Destroy VBT data */
		psb_intel_destroy_bios(dev);

		kfree(dev_priv);
		dev->dev_private = NULL;
	}
	gma_power_uninit(dev);
}
static void original_pci_remove(struct pci_dev *pdev)
{
	struct drm_device *dev = pci_get_drvdata(pdev);

	drm_dev_unregister(dev);
	original_driver_unload(dev);
	drm_dev_put(dev);
}