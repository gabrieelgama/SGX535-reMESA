# CORE_ID helper target build and installation: completed without execution

This resumed the previously authorized build/install phase at its failed native build step after the operator installed `libc6-dev:i386`. H0, P1, FIRST-MMIO-01, helper review and prior failed-build evidence remain separate and unchanged. No helper execution, SGX MMIO, `/dev/mem` access or PCI resource read occurred.

| Required final field | Result |
| --- | --- |
| TARGET REVERIFICATION | **PASS retained from the immediately preceding build phase**; not repeated. Pinned SSH identity was verified before the three reviewed files were transferred. |
| TARGET BUILD | **PASS**. Exact reviewed native command exited `0`, empty stdout/stderr. Compiler `cc (Debian 14.2.0-19) 14.2.0`, target `i686-linux-gnu`. |
| FINAL ELF | ELF32 little-endian, `EXEC`, Intel 80386; interpreter `/lib/ld-linux.so.2`; Build ID `5e18f0f76b3d4a8c308cfa59c8ee77244561d095`; dynamic dependency `libc.so.6`. No candidate execution. |
| LOAD ROUTINE DISASSEMBLY | `080497d5: mov eax,DWORD PTR [esp+0x4]`; `080497d9: mov eax,DWORD PTR [eax]`; `080497db: ret`. Bytes `8b 44 24 04; 8b 00; c3`. |
| ONE-LOAD DESIGN | **PASS at linked-instruction level**. `main` has one call at `0x8049540` after `lea eax,[esi+0x10]`; the mapped page is one `0x1000` page at BAR offset `0x40000`. The first assembly load is an ordinary stack argument read; the second is the sole intended mapped-address 32-bit load. No claim of one downstream physical PCI transaction or hardware safety. Reviewed source hashes matched on target. |
| CANDIDATE SHA-256 | `050b110abdb11d301f4cc75ec3d4370a5192a354a61d86f86d3d9023bee82e58`. |
| SUDOERS VALIDATION | **PASS**. Exact draft and installed file parsed by `/usr/sbin/visudo -cf`. The first `visudo` attempt without `/usr/sbin/` failed with command-not-found; the absolute-path invocation succeeded without changing the rule. |
| INSTALLED | **YES**. Only the candidate and exact reviewed sudoers draft were installed; the helper directory already existed. |
| INSTALLED SHA-256 MATCH | **YES**: installed binary hash equals candidate hash. Installed sudoers hash equals reviewed draft hash `bdbd3ac6dbcc010060ba2ae96026e041020b5edb0b1036d8b0742a8982e8b9d4`. |
| INSTALLED OWNERSHIP/MODES | Binary `root:root 0755`, regular file; sudoers `root:root 0440`, regular file. `/usr`, `/usr/local`, `/usr/local/libexec`, `/etc`, `/etc/sudoers.d` remained root-owned real directories without group/other write access. |
| PRIVILEGE RULE | `gama ALL=(root) NOPASSWD:NOSETENV: /usr/local/libexec/sgx535-core-id ""`. `sudo -n -ll` matched this exact no-argument rule and displayed `!setenv, !authenticate`. An extra-argument listing matched a **pre-existing** `/etc/sudoers` `ALL` entry instead, not this new rule. This session did not create or change that older entry. No helper invocation was used to test permissions. |
| MMIO ATTEMPT COUNT | **0**. |
| CORE_ID VALUE | **UNKNOWN**. |
| CORE_REVISION | Untouched. |
| GATE B | **BLOCKED**. |
| WHITELIST | `[]`. |
| READY FOR SEPARATE ONE-READ AUTHORIZATION | **YES for the reviewed build/install artifact**; no MMIO authorization is implied. The existing broader authenticated sudo policy is disclosed above for operator judgment. |

The exact build command was `cc -std=c11 -O2 -Wall -Wextra -Werror -fno-lto -fno-pie -no-pie -Wl,-z,relro,-z,now -o sgx535-core-id.candidate core_id_once.c load32_once.S` in `/home/gama/sgx535-core-id-build-20260928-0327`. Linked-binary inspection used `file`, `readelf -W -h -l -d`, `objdump -drw -Mintel` on `main` and `sgx535_load32_once`, `nm -n`, and `sha256sum`. `ldd` and the helper were never run.

The only privileged installation commands performed were:

```sh
/usr/bin/sudo -n /usr/bin/install -o root -g root -m 0755 sgx535-core-id.candidate /usr/local/libexec/sgx535-core-id
/usr/bin/sudo -n /usr/bin/install -o root -g root -m 0440 sudoers.draft /etc/sudoers.d/sgx535-core-id
```

All raw command stdout/stderr, exit statuses, exact SSH argv and host UTC timestamps are preserved under `raw/`; target source hashes were carried forward from the previous build session and reconfirmed in the candidate metadata capture. [Manifest](manifest.json) hashes this session. **Stop before execution.**
