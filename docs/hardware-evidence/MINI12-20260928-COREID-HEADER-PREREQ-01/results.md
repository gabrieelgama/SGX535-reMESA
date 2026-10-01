# CORE_ID helper native-build prerequisite

The pinned SSH endpoint's local package metadata reports architecture `i386`, available `libc6-dev` version `2.41-12+deb13u4`, and dpkg status `unknown ok not-installed`. `/usr/include/inttypes.h` is absent. The prior native build failed at `#include <inttypes.h>` before producing a linked candidate. The narrow missing prerequisite is the target's `libc6-dev` package. No package installation or second build was attempted; operator privilege is required.

The minimal operator command is:

```sh
sudo apt-get install --no-install-recommends libc6-dev
```

The exact pinned-SSH metadata query, separate host and target timestamps, stdout/stderr, exit status and hashes are preserved in `query.*` and [manifest.json](manifest.json). H0, P1, FIRST-MMIO-01, the helper review, and the prior failed-build record were not changed. The helper was not executed. SGX MMIO attempts remain `0`; CORE_ID remains UNKNOWN; CORE_REVISION untouched; Gate B BLOCKED; whitelist `[]`.
