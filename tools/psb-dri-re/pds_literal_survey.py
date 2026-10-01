#!/usr/bin/env python3
"""List PDS-shaped immediate words in psb_dri.so disassembly, without loading it.

This is a search aid, not a PDS decoder. It never executes the target ELF.
"""

import argparse
import csv
import pathlib
import re
import subprocess


def functions(path):
    with path.open(newline="") as stream:
        rows = csv.DictReader(stream)
        return sorted((int(row["elf_va"], 16), row["name"]) for row in rows)


def owner(address, entries):
    from bisect import bisect_right

    pos = bisect_right([item[0] for item in entries], address) - 1
    return entries[pos] if pos >= 0 else (None, "UNKNOWN")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=pathlib.Path)
    parser.add_argument("functions_csv", type=pathlib.Path)
    args = parser.parse_args()
    entries = functions(args.functions_csv)
    disassembly = subprocess.run(
        ["llvm-objdump", "-d", "--x86-asm-syntax=intel", str(args.elf)],
        check=True,
        capture_output=True,
        text=True,
    ).stdout
    writer = csv.writer(__import__("sys").stdout)
    writer.writerow(["instruction_va", "function_va", "function_name", "immediate", "assembly"])
    for line in disassembly.splitlines():
        match = re.match(r"\s*([0-9a-f]+):.*?\s(?:mov|or|and|add)\s+.*?0x([0-9a-f]+)\s*$", line)
        if not match:
            continue
        address, immediate = (int(item, 16) for item in match.groups())
        if not (0x07000000 <= immediate < 0x08000000 or immediate == 0xAF000000):
            continue
        function_va, function_name = owner(address, entries)
        writer.writerow(
            [f"0x{address:08x}", f"0x{function_va:08x}" if function_va is not None else "", function_name,
             f"0x{immediate:08x}", line.strip()]
        )


if __name__ == "__main__":
    main()
