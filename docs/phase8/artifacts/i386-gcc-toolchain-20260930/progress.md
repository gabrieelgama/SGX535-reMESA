# Toolchain qualification ledger

Scope: provision and qualify only; no kernel preparation/rebuild in this turn. The plan labels kernel preparation/build as a future implementation plan, and the user limits this task to the smallest toolchain step.

- Starting repository state and file hashes captured before edits.
- No target contact or deployment is permitted.
- Use a fresh rootless local package environment; do not modify global apt/dpkg state.
- Existing main working tree remains in place as requested; provisioning and smoke outputs are outside it. No production-code behavior change is planned.

- Signed isolated APT update passed. Exact planned cross versions/hashes present.
- Rootless extraction chosen, as permitted by the plan; executable/library/specs relocation must pass smoke checks. Native SSL dependency upgrades are extracted locally only; no system packages are changed.

- Smoke-test ruling: extra -Wextra/-Werror introduced by the smoke rejected an existing fixdep sign comparison. Use retained KBUILD_HOSTCFLAGS (-Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89), not a modified source or suppressed production guard. Original failing output retained.
- Rootless cross libc link scripts use /usr/i686-linux-gnu paths; explicit --sysroot=<local extracted root> is required and passed both link and retained cc-can-link checks.

- Native SSL rootless probe first missed /usr/include/aarch64-linux-gnu/openssl/opensslconf.h. Verified its packaged location; explicit extracted ordinary + multiarch include paths plus native library path passed. No source/header edit or system install. Retained failed and passed commands.
- Core cross/native qualification complete; full 193-test suite passed in 11.443 s, three UBSan harnesses pass, two dry hashes unchanged.

- All package hashes/content identities match authenticated metadata. All 20 executed tool identities are ARM64; no missing runtime dependencies.
- Target GCC 14.2.0 (Debian 14.2.0-19cross1) / binutils 2.44 qualified for smoke obligations. Cross objects are ELF32 little-endian e_machine=3; native helper/fixdep are ELF64 AArch64. No x86 program executed.
- Target qualified symvers/config/generated-reference hashes remain unchanged; original imports 222/222 PASS, old candidate 154 mismatches REJECT.
- Kernel preparation and candidate rebuild intentionally NOT performed: user authorized the smallest toolchain step, and build-plan preparation/rebuild is explicitly a future implementation stage.
