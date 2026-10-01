# CORE_ID helper target build/install phase: STOPPED AT NATIVE BUILD

This is a separate evidence session. H0, P1, FIRST-MMIO-01 and the reviewed helper sources were not modified. No helper binary was executed, no SGX MMIO was attempted, and no sudoers rule or helper was installed.

| Required result | Outcome |
| --- | --- |
| TARGET REVERIFICATION | **PASS**. Pinned SSH and fresh minimal DMI/PCI identity matched retained Dell Inc. Inspiron 1210, board `0X605H`, BIOS A02, i686, BDF `0000:00:02.0`, `8086:8108`, subsystem `1028:02b1`, PCI revision `0x06`. |
| TARGET BUILD | **FAIL**. Native `cc` exited `1` before compiling the source: `core_id_once.c:3:10: fatal error: inttypes.h: No such file or directory`. No alternate flags or package investigation followed. |
| FINAL ELF | N/A; no successful linked candidate. Target compiler reported `cc (Debian 14.2.0-19) 14.2.0`, `i686-linux-gnu`. |
| LOAD ROUTINE DISASSEMBLY | N/A for a final linked target binary. The earlier host object audit is not substituted for this required check. |
| ONE-LOAD DESIGN | **INCONCLUSIVE** for the native linked binary. |
| CANDIDATE SHA-256 | N/A. |
| SUDOERS VALIDATION | **NOT PERFORMED**; the required build and binary-inspection gates did not pass. |
| INSTALLED | **NO**. |
| INSTALLED SHA-256 MATCH | N/A. |
| INSTALLED OWNERSHIP/MODES | N/A. |
| PRIVILEGE RULE | N/A; the reviewed draft remains uninstalled. |
| MMIO ATTEMPT COUNT | **0**. |
| CORE_ID VALUE | **UNKNOWN**. |
| CORE_REVISION | Untouched. |
| GATE B | **BLOCKED**. |
| WHITELIST | `[]`. |
| READY FOR SEPARATE ONE-READ AUTHORIZATION | **NO**. |

The exact three reviewed files were transferred to the private target directory `/home/gama/sgx535-core-id-build-20260928-0327`. Target `sha256sum` matched the local reviewed hashes:

| File | SHA-256 |
| --- | --- |
| `core_id_once.c` | `be9945c8bf507c70ca5162907591afa1b2dadb77b0bf11f2d56454352280ad8f` |
| `load32_once.S` | `fda632137630d9fa6952fbb29733bebfdd7e78e73b800f38d94d496c7798c2ed` |
| `sudoers.draft` | `bdbd3ac6dbcc010060ba2ae96026e041020b5edb0b1036d8b0742a8982e8b9d4` |

The exact build command was `cc -std=c11 -O2 -Wall -Wextra -Werror -fno-lto -fno-pie -no-pie -Wl,-z,relro,-z,now -o sgx535-core-id.candidate core_id_once.c load32_once.S`. Build stdout was empty; stderr was 156 bytes. [Raw captures](raw/) include command arguments, host UTC start/finish, target UTC identity/compile-check timestamps where emitted, exit statuses, stdout/stderr and hashes. [The manifest](manifest.json) hashes every new file. No further target command followed the failed native build.
