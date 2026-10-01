# CORE_ID DMI-fix helper replacement: installed, not executed

The installed helper SHA-256 initially matched known v1 `050b110abdb11d301f4cc75ec3d4370a5192a354a61d86f86d3d9023bee82e58`. No helper invocation was made. The two reviewed DMI-fix source files were transferred to `/home/gama/sgx535-core-id-dmi-fix-build-20260928` and their target hashes matched local reviewed hashes:

- `core_id_once.c`: `a01ee44d44b9b7c5a33c6c1219e42cff9d095acad2adc52b2eca104ec047b13b`
- `load32_once.S`: `fda632137630d9fa6952fbb29733bebfdd7e78e73b800f38d94d496c7798c2ed`

The exact reviewed native build command succeeded with empty stdout/stderr:

```sh
cc -std=c11 -O2 -Wall -Wextra -Werror -fno-lto -fno-pie -no-pie -Wl,-z,relro,-z,now -o sgx535-core-id.candidate core_id_once.c load32_once.S
```

The candidate is ELF32 Intel i386, dynamically linked to `libc.so.6`, interpreter `/lib/ld-linux.so.2`, Build ID `d7271a6cb12ce0d9817867b88bf71062d4fc73d9`. Candidate SHA-256: `dd3993db4835d1e51848ab2cc6d6a647bebd5e5d38c6cad6c3848584b307a20d`.

The final linked `sgx535_load32_once` routine is byte-identical to v1: `8b 44 24 04` (`mov eax,[esp+4]`), `8b 00` (`mov eax,[eax]`), `c3` (`ret`). `main` has one call to it, after computing mapped page + `0x10`; the one-page mapping uses BAR offset `0x40000`. The C source differs from v1 only in the exact DMI `product_name` literal and its explanatory comment. Machine-code inspection establishes one explicit mapped-address 32-bit load instruction, not one guaranteed downstream PCI transaction or SGX safety.

Only the helper binary was replaced, using `/usr/bin/sudo -n /usr/bin/install -o root -g root -m 0755 sgx535-core-id.candidate /usr/local/libexec/sgx535-core-id`. The immediate final read-only check returned installed SHA-256 `dd3993db4835d1e51848ab2cc6d6a647bebd5e5d38c6cad6c3848584b307a20d`, matching the candidate, with owner/group `0:0`, mode `0755`, regular file. **No sudoers file was changed. No command followed that final target check.**

The v2 helper was not executed. No SGX MMIO occurred in this replacement phase. MMIO attempt count remains `0`; CORE_ID UNKNOWN; CORE_REVISION untouched; Gate B BLOCKED; whitelist `[]`. Raw stdout/stderr, command argv, host UTC timestamps and hashes are in `raw/` and [manifest.json](manifest.json).
