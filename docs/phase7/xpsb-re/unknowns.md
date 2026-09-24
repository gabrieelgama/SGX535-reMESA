# Unknown reduction after the Xpsb/libdrm pass

This extends [P7B unknowns](../psb-dri-re/unknowns.md) and [the project register](../../unknowns.md); existing IDs are preserved.

| ID | current state | evidence now available | remaining exact need |
|---|---|---|---|
| P7B-U01 | partially constrained; exact pairing UNKNOWN | both retained ELFs and public PSB kernel-source 4.41.1 embed `5.0.1.0046`, and direct request sizes match; DDX/libdrm mismatches remain | authenticated build/runtime manifest and exact PSB/libdrm/DDX binaries or matching hashes |
| P7B-U02 | partially constrained; full path UNKNOWN | P7E-002 resolves the private mode-3 exit to finalization and index-0 submit; Xpsb's composite path is separate | exact GL frontend invocation, all dirty-state preparation, TA/program semantics and paired kernel acceptance |
| P7B-U03 | still UNKNOWN | Xpsb's revision-branching MMIO option routine is static binary evidence, not Dell core identity | target feature/revision specification and physical identity evidence |
| P7B-U04 | partially constrained | 440-byte `.rodata` range copied to a BO; a subset is read through `XpsbShaderCode` into composite output records; neither action proves a microkernel | bounded instruction/data format, uploaded BO consumer and execution path |
| P7B-U05 | partially constrained | Xpsb direct register-init routine and XHW thread identified | exact power/clock/reset/firmware prerequisites, matching kernel contract and Dell applicability |
| U15/U16 | partially constrained; bootstrap UNKNOWN | X-server XHW setup and static upload are observed in historical bytes | complete matching Poulsbo initialization and firmware/microkernel artifact or authoritative absence proof |
| U18 | partially constrained | candidate libdrm ioctl numbers and archived kernel validation/relocation/fence handlers | matched ABI version, field-by-field binary data flow and complete sync semantics |
| U19/U20 | partially constrained | both binaries contain SGX program/state generation and command paths | clean instruction/packet specification and hardware-revision coverage |

New narrowly scoped questions:

| ID | classification | exact question | narrowest next evidence |
|---|---|---|---|
| P7C-U01 | UNKNOWN exact pairing; compatible-family kernel source found | Which DDX/libdrm/PSB kernel builds were actually paired with the two retained 0.18 ELFs? | build manifest or matching built binaries; see [version pairing](version-pairing.md) |
| P7C-U02 | UNKNOWN | Is the 0.32.0 DDX loader spelling mismatch a packaging/source-only typo or an actual module-load failure? | matching built DDX binary and loader behavior source; no execution required for initial static test |
| P7C-U03 | partially constrained; exact pair UNKNOWN | What is the full XHW BO request/reply layout and ordering for this binary/kernel pair? Operation-zero init fields and scene-info/bind-fire operation numbers match the candidate family (P7H-005). Both scene-info option branches give identical outputs for the frozen 32×32 target, except for cookie word 15, which the routine leaves unwritten (P7H-008). | selected operation-2 fields, later OOM cookie-word-15 behavior and selected-path lifetime correlation; exact paired PSB source/header |
| P7C-U04 | partially constrained | What do the 440 static bytes encode, who consumes the uploaded BO, and is an additional SGX payload required? | [source/XREF trace](static-buffer.md) narrows CPU-side composite use; matched SGX program format, uploaded BO consumer and bootstrap inventory still needed |
| P7C-U05 | **CLOSED for public 0.30-4 package only** | Does that `psb-firmware` package contain an SGX bootstrap input? | source archive/spec show only `msvdx_fw.bin`, requested by kernel MSVDX code; other SGX payloads remain a separate UNKNOWN |
| P7C-U06 | UNKNOWN | Are generic BO/fence wrapper request encodings and structs identical in the actual linked libdrm? | matching `libdrm.so.2` bytes or build provenance |

Newly bounded questions:

| ID | classification | exact question | evidence needed |
|---|---|---|---|
| P7D-U01 | UNKNOWN | Which device address or relocation, if any, makes the uploaded 440-byte BO visible to a GPU consumer? | full data-flow from private `+0x7c` wrapper through mapped BO/device-address use; [current trace](static-buffer.md) finds init/teardown only |
| P7D-U02 | UNKNOWN | What instruction/data format and SGX535 revision, if any, apply to the composite source words and unindexed tail? | applicable primary ISA/format source or a matching historical decoder and confirmed path |
| P7D-U03 | partially constrained; not closed | The frame tracker calls renderer `+0x28` on mode-3 exit, resolving to scene finalization and index-0 submit; swap, flush-helper, framebuffer change and capacity-retry paths can cause exit (P7E-001–P7E-004) | exact frontend call of `+0x18/+0x1c`, full dirty-state sequencing and all scene/fence error paths |
| P7D-U04 | UNKNOWN | Were the public 5.0.1.0046 kernel source, DDX and libdrm actually built and installed with these retained ELFs? | authenticated package/build manifest or exact binary hashes |

The Dell's physical SGX revision, CORE_ID/CORE_REVISION read safety, power/clock/reset requirements, concurrency, CPU read-failure behavior and recovery (U01–U06/U10/U11/U14) are **unchanged UNKNOWNs**. No new static software path is a hardware authorization.
