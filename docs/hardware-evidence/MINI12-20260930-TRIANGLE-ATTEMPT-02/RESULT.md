# Frozen triangle Attempt 02: stopped before module removal

The operator authorized only `MINI12-SGX535-REV121-FROZEN-32x32-SEQ1` and
confirmed physical presence. The corrected two-process sudo transport was
used for all root scripts. No credential is recorded in this evidence.

## Completed stages

1. Fresh pinned-SSH passive preflight exited 0 with empty stderr and exactly
   matched the retained 28/28 capture. Fresh root transition inventory also
   exited 0 with empty stderr and exactly matched the retained root capture.
2. Stage 0 found the proposed staging paths absent and `slimski` running.
   The exact candidate module and static i386 client were transferred to
   temporary unprivileged paths. Both target-side SHA-256 values matched the
   pinned artifacts. An initial read-only verification used bare `modinfo`
   from an unprivileged PATH and exited 127; the already observed exact tool
   path `/usr/sbin/modinfo` was used for the repeated read-only verification.
   That verification exited 0 with empty stderr, expected ELF32/i386 types
   and target vermagic. No driver action occurred between the checks.
3. The two files were copied into new root-owned mode-0700
   `/root/sgx535-frozen-seq1/`, with module mode 0600 and client mode 0700.
   Root-side hashes matched
   `934bd97164c803e52d96528b9ec464d587a6a68e2f671aa255407657cc5342cf`
   and
   `758076e2f7e20d0eff2df565d4440c8b3fd3f846922427e770f55ebc472edccf`.
   Candidate vermagic matched `5.10.240-antix.1-486-smp SMP mod_unload
   modversions 486`.
4. The single reviewed `/usr/bin/sv stop` returned `ok: down`. A root check
   found `slimski` and Xorg absent, no root-visible DRM/fb FD holders, and
   the original loaded gma500 Build ID still bound to PCI `0000:00:02.0`.

## Stop guard and restoration

The required `/usr/sbin/modprobe -n -v -r gma500_gfx` preview exited **1**,
produced no removal plan, and reported `Module gma500_gfx is in use`.
`/proc/modules` showed reference count 1 after the graphical client was
gone. The remaining holder is **not identified** by this attempt. The
reviewed procedure requires STOP before module removal on a refused or
unexpected preview. No `modprobe -r`, force operation, PCI unbind or
alternate release was attempted.

A read-only guard confirmed the unchanged original Build ID, original PCI
binding, `card0` presence and `slimski` down. The reviewed service-only
reversal, one `/usr/bin/sv start`, returned `ok: run`. A final pinned-SSH
passive preflight exited 0 with empty stderr and exactly matched the retained
preflight capture, including original module identity and one Xorg.

## Outcome

- **Reviewed transition completed:** no; it stopped at the module-removal
  preview. The original driver remained loaded and was never removed.
- **Candidate module loaded:** no.
- **DRM opened or fixed ioctl issued:** no.
- **SGX work submitted:** no; TA and raster were not fired.
- **TA/raster status and color output:** none, because execution never began.
- **Triangle observed:** no.
- **Active display:** service stop and reviewed service-only restart occurred;
  the final passive state exactly matched the retained baseline.
- **Staged files:** remain at the secure root-owned path and temporary
  unprivileged paths. They were not executed.

The raw captures and per-stage command metadata are retained in this
directory. This single authorized attempt has ended. A future attempt must
first identify the residual module use and qualify an exact nonforced release
path; the current whitelist action cannot advance through its stage-2 guard
as observed. No automatic retry is permitted.
