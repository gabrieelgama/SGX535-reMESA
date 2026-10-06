# Cycle05 authorization

The operator explicitly authorized ONE new non-SGX first-load/recovery cycle using the unchanged staged files and v2 procedure, solely to capture the missing experimental boot ID, derivative identity, hook trace and ownership and verify STOCK recovery. No restaging, hot replacement, retry or SGX operation. Gate B remains BLOCKED and SGX whitelist [] unless evidence establishes the required predicates. This is a separate cycle; Cycle04 remains spent and unchanged.

Bounds and guards are unchanged: 600-second operator boot watch; 1,200-second automatic kernel-clock capture ceiling; one 40-second connection per capture with a 35-second root child; existing staged-file creation receipts; reviewed physical/menu/identity/health/ownership guards. Recovery uses the operator machine boundary, never hot restoration. No new code/image/module is part of this cycle.
