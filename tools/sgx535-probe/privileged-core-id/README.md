# CORE_ID one-read privilege design — review only

**No helper is installed, no sudoers rule is installed, and no SGX MMIO attempt is authorized by this design.** FIRST-MMIO-01 is closed as NOT EXECUTED. MMIO attempt count: **0**. CORE_ID value: **UNKNOWN**. Gate B: **BLOCKED**. Whitelist: `[]`.

## Purpose and retained evidence

This is a purpose-built i386 helper for the Dell Inspiron 1210 / Atom Z520 at PCI function `0000:00:02.0`. [H0's installed-module check](../../../docs/hardware-evidence/MINI12-20260927-H0/installed-module-static-check.md) selects `psb_chip_ops` for `8086:8108` and maps the SGX aperture at BAR0 + `0x40000` for `0x8000` bytes. [FIRST-MMIO-01's preflight](../../../docs/hardware-evidence/MINI12-20260928-FIRST-MMIO-01/preflight-decision.md) retains BAR0 `0xd8100000–0xd817ffff`, `CORE_ID` offset `0x10`, and historical 32-bit access evidence. Thus the proposed address is `0xd8100000 + 0x40000 + 0x10 = 0xd8140010`. These are software and OS-visible facts, not a safe-read contract.

## Helper design and source code

[core_id_once.c](core_id_once.c) accepts no arguments and never parses an address. It checks exact DMI values, PCI BDF path/vendor/device/subsystem/revision, `gma500` binding, loaded module, selected OS-visible state, BAR0 start/end/flags, arithmetic, page size and `resource0` size before the load. Mismatch or setup failure exits with a short `REFUSE` diagnostic. [load32_once.S](load32_once.S) is an i386 cdecl routine with a stack-argument load, one `movl (%eax), %eax` from the mapped register address, then `ret`. It has no retry or store. The draft [sudoers rule](sudoers.draft) names only the final helper path with the sudoers `""` argument restriction.

The expected installed binary is `/usr/local/libexec/sgx535-core-id`, owned `root:root`, mode `0755`, in root-owned directories not writable by `gama`. The installed `/etc/sudoers.d/sgx535-core-id` should be `root:root` mode `0440`. The repository source and draft are review artifacts owned by the workspace user; they are **not** the installed helper or rule. The final binary must be built and audited on the i686 target before root-owned installation. Do not execute it during build or installation review.

## Exact mechanism and why it is narrow

The helper opens only `/sys/bus/pci/devices/0000:00:02.0/resource0` with `O_RDONLY|O_CLOEXEC|O_NOFOLLOW`, maps one 4096-byte BAR0 page with `PROT_READ|MAP_SHARED` at file offset `0x40000`, and calls the assembly load on mapped byte `0x10`. The intended physical byte address is `0xd8140010`; the operand is aligned and 32 bits wide. No write mapping, `/dev/mem` open, raw address argument, BAR scan, neighboring dereference, or automatic retry exists. Cleanup and printing follow the load.

The [Linux PCI sysfs documentation](https://docs.kernel.org/5.10/PCI/sysfs-pci.html) defines `resource` as the host-address text and `resource0` as the mmapable PCI resource. The [x86 PAT documentation](https://docs.kernel.org/5.10/x86/pat.html) lists plain PCI sysfs resource mappings as UC- and distinguishes `resource_wc`; only the plain `resource0` path is selected here. The one-load claim is at the **helper instruction level**: the isolated assembly's mapped-pointer dereference is one 32-bit `movl`. It is **not** proof that every kernel/CPU/bridge/device path emits exactly one downstream physical transaction. A page mapping spans 4096 bytes even though the code dereferences only four; the VMA does not itself request a range read in the documented mapping model, but mapping implementation, attributes and device effects on the installed kernel remain a pre-execution review condition.

Mechanism comparison:

| Mechanism | Decision |
| --- | --- |
| Plain PCI `resource0` mmap, one-page read-only map | Selected for the narrow BAR-relative address and documented uncached mapping behavior; fail closed if mmap is refused. |
| `/dev/mem` mmap or `pread` | Rejected: addresses system physical memory, adds `STRICT_DEVMEM`/attribute uncertainty, and `pread` does not give an auditable single CPU load instruction. |
| `resource0` `read`/`pread` | Rejected: PCI MMIO sysfs resource interface is documented as mmapable; a kernel copy path would obscure transaction count. |
| Existing DRM/SGX ioctl | Rejected: no proven one-register read ABI; experimentation is outside scope. |
| New kernel module or driver change | Rejected: installation/loading or replacing the active mapping is broader than this experiment. |

## Build and audit commands for the operator (future, no execution)

On the verified i686 target, from a copied, trusted source directory, build as an ordinary user:

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -fno-lto -fno-pie -no-pie \
  -Wl,-z,relro,-z,now -o sgx535-core-id.candidate \
  core_id_once.c load32_once.S
file sgx535-core-id.candidate
objdump -drw --disassemble=sgx535_load32_once sgx535-core-id.candidate
sha256sum sgx535-core-id.candidate core_id_once.c load32_once.S sudoers.draft
visudo -cf sudoers.draft
```

Expected architecture is **ELF 32-bit i386**. The complete load routine must disassemble as `movl 4(%esp),%eax; movl (%eax),%eax; ret`, with exactly one call from `main`. Verify the linked binary and its hash before installation; stop if the assembly or call site differs. `objdump`, `file`, `sha256sum` and `visudo` availability on the target has **not** been rechecked in this cycle. Compilation, disassembly and syntax checks do not execute the helper or touch MMIO.

Workspace verification on 2026-09-28: an ARM64-host C syntax check with `__i386__` defined passed with `-Wall -Wextra -Werror`. Clang assembled the load routine as an i386 relocatable object; `llvm-objdump` showed exactly `8b 44 24 04; 8b 00; c3` for that routine. This is **not** a target build or linked-binary audit. The workspace lacks an i386 userspace sysroot for a complete C build, and the sudoers draft has not been checked with target `visudo`.

## Exact sudoers rule and future operator installation

The complete proposed rule is:

```sudoers
gama ALL=(root) NOPASSWD:NOSETENV: /usr/local/libexec/sgx535-core-id ""
```

In sudoers, the exact `""` argument form permits **no command-line arguments**; an omitted argument specification would be broader. `NOSETENV` denies sudo command-line environment overrides. The rule contains no shell, interpreter, editor, file permission command or wildcard. It is a path restriction, not a defense against replacing that path; root ownership and non-writable parent directories are essential. The helper also rejects `argc != 1`. The [sudoers manual](https://www.sudo.ws/docs/man/1.9.14/sudoers.man.pdf) documents the empty-string argument and `NOSETENV` forms. The proposed rule grants invocation of this one binary; it cannot by itself enforce the one-read lifetime, so authorization for execution remains separate.

**Do not run these commands in this preparation cycle.** After reviewing the built binary and rule, an operator with an existing administrative path could install them with:

```sh
sudo install -d -o root -g root -m 0755 /usr/local/libexec
sudo install -o root -g root -m 0755 sgx535-core-id.candidate /usr/local/libexec/sgx535-core-id
sudo sha256sum /usr/local/libexec/sgx535-core-id
sudo stat -c '%U:%G %a %F %n' /usr /usr/local /usr/local/libexec /usr/local/libexec/sgx535-core-id
sudo install -o root -g root -m 0440 sudoers.draft /etc/sudoers.d/sgx535-core-id
sudo visudo -cf /etc/sudoers.d/sgx535-core-id
sudo stat -c '%U:%G %a %F %n' /etc/sudoers.d/sgx535-core-id
```

Before installation, verify that the destination binary path is absent and that every parent directory is a real root-owned directory not writable by `gama`. After installing the binary, compare its hash with the reviewed candidate **before** installing the sudoers rule; stop if it differs. If sudoers validation fails, remove the installed rule immediately. No installation step invokes the helper. Do not use `sudo` for the helper until a separate, explicit one-attempt authorization is given. The proposed rollback, for an operator with existing administrative access, is:

```sh
sudo rm -f /etc/sudoers.d/sgx535-core-id
sudo rm -f /usr/local/libexec/sgx535-core-id
```

## Ways it could still fail and unresolved safety

The installed kernel might deny `resource0` mmap through exclusivity, lockdown or platform policy. The target's toolchain or loader could produce a different binary; the binary must be audited again. A read-only VMA forbids user stores but does not establish device read-side effects. CPU memory-type aliasing, speculation, bridge behavior, device width handling, and faults/hangs can defeat the simple one-instruction-to-one-transaction expectation. Mapping the page could fail, or an MMIO load could fault/hang. Checking sysfs state before mapping cannot lock the driver's lifetime or PM state; unbind, suspend, reset or clock/power changes can race after checks. OS `active` and `control=on` do not prove SGX internal readiness. Physical SGX core revision, register side effects, errata, bounded CPU failure and recovery remain unproved. Gate B rows 03–10 and 12–16 therefore remain unresolved, Gate B **BLOCKED**, whitelist `[]`.

The helper deliberately does not request a power hold, change PM, load a module, issue an ioctl, or acquire a driver lock. If a future review concludes that the mapping or one-load behavior cannot be established on the installed kernel, **do not install or execute it**. The prior one-read authorization expired without an attempt; this document grants none.
