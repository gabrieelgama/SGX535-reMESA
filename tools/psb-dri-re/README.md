# Reproduce the static `psb_dri.so` extraction

These tools parse the **exact** retained file named in [the evidence report](../../docs/phase7/psb-dri-re/README.md). The Python parsers require its SHA-256 to match `74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8`; they never load the ELF as code. No script contacts DRM or hardware.

From the repository root, run the parsers in order:

```sh
python3 tools/psb-dri-re/elf_static.py
python3 tools/psb-dri-re/dri_map.py
```

For Ghidra exports, create a **separate disposable** project outside the repository and import the retained ELF. Run `ExportPsb.java` through `analyzeHeadless` on that project after analysis, with `-max-cpu 2` on a constrained host. This produces bounded CSV inventories under `docs/phase7/psb-dri-re/analysis/`; `DecompileSelected.java` uses [targets.txt](targets.txt) and writes individual function bodies to `/tmp/sgx535-psb-decompile/`. It requires exact function-entry addresses and reports a missing entry rather than silently decompiling a containing function. The displayed Ghidra image addresses in the original project add `0x10000` to ELF VAs; both scripts derive and check that delta from `__driDriverExtensions`.

After the Ghidra export, run:

```sh
python3 tools/psb-dri-re/curate_map.py
```

This adds the small reviewed function set to `function-map.csv`. Rerunning `dri_map.py` alone replaces that file with only the 23 DRI callback rows, so run `curate_map.py` last. The curated names are research labels; they are not applied to the original GUI project. The decompiled proprietary bodies and the Ghidra database stay outside the repository.
