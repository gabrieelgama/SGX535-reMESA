# `gma_get_core_freq()` history trace

## Reachable history

The checked-in `references/linux` clone has one reachable commit (`9b87fdc9…`); its parent object is unavailable. Its local Git history cannot establish first appearance. Public Linux archives fill part of that gap:

1. The 2011 gma500 staging import includes `psb_get_core_freq()` in the Poulsbo path and the same selector write/read and decode table.
2. A July 2011 gma500 cleanup keeps the helper in `psb_device.c` while moving setup through `psb_ops`.
3. Patrik Jakobsson's 2014 DRI-devel patch, *drm/gma500: Unify _get_core_freq for cdv and psb*, creates `gma_device.c` and moves the helper from `psb_device.c` without changing its transaction or decode.

This is the earliest public path reached in the available archives. It is not proof that no earlier private or vanished PSB tree existed.

## What the code proves

The helper obtains PCI device `00:00.0`, writes `0xD0050300` to configuration offset `0xd0`, reads offset `0xd4`, and converts `clock & 7` to 100, 133, 150, 178, 200, or 266. The target source calls it from Poulsbo chip setup. `psb_backlight_setup()` consumes `dev_priv->core_freq` when calculating `BLC_PWM_CTL` ([P6-001](evidence-matrix.csv), [P6-002](evidence-matrix.csv)).

No retrieved comment, commit message, or Intel reference identifies the selected root-bridge field, calls this the SGX execution clock, states that it clocks SGX, or links it to the SGX aperture. The function’s name is not a hardware definition. It is also prohibited as a passive probe because the selector operation is a PCI configuration write.

An adversarial terminology search also found a PHYTEC board manual that lists a 200 MHz “GFX frequency” for US15WP/US15WPT. That is a secondary board-level label for a different system. It does not define the clocked block, identify the Dell board, establish SGX register-interface clocking, or replace the missing Intel/Imagination contract.

## Historical SGX PM diagnostic

The 2011 staging import carried an optional, disabled `TRAP_SGX_PM_FAULT` macro. When compiled, it tested `inl(dev_priv->apm_base + PSB_APM_STS) & 0x3`, emitted a warning saying an SGX read occurred “when it’s off,” and then called `ioread32` anyway. This is evidence that historical code authors anticipated an SGX-off access condition. It does **not** define the APM bits, establish their applicability to the Dell machine, prevent the read, or state CPU-visible behavior after a violation.

## Conclusion

`core_freq` is proven to be a historical software value used for Poulsbo backlight PWM in the target source path. Whether it reflects a chipset clock shared with other domains, indirectly constrains SGX, or represents any physical SGX frequency remains `UNKNOWN`.
