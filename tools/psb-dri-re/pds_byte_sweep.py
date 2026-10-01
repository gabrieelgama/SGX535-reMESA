#!/usr/bin/env python3
"""Find selected little-endian PDS words in retained ELF bytes without loading them."""

import argparse
from pathlib import Path


WORDS = (0x07000345, 0x07042345, 0xAF000000)


def offsets(data, needle):
    start = 0
    while True:
        hit = data.find(needle, start)
        if hit < 0:
            return
        yield hit
        start = hit + 1


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("package_root", type=Path)
    args = parser.parse_args()
    for path in sorted(args.package_root.rglob("*.so")):
        data = path.read_bytes()
        for word in WORDS:
            hits = list(offsets(data, word.to_bytes(4, "little")))
            if hits:
                print(path.relative_to(args.package_root), f"0x{word:08x}",
                      ",".join(f"0x{hit:x}" for hit in hits))


if __name__ == "__main__":
    main()
