#!/usr/bin/env python3
"""Extract the retained ELF's 17 state atoms; parse bytes, never load code."""
import csv
import hashlib
import json
from pathlib import Path
import struct

ROOT = Path(__file__).resolve().parents[2]
BINARY = ROOT / "references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/dri/psb_dri.so"
OUT = ROOT / "docs/phase7/psb-dri-re"
EXPECTED = "74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8"


def main():
    data = BINARY.read_bytes()
    if hashlib.sha256(data).hexdigest() != EXPECTED:
        raise SystemExit("Unexpected retained binary")
    segments = json.loads((OUT / "analysis/elf-summary.json").read_text())["program_headers"]

    def off(va):
        for p in segments:
            if p["type"] == 1 and p["vaddr"] <= va < p["vaddr"] + p["filesz"]:
                return p["offset"] + va - p["vaddr"]
        raise ValueError(hex(va))

    def u32(va):
        return struct.unpack_from("<I", data, off(va))[0]

    def text(va):
        start = off(va)
        return data[start:data.index(0, start)].decode("ascii", "replace")

    rows = []
    for i in range(17):
        slot = 0x2BB200 + 4 * i
        atom = u32(slot)
        name, mask0, mask1, callback = (u32(atom + 4 * j) for j in range(4))
        rows.append([i, f"0x{slot:08x}", f"0x{atom:08x}", f"0x{off(atom):08x}",
                     text(name), f"0x{mask0:08x}", f"0x{mask1:08x}",
                     f"0x{callback:08x}", "CONFIRMED", EXPECTED])
    with (OUT / "state-atoms.csv").open("w", newline="") as f:
        w = csv.writer(f, lineterminator="\n")
        w.writerow(["order", "pointer_slot_va", "atom_va", "atom_file_offset", "source_label",
                    "dirty_mask_0", "dirty_mask_1", "callback_va", "confidence", "binary_sha256"])
        w.writerows(rows)
    for row in rows:
        print(row[:8])


if __name__ == "__main__":
    main()
