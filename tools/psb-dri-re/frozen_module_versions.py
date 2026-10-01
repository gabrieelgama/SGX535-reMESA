"""Fail closed when a candidate module or build table lacks target CRC evidence.

The retained installed module proves CRCs only for symbols it imports.
Candidate-only imports remain unverified until a qualified target table exists.
The --check-symvers mode tests the build table against those reference imports
before compilation; it does not establish the table's complete provenance.
"""

import hashlib
import json
from pathlib import Path
import re
import struct
import sys


ELF32 = struct.Struct("<16sHHIIIIIHHHHHH")
SECTION = struct.Struct("<IIIIIIIIII")
VERSION_SIZE = 64


def read_versions(path):
    data = path.read_bytes()
    if len(data) < ELF32.size:
        raise ValueError("ELF header truncated")
    fields = ELF32.unpack_from(data)
    ident, machine, offset, entry_size, count, names_index = (
        fields[0], fields[2], fields[6], fields[11], fields[12], fields[13]
    )
    if (ident[:7] != b"\x7fELF\x01\x01\x01" or machine != 3 or
            entry_size != SECTION.size or count < 3 or names_index >= count or
            offset + count * entry_size > len(data)):
        raise ValueError("expected complete little-endian i386 ELF32 sections")
    sections = [SECTION.unpack_from(data, offset + i * entry_size)
                for i in range(count)]

    def payload(section):
        start, size = section[4], section[5]
        if start + size > len(data):
            raise ValueError("ELF section outside file")
        return data[start:start + size]

    strings = payload(sections[names_index])
    versions = None
    for section in sections:
        name_offset = section[0]
        if name_offset >= len(strings):
            raise ValueError("ELF section name outside string table")
        name = strings[name_offset:].split(b"\0", 1)[0]
        if name == b"__versions":
            if versions is not None:
                raise ValueError("duplicate __versions section")
            versions = payload(section)
    if versions is None or len(versions) % VERSION_SIZE:
        raise ValueError("missing or malformed __versions section")

    result = {}
    for start in range(0, len(versions), VERSION_SIZE):
        record = versions[start:start + VERSION_SIZE]
        name_bytes = record[4:].split(b"\0", 1)[0]
        if not name_bytes:
            raise ValueError("empty versioned symbol name")
        try:
            name = name_bytes.decode("ascii")
        except UnicodeDecodeError as exc:
            raise ValueError("non-ASCII versioned symbol name") from exc
        if name in result:
            raise ValueError(f"duplicate versioned symbol: {name}")
        result[name] = struct.unpack_from("<I", record)[0]
    if "module_layout" not in result:
        raise ValueError("module_layout version absent")
    return result, hashlib.sha256(data).hexdigest()


def compare(reference, candidate):
    ref, ref_hash = read_versions(reference)
    got, got_hash = read_versions(candidate)
    shared = ref.keys() & got.keys()
    mismatches = {
        name: {"reference": f"0x{ref[name]:08x}",
               "candidate": f"0x{got[name]:08x}"}
        for name in sorted(shared) if ref[name] != got[name]
    }
    unverified = sorted(got.keys() - ref.keys())
    return {
        "classification": "REJECT" if mismatches or unverified else "PASS",
        "reference_sha256": ref_hash,
        "candidate_sha256": got_hash,
        "reference_import_count": len(ref),
        "candidate_import_count": len(got),
        "shared_count": len(shared),
        "mismatched_count": len(mismatches),
        "mismatches": mismatches,
        "unverified_candidate_imports": unverified,
        "module_layout": {
            "reference": f"0x{ref['module_layout']:08x}",
            "candidate": f"0x{got['module_layout']:08x}",
        },
    }


def read_symvers(path):
    data = path.read_bytes()
    try:
        lines = data.decode("ascii").splitlines()
    except UnicodeDecodeError as exc:
        raise ValueError("non-ASCII Module.symvers") from exc
    versions = {}
    for number, line in enumerate(lines, 1):
        fields = line.split()
        if (len(fields) not in (4, 5) or
                not re.fullmatch(r"0x[0-9a-fA-F]{8}", fields[0])):
            raise ValueError(f"malformed Module.symvers line {number}")
        name = fields[1]
        if name in versions:
            raise ValueError(f"duplicate Module.symvers symbol: {name}")
        versions[name] = int(fields[0], 16)
    if not versions:
        raise ValueError("empty Module.symvers")
    return versions, hashlib.sha256(data).hexdigest()


def compare_symvers(reference, build_table):
    ref, ref_hash = read_versions(reference)
    table, table_hash = read_symvers(build_table)
    missing = sorted(ref.keys() - table.keys())
    mismatches = {
        name: {"reference": f"0x{ref[name]:08x}",
               "build_table": f"0x{table[name]:08x}"}
        for name in sorted(ref.keys() & table.keys()) if ref[name] != table[name]
    }
    return {
        "classification": "REJECT" if missing or mismatches else "PASS",
        "scope": "REFERENCE_IMPORTS_ONLY",
        "candidate_imports_verified": False,
        "reference_sha256": ref_hash,
        "build_table_sha256": table_hash,
        "reference_import_count": len(ref),
        "build_table_symbol_count": len(table),
        "mismatched_count": len(mismatches),
        "mismatches": mismatches,
        "missing_imports": missing,
        "module_layout": {
            "reference": f"0x{ref['module_layout']:08x}",
            "build_table": (f"0x{table['module_layout']:08x}"
                            if "module_layout" in table else None),
        },
    }


def main():
    check_table = len(sys.argv) == 4 and sys.argv[1] == "--check-symvers"
    if not check_table and len(sys.argv) != 3:
        raise SystemExit(
            "usage: frozen_module_versions.py TARGET_MODULE CANDIDATE_MODULE\n"
            "   or: frozen_module_versions.py --check-symvers TARGET_MODULE BUILD_MODULE_SYMVERS"
        )
    try:
        report = (compare_symvers(Path(sys.argv[2]), Path(sys.argv[3]))
                  if check_table else
                  compare(Path(sys.argv[1]), Path(sys.argv[2])))
    except (OSError, ValueError) as exc:
        print(f"invalid module comparison: {exc}", file=sys.stderr)
        raise SystemExit(2) from exc
    print(json.dumps(report, indent=2, sort_keys=True))
    raise SystemExit(0 if report["classification"] == "PASS" else 1)


if __name__ == "__main__":
    main()
