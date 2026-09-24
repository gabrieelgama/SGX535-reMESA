#!/usr/bin/env python3
"""Decode psb_dri.so DRI descriptor data using the Mesa 7.4.4 ABI.

The ABI field names come from MesaLib-7.4.4/include/GL/internal/dri_interface.h.
This script parses bytes only. A nonzero callback is a confirmed descriptor
pointer; behavior inside that callback requires separate analysis.
"""

import csv
import hashlib
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]
BINARY = ROOT / "references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/dri/psb_dri.so"
OUT = ROOT / "docs/phase7/psb-dri-re"
EXPECTED = "74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8"

# Ordered callback fields after the 8-byte __DRIextension base.
FIELDS = {
    "DRI_ReadDrawable": [],
    "DRI_MediaStreamCounter": ["waitForMSC", "getDrawableMSC"],
    "DRI_CopySubBuffer": ["copySubBuffer"],
    "DRI_SwapControl": ["setSwapInterval", "getSwapInterval"],
    "DRI_Legacy": ["createNewScreen", "createNewDrawable", "createNewContext"],
    "DRI_DRI2": ["createNewScreen", "createNewDrawable", "createNewContext"],
    "DRI_FrameTracking": ["frameTracking", "queryFrameTracking"],
    "DRI_Core": ["createNewScreen", "destroyScreen", "getExtensions", "getConfigAttrib",
                 "indexConfigAttrib", "createNewDrawable", "destroyDrawable", "swapBuffers",
                 "createNewContext", "copyContext", "destroyContext", "bindContext", "unbindContext"],
}


def main():
    data = BINARY.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXPECTED:
        raise SystemExit("Binary digest mismatch")
    sections = json.loads((OUT / "analysis/elf-summary.json").read_text())["sections"]
    segments = json.loads((OUT / "analysis/elf-summary.json").read_text())["program_headers"]

    def off(va):
        for s in segments:
            if s["type"] == 1 and s["vaddr"] <= va < s["vaddr"] + s["filesz"]:
                return s["offset"] + va - s["vaddr"]
        raise ValueError(f"unmapped VA 0x{va:08x}")

    def u32(va):
        return struct.unpack_from("<I", data, off(va))[0]

    def asciiz(va):
        start = off(va)
        end = data.find(b"\0", start)
        return data[start:end].decode("ascii")

    # Exported root array and a separate candidate screen-extension array.
    roots = [u32(0x2bb1bc + i * 4) for i in range(4)]
    if roots != [0x2b9620, 0x2b3280, 0x2b3294, 0]:
        raise SystemExit(f"unexpected exported root: {roots}")
    screen = [u32(0x2bb2d0 + i * 4) for i in range(6)]
    if screen != [0x2b324c, 0x2b3264, 0x2b3270, 0x2b32a8, 0x2b3254, 0]:
        raise SystemExit(f"unexpected candidate screen extension array: {screen}")

    rows = []
    functions = {}
    for graph, addresses in (("exported_root", roots[:-1]), ("candidate_screen_list", screen[:-1])):
        for va in addresses:
            name = asciiz(u32(va))
            if name not in FIELDS or u32(va + 4) != 1:
                raise SystemExit(f"unrecognized descriptor at 0x{va:08x}: {name}")
            rows.append((graph, name, f"0x{va:08x}", f"0x{off(va):x}", 1, "0x00", "name", "", "",
                         "", "CONFIRMED", "ELF pointer and Mesa 7.4.4 __DRIextensionRec name field"))
            rows.append((graph, name, f"0x{va:08x}", f"0x{off(va):x}", 1, "0x04", "version", "", "",
                         "", "CONFIRMED", "ELF value and Mesa 7.4.4 ABI version field"))
            for i, role in enumerate(FIELDS[name]):
                field_off = 8 + 4 * i
                target = u32(va + field_off)
                known = f"0x{target:08x}" if target else "NULL"
                ghidra = f"FUN_{target + 0x10000:08x}" if target else ""
                proposed = f"{name}.{role}" if target else ""
                rows.append((graph, name, f"0x{va:08x}", f"0x{off(va):x}", 1, f"0x{field_off:02x}",
                             role, known, ghidra, proposed, "CONFIRMED",
                             "32-bit descriptor field at Mesa 7.4.4 ABI offset; function behavior not inferred"))
                if target:
                    functions.setdefault(target, []).append(proposed)

    with (OUT / "dri-extensions.csv").open("w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(("graph", "extension_name", "descriptor_elf_va", "descriptor_file_offset", "abi_version",
                    "field_offset", "field_role", "callback_elf_va", "ghidra_original_name", "proposed_role",
                    "evidence_class", "evidence"))
        w.writerows(rows)
    # A string alone is not an extension descriptor. Scan data sections for
    # aligned direct pointers to *all* DRI_ names, including loader-side names.
    string_rows = list(csv.DictReader((OUT / "analysis/interesting-strings.csv").open(newline="")))
    scan = []
    for item in string_rows:
        name = item["ascii"]
        if not name.startswith("DRI_"):
            continue
        name_va = int(item["elf_va"], 16)
        matches = []
        for section_name in (".data.rel.ro", ".data", ".rodata"):
            section = sections[section_name]
            start, size = section["offset"], section["size"]
            for n in range(0, size - 3, 4):
                if struct.unpack_from("<I", data, start + n)[0] == name_va:
                    va = section["addr"] + n
                    version = struct.unpack_from("<I", data, start + n + 4)[0] if n + 8 <= size else None
                    matches.append((section_name, va, start + n, version))
        if matches:
            for section_name, va, offset, version in matches:
                scan.append((name, item["elf_va"], section_name, f"0x{va:08x}", f"0x{offset:x}",
                             version if version is not None else "", "YES" if va in roots + screen else "NO"))
        else:
            scan.append((name, item["elf_va"], "", "", "", "", "NO"))
    with (OUT / "dri-name-pointer-scan.csv").open("w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(("name", "name_string_elf_va", "pointer_section", "pointer_elf_va",
                    "pointer_file_offset", "following_u32", "in_confirmed_extension_arrays"))
        w.writerows(scan)
    with (OUT / "function-map.csv").open("w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(("elf_va", "ghidra_va", "original_name", "proposed_roles", "classification",
                    "evidence_class", "reason", "callers", "callees", "important_strings", "important_imports"))
        for va, roles in sorted(functions.items()):
            w.writerow((f"0x{va:08x}", f"0x{va + 0x10000:08x}", f"FUN_{va + 0x10000:08x}",
                        ";".join(roles), "DRI_GLUE", "CONFIRMED",
                        "Nonzero pointer in Mesa 7.4.4 DRI descriptor field; body unreviewed", "", "", "", ""))
    print(json.dumps({"exported_root": [hex(x) for x in roots],
                      "candidate_screen_list": [hex(x) for x in screen],
                      "extensions": len(FIELDS), "non_null_callback_fields": sum(1 for row in rows if row[7].startswith("0x")),
                      "unique_callback_addresses": len(functions)}, indent=2))


if __name__ == "__main__":
    main()
