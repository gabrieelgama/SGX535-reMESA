# Attempt 03 original-driver recovery

The operator explicitly authorized restoration of the preserved installed original driver and display service. No candidate deployment or fixed-scene client is authorized.

Stages: passive HOLD/hash/identity check; one normal insertion of the exact original installed module; verify original Build ID and PCI/DRM/fbdev binding; normal console rebind only if needed; one slimski start after graphics verification; final passive baseline checks. Every failure stops. No force, reset, reboot, SGX submission, or insertion retry.

Original SHA-256: `7b42a99d157ad00494c358a7663a2daf438ba9fc9a8f2ca5076d28ddeb6adafb`. Original Build ID: `d8dcb4d38b774ad64799d5e13aaedede069371f3`.

Scripts and stdout/stderr/time/hash captures are retained separately from Attempt 03. Credential transport uses the reviewed two-process sudo path; credentials are never saved.

The passive normal-load preview identified exactly two absent required original dependencies: `i2c-algo-bit` and `drm_kms_helper`. The normal `modprobe gma500_gfx` restore is guarded by that exact three-insmod dry-run plan and original dependency metadata. No alternate dependency or install helper is allowed.
