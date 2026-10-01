# Fixed CORE_REVISION helper: build, installation, single read

The preceding operator-reported CORE_ID observation is preserved separately in ../MINI12-20260928-COREID-OPERATOR-01/results.md: raw 0x01130000. No CORE_ID reread occurred here.

## Register and address

Retained SGX535 header docs/archaeology-data/H535.txt defines EUR_CR_CORE_REVISION at SGX-relative 0x0014 and four eight-bit field masks. Retained Poulsbo source PSB_psb_reg_h.txt agrees at 0x0014; PSB_RSGX32 in PSB_psb_drv_h.txt uses ioread32, establishing a 32-bit historical access. With retained BAR0 0xd8100000 and SGX aperture at BAR0 + 0x40000, the intended location is 0xd8100000 + 0x40000 + 0x14 = 0xd8140014, aligned 32-bit. These source facts do not independently prove read safety.

## Build and installation

Three fixed-source files in tools/sgx535-probe/privileged-core-revision/ were transferred with matching SHA-256. Native i386 build command: cc -std=c11 -O2 -Wall -Wextra -Werror -fno-lto -fno-pie -no-pie -Wl,-z,relro,-z,now -o sgx535-core-revision.candidate core_revision_once.c load32_once.S. Build exit 0. Candidate: ELF32 Intel 80386, SHA-256 e9e0206eff7fa62d60e7b0bc7df77e842301e7498262865cb8c2a05a45450454.
Final linked load routine: mov eax,DWORD PTR [esp+0x4]; mov eax,DWORD PTR [eax]; ret. The linked main has one call with pointer mapped_page + 0x14. The explicit mapped-address load is 32-bit; ordinary stack/code/data loads and syscalls are distinct. Inspection does not prove one physical downstream PCI transaction.
Target passive identity, BAR0, driver, resource0 and directory checks passed. Installed binary: /usr/local/libexec/sgx535-core-revision, root:root 0755, SHA-256 equal to candidate. Sudoers: /etc/sudoers.d/sgx535-core-revision, root:root 0440, SHA-256 7b5eda756a7800185dce238100d95e2911beef2f46570fe4ebf8acdb937e5803, visudo parsed OK. Exact no-argument rule: gama ALL=(root) NOPASSWD:NOSETENV: /usr/local/libexec/sgx535-core-revision "". Existing CORE_ID helper and sudoers were not modified.

## Single authorized attempt

Immediate passive DMI/PCI/BAR0 check passed at target UTC 2026-09-28T02:03:29.012266+00:00. Exactly one invocation of /usr/bin/sudo -n /usr/local/libexec/sgx535-core-revision occurred via raw/single-core-revision-attempt.stdin. No retry.
Target start UTC: 2026-09-28T02:03:42.346283+00:00
Target finish UTC: 2026-09-28T02:03:42.473220+00:00
Host capture start UTC: 2026-09-28T04:13:17.382961+00:00
Host capture finish UTC: 2026-09-28T04:13:18.691818+00:00
Helper exit status: 0; SSH/wrapper exit status: 0.
Exact helper stdout bytes: b'CORE_REVISION=0x00010201\n' (raw/helper.stdout).
Exact helper stderr bytes: b'' (raw/helper.stderr).
Target responsive afterward: YES. SSH returned the post-invocation wrapper result. No later MMIO occurred.

HW-OBSERVED raw CORE_REVISION value: 0x00010201. The value was returned by the intended one-load fixed-register helper. SGX535 header masks parse this raw value into designer 0, major 1, minor 2, maintenance 1; this is source-defined field extraction, not independently proven stepping, historical rev label, errata conclusion, or general MMIO safety. CORE_ID remains raw 0x01130000 with its separate operator-reported provenance. Gate B remains BLOCKED; whitelist [].

Host and target clocks show different UTC times; timestamps are preserved separately without assuming clock agreement. No further SGX MMIO action is authorized by this record.
