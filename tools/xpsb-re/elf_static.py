#!/usr/bin/env python3
"""Read-only, dependency-free ELF32 inventory for the preserved Xpsb.so.

This parses bytes and writes CSV/JSON; it never loads or invokes the library.
All addresses emitted are ELF virtual addresses, not Ghidra image addresses.
"""

import csv
import hashlib
import json
from pathlib import Path
import re
import struct


ROOT = Path(__file__).resolve().parents[2]
BINARY = ROOT / "references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx/drivers/Xpsb.so"
OUT = ROOT / "docs/phase7/xpsb-re/analysis"
EXPECTED_SHA256 = "da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f"
KEYWORDS = re.compile(
    r"(?i)(?:\bpsb\b|psb_|poulsbo|gma\s*500|power\s*vr|pvr|sgx|eura|usse|\buse\b|\bpds\b|"
    r"microkernel|firmware|\bdri\b|DRI_|\bdrm\b|drm[A-Z]|ioctl|reloc|fence|shader|"
    r"\.c$|\.h$|mesa\s+[0-9]|/dev/dri|command.?buffer|outbuf)"
)


def cstring(data, offset):
    if not 0 <= offset < len(data):
        return ""
    end = data.find(b"\0", offset)
    return data[offset:end if end >= 0 else len(data)].decode("utf-8", "replace")


def csv_write(path, header, rows):
    with path.open("w", newline="") as f:
        writer = csv.writer(f, lineterminator="\n")
        writer.writerow(header)
        writer.writerows(rows)


def main():
    data = BINARY.read_bytes()
    digest = hashlib.sha256(data).hexdigest()
    if digest != EXPECTED_SHA256:
        raise SystemExit(f"Unexpected binary SHA-256: {digest}")
    if data[:6] != b"\x7fELF\x01\x01":
        raise SystemExit("Not little-endian ELF32")
    e_phoff, e_shoff = struct.unpack_from("<II", data, 28)
    e_phentsize, e_phnum, e_shentsize, e_shnum, e_shstrndx = struct.unpack_from("<HHHHH", data, 42)

    phdrs = []
    for i in range(e_phnum):
        fields = struct.unpack_from("<8I", data, e_phoff + i * e_phentsize)
        phdrs.append(dict(zip(("type", "offset", "vaddr", "paddr", "filesz", "memsz", "flags", "align"), fields)))

    def va_to_offset(va):
        for p in phdrs:
            if p["type"] == 1 and p["vaddr"] <= va < p["vaddr"] + p["filesz"]:
                return p["offset"] + va - p["vaddr"]
        return None

    raw_sections = [struct.unpack_from("<10I", data, e_shoff + i * e_shentsize) for i in range(e_shnum)]
    shstr = raw_sections[e_shstrndx]
    names = data[shstr[4]:shstr[4] + shstr[5]]
    sections = {}
    for s in raw_sections:
        name = cstring(names, s[0])
        sections[name] = {"type": s[1], "addr": s[3], "offset": s[4], "size": s[5], "entsize": s[9]}

    dynsym = sections[".dynsym"]
    dynstr = sections[".dynstr"]
    strings = data[dynstr["offset"]:dynstr["offset"] + dynstr["size"]]
    symbols = []
    for i in range(dynsym["size"] // 16):
        off = dynsym["offset"] + i * 16
        name, value, size, info, other, shndx = struct.unpack_from("<IIIBBH", data, off)
        symbols.append({"index": i, "name": cstring(strings, name), "va": value, "size": size,
                        "type": info & 15, "binding": info >> 4, "section_index": shndx})

    relocations = []
    for section_name in (".rel.dyn", ".rel.plt"):
        section = sections[section_name]
        for i in range(section["size"] // 8):
            off = section["offset"] + i * 8
            va, info = struct.unpack_from("<II", data, off)
            si, typ = info >> 8, info & 255
            binary_off = va_to_offset(va)
            addend = struct.unpack_from("<I", data, binary_off)[0] if binary_off is not None else None
            relocations.append((section_name, f"0x{va:08x}", f"0x{binary_off:x}" if binary_off is not None else "",
                                typ, si, symbols[si]["name"] if si < len(symbols) else "",
                                f"0x{addend:08x}" if addend is not None else ""))

    needed = []
    dynamic = sections[".dynamic"]
    for off in range(dynamic["offset"], dynamic["offset"] + dynamic["size"], 8):
        tag, val = struct.unpack_from("<II", data, off)
        if tag == 0:
            break
        if tag == 1:
            needed.append(cstring(strings, val))

    OUT.mkdir(parents=True, exist_ok=True)
    csv_write(OUT / "dynamic-symbols.csv", ("index", "name", "elf_va", "size", "type", "binding", "section_index"),
              ((s["index"], s["name"], f"0x{s['va']:08x}", s["size"], s["type"], s["binding"], s["section_index"]) for s in symbols))
    csv_write(OUT / "relocations.csv", ("rel_section", "target_va", "target_file_offset", "type", "symbol_index", "symbol_name", "inplace_addend"), relocations)

    found_strings = []
    for m in re.finditer(rb"[\x20-\x7e]{4,}", data):
        value = m.group().decode("ascii")
        if KEYWORDS.search(value):
            possible_va = next((p["vaddr"] + m.start() - p["offset"] for p in phdrs
                                if p["type"] == 1 and p["offset"] <= m.start() < p["offset"] + p["filesz"]), None)
            found_strings.append((f"0x{possible_va:08x}" if possible_va is not None else "",
                                  f"0x{m.start():x}", value))
    csv_write(OUT / "interesting-strings.csv", ("elf_va", "file_offset", "ascii"), found_strings)

    summary = {
        "binary": str(BINARY.relative_to(ROOT)), "sha256": digest, "bytes": len(data),
        "elf": {"class": "ELF32", "endian": "little", "machine": "Intel 80386", "type": "DYN"},
        "program_headers": phdrs, "sections": sections, "needed": needed,
        "counts": {"dynamic_symbols": len(symbols), "imports": sum(s["section_index"] == 0 and bool(s["name"]) for s in symbols),
                   "exports": sum(s["section_index"] != 0 and bool(s["name"]) for s in symbols),
                   "relocations": len(relocations), "filtered_strings": len(found_strings)},
        "address_convention": "ELF VA; Ghidra image base is separately checked; file offsets are separately recorded",
    }
    (OUT / "elf-summary.json").write_text(json.dumps(summary, indent=2) + "\n")
    print(json.dumps({"sha256": digest, "counts": summary["counts"], "needed": needed}, indent=2))


if __name__ == "__main__":
    main()
