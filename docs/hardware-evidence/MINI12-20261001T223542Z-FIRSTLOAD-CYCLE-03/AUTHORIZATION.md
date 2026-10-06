# Cycle03 authorization and bounds

The operator explicitly authorized exactly ONE fresh non-SGX first-load/recovery
cycle described in docs/phase8/visible-triangle-readiness.md. Abort on any failed
prerequisite or unexpected state. No SGX submission, fixed ioctl, triangle, hot
replacement, restaging, overwrite or retries. Existing-file receipt matching
precedes selection. Both boot captures must finish within their 120-second
ceilings. No automatic reset; operator machine boundary and manual STOCK only.

Physical/control and local-sudo prerequisites are pending fresh confirmation.
SGX Gate B BLOCKED; SGX whitelist []; no SGX permission is created here.
