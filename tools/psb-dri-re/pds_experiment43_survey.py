#!/usr/bin/env python3
"""Static sweep for new 0x45-family and 0xad/0xae/0xaf immediate stores.

This is a locator, not a PDS decoder. It reads ELF bytes and disassembles
them with llvm-objdump; it never loads or executes the historical object.
Only memory-destination immediate stores are counted as verified producers.
"""

import argparse
import csv
import hashlib
from pathlib import Path
import re
import subprocess
import sys


LINE = re.compile(r"^\s*([0-9a-f]+):.*?\s+mov\s+[^,]*\[[^]]+\],\s*0x([0-9a-f]+)\s*$")


def selected(word):
    return ((word >> 24 == 0x07 and word & 0xff == 0x45)
            or word >> 24 in (0xad, 0xae, 0xaf))


def survey(path):
    data = path.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    disassembly = subprocess.run(
        ["llvm-objdump", "-d", "--x86-asm-syntax=intel", str(path)],
        check=True, capture_output=True, text=True,
    ).stdout
    rows = []
    for line in disassembly.splitlines():
        match = LINE.match(line)
        if not match:
            continue
        va, word = (int(value, 16) for value in match.groups())
        if selected(word):
            rows.append({"elf": str(path), "sha256": digest,
                         "store_va": f"0x{va:08x}", "word": f"0x{word:08x}",
                         "classification": "CPU immediate memory store; GPU semantics UNKNOWN"})
    return rows


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("elf", type=Path, nargs="+")
    args = parser.parse_args()
    writer = csv.DictWriter(sys.stdout, fieldnames=[
        "elf", "sha256", "store_va", "word", "classification"])
    writer.writeheader()
    for path in args.elf:
        writer.writerows(survey(path))


if __name__ == "__main__":
    main()
