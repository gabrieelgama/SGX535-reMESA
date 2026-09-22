# Phase 6.1 source-family inventory

This inventory records the independent source families examined in the red-team pass. `PRIMARY-DIRECT` means a vendor document or original source directly says the limited fact cited; `PRIMARY-INDIRECT` means original code, a maintainer patch, or an official document supports surrounding context but not the hardware contract; `SECONDARY` is a contemporaneous report or repackaging; `LOCATOR-ONLY` supplies names, URLs, or provenance.

| family | search/result | best material reached | quality | Poulsbo / SGX535 applicability | disposition |
|---|---|---|---|---|---|
| A. Intel public documentation | `364236`, US15W, SGX535, EMGD queries | Intel EMGD specification update `445348-016US` | PRIMARY-DIRECT | Poulsbo bibliography | identifies `364236`, no contract |
| B. Intel archived indexes | historic Intel URLs and document-number queries | Intel Community RDC reference | LOCATOR-ONLY | SCH | restricted route only |
| C. Intel specification updates | `319536`, `319538`, `445348`, `386599` queries | public Z5xx/SCH update references | PRIMARY-INDIRECT | SCH; not SGX read semantics | no candidate-register result |
| D. Intel SCH/US15W documentation | `319537`, `321422`, `386599` queries | public SCH datasheet/addendum metadata | PRIMARY-DIRECT for titles/scope | SCH | no recovered SGX access section |
| E. Intel historical Linux graphics | symbol/history queries | Linux gma500 source and 2011 LKML import | PRIMARY-INDIRECT | Poulsbo | historical software behavior only |
| F. Intel EMGD material | local provenance snapshots and public references | EMGD community mirror | PRIMARY-INDIRECT / community mirror | historical Poulsbo | read precedent, no access attributes |
| G. Intel PSB material | local PSB snapshots and DRI thread | historical PSB code | PRIMARY-INDIRECT | Poulsbo | read precedent and package lineage |
| H. Imagination public documentation | SGX535/Series5/TRM/ID queries | no applicable public TRM | none | not established | unavailable/not recovered |
| I. Imagination forums | PDS/SGX535 queries | IMG developer-forum posts P5-003/P5-004 | PRIMARY-INDIRECT | SGX535 / Intel scope boundary | support path, not contents |
| J. historical SGX DDK source | checked-in TI and EMGD snapshots | `sgx535defs.h`, init/reset paths | PRIMARY-INDIRECT | SGX535 generic; Poulsbo build target where noted | names/offsets only |
| K. Linux kernel history | local Git and public LKML/Git mirrors | staging import and later refactor patches | PRIMARY-INDIRECT | Poulsbo | history partially available; local clone is depth 1 |
| L. DRM mailing-list history | DRI-devel symbol searches | 2014 unification patch | PRIMARY-INDIRECT | Poulsbo | function moved, semantics unchanged |
| M. LKML history | `psb_get_core_freq`, `TRAP_SGX_PM_FAULT`, IDs | 2011 staging and cleanup patches | PRIMARY-INDIRECT | Poulsbo | historical caution, no contract |
| N. X.Org mailing lists | PSB/Xorg searches | no direct register evidence | none | display stack only | terminated as irrelevant to first read |
| O. X.Org PSB history | `xf86-video-psb` package/version searches | 0.2.1 and later source-package references | SECONDARY / LOCATOR-ONLY | Poulsbo 2D/display | no kernel SGX contract recovered |
| P. freedesktop historical material | DRI and list searches | 2009 DRI-devel thread | PRIMARY-INDIRECT | Poulsbo | says 3D binaries were not open; no register semantics |
| Q. MeeGo | package/source/index queries | no Poulsbo access-contract source | none | historical scope only | terminated: no public contract found |
| R. Moblin | package/source/index queries | Moblin PSB package/source lineage | SECONDARY / LOCATOR-ONLY | Poulsbo | package names and old URL only |
| S. MeeGo OBS/package metadata | `build.meego.com` / package queries | no retained applicable manifest | none | unknown | archive/index unavailable to search |
| T. Moblin packages | `psb-headers`, `xf86-video-psb`, `psb-firmware` queries | 2009 reports and Debian package discussion | SECONDARY | Poulsbo | metadata points to vanished/deprecated origins |
| U. distribution source packages | Debian/Ubuntu/Mandriva package queries | Debian bug 533450; Ubuntu source acceptance | SECONDARY | Poulsbo | package names/versions, no hardware contract |
| V. archived package indexes | Debian, Ubuntu, Yocto query variants | Yocto recipe metadata; archived bug logs | LOCATOR-ONLY | Poulsbo package history | no missing source drop recovered |
| W. Android/Linux SGX535 integrations | `EUR_CR_CORE_*`, DT, Android queries | Android/TI headers and 2023 Series5 DT discussion | PRIMARY-INDIRECT | SGX-generic, not Poulsbo | no transferable PM/read safety |
| X. BSD/other open source | Poulsbo/BSD variants | no applicable implementation located | none | unknown | terminated after no relevant result |
| Y. public patents | Poulsbo/SGX535 semantic variants | no primary register contract located | none | not applicable | excluded from gate evidence |
| Z. recursive citations | documents/threads/package metadata | `319535`, `319537`, `319538`, `321422`, `386599`, old Moblin X.Org URL | mixed | as scoped above | followed to public endpoint or recorded restricted/dead |

The inventory is a research-completeness record, not evidence that every listed family contains a contract. No `SECONDARY` or `LOCATOR-ONLY` item is used for a safety-critical gate `PASS`.
