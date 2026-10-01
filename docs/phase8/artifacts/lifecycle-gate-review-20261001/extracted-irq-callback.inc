void psb_irq_uninstall(struct drm_device *dev)
{
	struct drm_psb_private *dev_priv = dev->dev_private;
	unsigned long irqflags;
	unsigned int i;

	spin_lock_irqsave(&dev_priv->irqmask_lock, irqflags);

	if (dev_priv->ops->hotplug_enable)
		dev_priv->ops->hotplug_enable(dev, false);

	PSB_WVDC32(0xFFFFFFFF, PSB_HWSTAM);

	for (i = 0; i < dev->num_crtcs; ++i) {
		if (dev->vblank[i].enabled)
			psb_disable_pipestat(dev_priv, i, PIPE_VBLANK_INTERRUPT_ENABLE);
	}

	/* drm_irq_uninstall clears irq_enabled before this callback.  A
	 * direct PM call keeps its registered handler and historical mask. */
	if (!dev->irq_enabled)
		dev_priv->vdc_irq_mask = 0;
	else
		dev_priv->vdc_irq_mask &= _PSB_IRQ_SGX_FLAG |
					  _PSB_IRQ_MSVDX_FLAG |
					  _LNC_IRQ_TOPAZ_FLAG;

	/* These two registers are safe even if display island is off */
	PSB_WVDC32(~dev_priv->vdc_irq_mask, PSB_INT_MASK_R);
	PSB_WVDC32(dev_priv->vdc_irq_mask, PSB_INT_ENABLE_R);

	wmb();

	/* This register is safe even if display island is off */
	PSB_WVDC32(PSB_RVDC32(PSB_INT_IDENTITY_R), PSB_INT_IDENTITY_R);
	spin_unlock_irqrestore(&dev_priv->irqmask_lock, irqflags);
}