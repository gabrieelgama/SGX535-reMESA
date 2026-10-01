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

`pds_launch_state_check.py` validates the Route C launch-state CSV and CPU
packing/layout model. Run `python3 -m unittest discover -s tools/psb-dri-re -p test_pds_launch_state_check.py`. Its default CLI validates an OPEN
inventory; `--require-closed` deliberately exits1 because architectural rule
L12 is not proved. It imports the existing CPU image checker and never loads
an ELF or accesses hardware. See the [launch report](../../docs/phase7/psb-dri-re/pds-launch-state-coverage.md).

`frozen_triangle_image.py` emits a partial JSON CPU specification (`--json`), generates only the four new triangle CSVs (`--write-tables`), and reports the dependency-only hypothetical (`--assume-l12-closed`). `--complete` must fail. Run `python3 -B -m unittest discover -s tools/psb-dri-re -p test_frozen_triangle_image.py -v`. Null bytes are unresolved, never address zero.

`frozen_triangle_image.py --bundle` joins the partial CPU image with
`frozen_triangle_bo.py` and `frozen_triangle_contracts.py`. The BO model
packs49 relocation records,136-byte validation nodes and144-byte command
parameters; supplied addresses/handles are mandatory and no device is opened.
Run `python3 -B -m unittest discover -s tools/psb-dri-re -p "test_frozen_triangle*.py"`.
All complete modes reject while coverage/publication/bootstrap remain open.

`frozen_triangle_closure.py` joins the ordered publication policy, TA ready-state countercheck, eleven auxiliary source-domain records and primary L12. `--write-tables` generates eight focused CSVs; `--json` exposes the full model; `--complete` must reject B3/B4/B1/L12. It never accesses a device. The bundle includes this ledger; successful simulated polls are not architectural evidence.

P7H-055–057 add four-load and later-wait inventories, canonical family joins and explicit rule discriminators. `check_completion_cycle` validates observations only, with documented quiescence/semantics prerequisites. No hardware contract is inferred from a passing check.
