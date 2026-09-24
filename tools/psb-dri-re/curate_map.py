#!/usr/bin/env python3
"""Add reviewed static roles and bounded call context to the DRI callback map.

Only exact addresses checked against the target ELF are curated. This script
reads previous read-only exports; it never opens the historical binary.
"""

import csv
from collections import defaultdict
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DOC = ROOT / "docs/phase7/psb-dri-re"


def records(name):
    with (DOC / name).open(newline="") as stream:
        return list(csv.DictReader(stream))


callbacks = {
    row["callback_elf_va"]: row["proposed_role"]
    for row in records("dri-extensions.csv")
    if row["callback_elf_va"] not in ("", "NULL")
}

# Address: (role, class, evidence, reviewed observation). Do not infer a
# hardware contract from the function name or from an embedded assertion.
CURATED = {
    0x00021e0e: ("legacy screen constructor", "DRI_GLUE", "CONFIRMED", "Allocates 0xd4-byte screen and copies 16 DriverAPI pointers from 0x002b9660"),
    0x00021c89: ("DRI2 screen constructor", "DRI_GLUE", "CONFIRMED", "Tests null InitScreen2 table slot and returns null on that path"),
    0x000213a0: ("get screen extensions", "DRI_GLUE", "CONFIRMED", "Returns screen field +0x44"),
    0x00021790: ("core swap callback", "DRI_GLUE", "CONFIRMED", "Dispatches screen DriverAPI.SwapBuffers at screen +0x1c"),
    0x0002137d: ("set swap interval", "DRI_GLUE", "CONFIRMED", "Stores interval at drawable +0x68"),
    0x0002138b: ("get swap interval", "DRI_GLUE", "CONFIRMED", "Reads interval at drawable +0x68"),
    0x0002130f: ("copy sub-buffer", "DRI_GLUE", "CONFIRMED", "Dispatches screen callback at +0x38"),
    0x00021278: ("get drawable MSC", "DRI_GLUE", "CONFIRMED", "Dispatches drawable callback at +0x3c"),
    0x00021297: ("wait for MSC", "DRI_GLUE", "CONFIRMED", "Dispatches screen callbacks at +0x2c and +0x28"),
    0x000213ab: ("frame tracking callback", "DRI_GLUE", "CONFIRMED", "Returns constant 5; functionality not inferred"),
    0x00021396: ("copy context callback", "DRI_GLUE", "CONFIRMED", "Returns constant zero; functionality not inferred"),
    0x000214e9: ("legacy context constructor", "DRI_GLUE", "CONFIRMED", "Allocates 0x18-byte wrapper and dispatches screen callback"),
    0x000215aa: ("DRI2 context constructor wrapper", "DRI_GLUE", "CONFIRMED", "Calls legacy context constructor"),
    0x000215e2: ("legacy drawable constructor", "DRI_GLUE", "CONFIRMED", "Allocates 0x6c-byte wrapper and dispatches screen callback"),
    0x000216e6: ("DRI2 drawable constructor wrapper", "DRI_GLUE", "CONFIRMED", "Calls legacy drawable constructor and allocates two 8-byte fields"),
    0x00021f8d: ("destroy screen", "DRI_GLUE", "CONFIRMED", "Calls destructor and drmUnmap/drmCloseOnce conditionally"),
    0x0004be92: ("driver screen init", "POULSBO_USERSPACE", "CONFIRMED", "Checks PsbDRIRec size 0x20 and installs five-entry screen extension list"),
    0x000477b3: ("driver swap callback", "POULSBO_USERSPACE", "CONFIRMED", "Notifies Mesa then calls 0x0003aa2a copy path"),
    0x0003aa2a: ("historical swap copy", "POULSBO_USERSPACE", "CONFIRMED", "psb_swapbuffers.c assertion and branch to 2D blit builder"),
    0x000464e0: ("2D copy command builder", "POULSBO_USERSPACE", "CONFIRMED", "intel_blit.c assertion; emits words and calls 0x00037b51"),
    0x000374f5: ("direct scene-unref request", "DRM_PSB", "CONFIRMED", "Calls drmCommandWriteRead with index 3 and size 0x14"),
    0x00037b51: ("direct command submit wrapper", "DRM_PSB", "CONFIRMED", "Builds 0x90-byte stack argument and calls drmCommandWrite index 0"),
    0x00037854: ("offset relocation builder", "DRM_PSB", "CONFIRMED", "psbSetOffsetRelocation assertion; fills relocation words"),
    0x00038762: ("output buffer allocator", "DRM_PSB", "CONFIRMED", "psb_outbuf.c assertions; reserves mapped CPU space and BO offset"),
    0x000384ab: ("output buffer release", "DRM_PSB", "CONFIRMED", "psb_outbuf.c assertions; closes allocation and checks relocation capacity"),
    0x0004297b: ("BO pool creator", "DRM_PSB", "CONFIRMED", "Calls drmBOCreate/Map/Unmap and builds per-buffer records"),
    0x00044097: ("fence wait wrapper", "DRM_PSB", "CONFIRMED", "Locks mutex and calls drmFenceWait"),
    0x0003b606: ("PDS input state emission", "SGX_SPECIFIC", "CONFIRMED", "psb_ta.c assertion; emits state words for PDS data"),
    0x0003b73b: ("PDS TA state emission", "SGX_SPECIFIC", "CONFIRMED", "psb_ta.c assertion; emits state words for PDS data"),
    0x0003b890: ("TA index-list builder", "SGX_SPECIFIC", "CONFIRMED", "psb_ta_index_list assertion; reserves 0x14 bytes and records relocations"),
    0x0002673d: ("scene submit caller", "POULSBO_USERSPACE", "CONFIRMED", "Calls 0x0002a39a with context scene pointer"),
    0x0002a39a: ("scene finalization and submit", "POULSBO_USERSPACE", "CONFIRMED", "Emits TA state and calls 0x00037b51 with engine-selection argument zero"),
    0x00026896: ("indexed primitive draw", "POULSBO_USERSPACE", "CONFIRMED", "pbr_draw_indexed_prim assertion; calls scene draw"),
    0x00027fa0: ("scene draw-elements", "POULSBO_USERSPACE", "CONFIRMED", "psb_scene_draw_elements assertion; calls TA index-list builder"),
    0x00029fbd: ("scene state setup", "POULSBO_USERSPACE", "CONFIRMED", "Calls vertex PDS upload and TA index-list builder"),
    0x0003b125: ("swap command-word builder", "POULSBO_USERSPACE", "CONFIRMED", "psb_emit_cmd_buffer assertion; reserves output and attaches relocations"),
    0x00046924: ("alternate swap copy path", "POULSBO_USERSPACE", "CONFIRMED", "Calls 0x0003b125 and direct submit wrapper"),
    0x00030ffd: ("PDS-related program buffer setup", "SGX_SPECIFIC", "CONFIRMED", "Reserves variable-length 8-byte-word buffer and zeros it"),
    0x00030fb7: ("PDS-related program finalization", "SGX_SPECIFIC", "CONFIRMED", "Sets end bit on last word and closes output buffer"),
    0x00040355: ("vertex PDS upload path", "SGX_SPECIFIC", "CONFIRMED", "psb_vs.c assertion; constructs program/output block and relocations"),
    0x0004252d: ("vertex PDS hardware upload path", "SGX_SPECIFIC", "CONFIRMED", "psb_vs.c assertion; constructs program/output block"),
    0x0003503d: ("fragment USC compile wrapper", "SGX_SPECIFIC", "CONFIRMED", "psbCompileProgusc assertion; calls compiler path and records result"),
    0x00034ce4: ("vertex USC compile wrapper", "SGX_SPECIFIC", "CONFIRMED", "psbCompileVPProgusc assertion; calls compiler path and records result"),
    0x001fb7af: ("USSE code-generation path", "SGX_SPECIFIC", "CONFIRMED", "usc/hw.c strings; allocates output and invokes generation helpers"),
}

calls_in = defaultdict(set)
calls_out = defaultdict(set)
for edge in records("analysis/callgraph.csv"):
    a, b = edge["caller_elf_va"], edge["callee_elf_va"]
    calls_out[a].add(b)
    calls_in[b].add(a)

imports = defaultdict(set)
for row in records("analysis/import-callers.csv"):
    imports[row["caller_elf_va"]].add(row["import_name"])

string_values = {row["elf_va"]: row["ascii"] for row in records("analysis/interesting-strings.csv")}
strings = defaultdict(set)
for row in records("analysis/string-xrefs.csv"):
    value = string_values.get(row["string_elf_va"])
    if value:
        strings[row["function_elf_va"]].add(value)

all_addrs = set(callbacks) | {f"0x{k:08x}" for k in CURATED}
headers = ["elf_va", "ghidra_va", "original_name", "proposed_roles", "classification", "evidence_class", "reason", "callers", "callees", "important_strings", "important_imports"]
with (DOC / "function-map.csv").open("w", newline="") as stream:
    writer = csv.DictWriter(stream, fieldnames=headers, lineterminator="\n")
    writer.writeheader()
    for addr in sorted(all_addrs):
        number = int(addr, 16)
        curated = CURATED.get(number)
        role = callbacks.get(addr, curated[0] if curated else "")
        if curated and addr in callbacks:
            reason = f"Mesa 7.4.4 descriptor field; {curated[3]}"
        elif curated:
            reason = curated[3]
        else:
            reason = "Non-null pointer in Mesa 7.4.4 DRI descriptor field; body decompiled but not semantically resolved here"
        writer.writerow({
            "elf_va": addr,
            "ghidra_va": f"0x{number+0x10000:08x}",
            "original_name": f"FUN_{number+0x10000:08x}",
            "proposed_roles": role,
            "classification": curated[1] if curated else "DRI_GLUE",
            "evidence_class": curated[2] if curated else "CONFIRMED",
            "reason": reason,
            "callers": ";".join(sorted(calls_in[addr])[:12]),
            "callees": ";".join(sorted(calls_out[addr])[:12]),
            "important_strings": ";".join(sorted(strings[addr])[:8]),
            "important_imports": ";".join(sorted(imports[addr])),
        })
