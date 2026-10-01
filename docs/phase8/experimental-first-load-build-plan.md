# Experimental first-load image implementation plan

> Execution: inline, offline only; preserve the working tree and all historical inputs. No commits or deployment.

**Goal:** produce one isolated initramfs and non-saving GRUB entry whose actual hook is qualified offline.
**Spec:** the operator's current 13-part construction request and [captured stock architecture](first-load-stock-boot-observation.md).
**Architecture:** preserve the early archive and all main-archive records except `/init`; add a private pinned derivative payload and one hook before init-top/udev. Use explicit normal `insmod` for the fixed topologically ordered dependency closure, avoiding alias races and new modprobe indices. Preserve existing indices byte-exact. Hold boot continuation on any failure.

## Constraints and review focus

- No target contact, module execution on this host, candidate rebuild, CRC edits or stock-input mutation.
- Actual hook logic is tested with I/O boundaries substituted; decisions are not rewritten as a model.
- Hardlinked BusyBox tools have zero-size archive records: resolve their shared payload, preserve their frames unchanged.
- Preexisting owners/dependencies, bad payloads, kernel blacklist, failed insertions, wrong loaded identity/binding/IRQ must reject without cleanup/retry.
- Failed hook must never continue to udev/root; future probe hangs may require physical reset and are not bounded by shell tests.
- A copied `savedefault` would defeat fallback: remove it, preserve saved stock configuration and kernel arguments.

## Tasks

- [x] Verify hashes/ELF/versions/dependency closure, tool hardlinks and existing boot inputs; preserve starting state.
- [x] Write failing actual-hook and archive-construction tests. Implement fixed hook with explicit guards, ordered insertions and post-probe verification.
- [x] Implement deterministic newc record splicing/gzip construction and non-saving entry derivation; test malformed/duplicate/corrupted inputs.
- [x] Build twice, independently inspect contents with GNU cpio; compare every stock record and hardlink/metadata, prove the four-path delta.
- [x] Recheck actual imports/config provenance, complete regression/UBSan/generator/dry runs, preservation and evidence links.
- [x] Review the final artifact and update current checkpoint pointers; keep live boot/display/SSH/recovery UNKNOWN and Gate B BLOCKED.

## Selected loading mechanism

The captured stock image includes BusyBox `insmod`, `sha256sum`, `uname`, `readlink`, `grep`, `cat`, shell and sleep applets. Fixed file insertion needs no alias/dep-index lookup. Dependencies are inserted in the order derived from captured modules.dep and actual module metadata; absent/ambiguous dependencies are refused. A private payload outside the indexed module tree prevents pulling a stock original into the image. Later modalias discovery cannot co-load a same-named Live module. This does not establish future root script behavior or runtime success.

## Stop boundary

Finish with offline artifacts and qualification. The next task must review staging/boot/recovery separately; no deployment is implied by successful construction.
