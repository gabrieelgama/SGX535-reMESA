# MeeGo and Moblin package archaeology

| package or source name | observed version/date/origin | target and contents established | source quality | relevance to Gate B |
|---|---|---|---|---|
| `xf86-video-psb` | Moblin upstream cited as deprecated in 2009; Ubuntu accepted `0.2.1-1ubuntu2` in 2007 | Poulsbo X.Org 2D driver lineage | SECONDARY / package metadata | no SGX register contract |
| `psb-headers` | named in 2009 Moblin-source reporting | historical headers/source split | LOCATOR-ONLY | source URL no longer available in the cited form |
| `psb-firmware`, `Xpsb`, `xpsb-glx` | named in 2009 package reports and DRI discussion | proprietary/binary 3D stack components existed | SECONDARY; DRI maintainer statement is PRIMARY-INDIRECT | cannot be copied or used as public semantics |
| `libdrm-poulsbo1`, `psb-modules`, `psb-kernel-source` | recorded in Debian package discussion | distribution packaging around PSB | SECONDARY | no recovered source patch closes a gate |
| EMGD packages | Ubuntu historical pages and local EMGD mirror | distinct Intel driver distribution, some binary/user-mode components | SECONDARY/community mirror | not official hardware documentation |
| MeeGo OBS/package paths | search of `build.meego.com`, package names, and public indexes | no retained Poulsbo SGX clock/power/reset manifest reached | none | no evidence added |

The 2009 DRI-devel thread is the strongest public historical package statement: it says the old Moblin X.Org driver was available while the 3D binary pieces were being made public separately and were not open source. This explains why package lineage does not become a reusable or transparent SGX access contract.

Every discovered package reference was followed to a public package record, a deprecated/dead Moblin location, a distribution discussion, or a binary/proprietary boundary. None supplied a kernel patch or manifest defining `CORE_ID`/`CORE_REVISION` access attributes, SGX clocks, power, reset, failure behavior, or recovery.
