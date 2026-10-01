#!/usr/bin/env python3
"""Check selected PDS word relationships without decoding the Series5 ISA.

Inputs are CPU-side construction rules recovered at ELF 0x25970 / Xpsb 0x82c0.
This program never opens or executes either historical ELF.  It tests only
arithmetic constraints and deliberately makes no GPU read-set assertion.
"""

import csv
import hashlib
from pathlib import Path


def a2(ds1: int) -> int:
    return ((ds1 >> 1) << 13) | 0x070400A2 | (((ds1 & 1) + 2) * 0x800)


def op64(ds0: int, ds1: int) -> int:
    return (
        ((ds0 >> 1) << 18)
        | ((ds1 >> 1) << 13)
        | 0x07000064
        | ((ds0 & 1) << 11)
        | (((ds0 + 1) & 1) << 9)
        | (((ds1 & 1) + 2) * 0x80)
    )


def candidate45(ds0: int, ds1: int) -> int:
    """Untested hypothesis: substitute 0x45 for 0x64 in its index formula."""
    return (
        ((ds0 >> 1) << 18)
        | ((ds1 >> 1) << 13)
        | 0x07000045
        | ((ds0 & 1) << 11)
        | (((ds0 + 1) & 1) << 9)
        | (((ds1 & 1) + 2) * 0x80)
    )


def main() -> None:
    root = Path(__file__).resolve().parents[2]
    corpus = root / "docs/phase7/psb-dri-re/pds-instruction-corpus.csv"
    package = root / "references/home:lkundrak:poulsbo/xpsb-glx/extracted/xpsb-glx"
    paths = {"psb_dri.so": package / "dri/psb_dri.so", "Xpsb.so": package / "drivers/Xpsb.so"}
    hashes = {
        "psb_dri.so": "74ca42991906741ee91be1bc088ae0b142fa71b6a85658c5dad0851ce50dd0d8",
        "Xpsb.so": "da531587b1ec59fe433fa20ddd2e691b2f8b7fb35bb82525d4db99259c5e571f",
    }
    blobs = {name: path.read_bytes() for name, path in paths.items()}
    for name, blob in blobs.items():
        assert hashlib.sha256(blob).hexdigest() == hashes[name], name
    with corpus.open(newline="") as fh:
        for row in csv.DictReader(fh):
            if row["construction"] != "literal" or not row["store_va"]:
                continue
            va = int(row["store_va"], 16)
            raw = int(row["word"], 16).to_bytes(4, "little")
            assert raw in blobs[row["elf"]][va : va + 16], (row["elf"], row["store_va"], row["word"])

    assert a2(1) == 0x070418A2
    assert op64(2, 2) == 0x07042364
    assert candidate45(0, 0) == 0x07000345
    assert candidate45(2, 2) == 0x07042345
    assert (0x07000345 ^ 0x07042345) == 0x00042000
    assert not any(candidate45(x, y) == 0x070B0345 for x in range(32) for y in range(32))

    out = root / "docs/phase7/psb-dri-re/pds-field-constraints.csv"
    with out.open("w", newline="") as fh:
        writer = csv.writer(fh)
        writer.writerow(["test", "inputs", "computed_word", "retained_word", "result", "classification"])
        rows = [
            ("A2", "DS1 index 1", a2(1), 0x070418A2, "CPU formula cross-check", "CONFIRMED"),
            ("64", "DS0 index 2; DS1 index 2", op64(2, 2), 0x07042364, "CPU formula cross-check", "CONFIRMED"),
            ("45 hypothesis", "indices 0;0", candidate45(0, 0), 0x07000345, "fits", "INFERRED"),
            ("45 hypothesis", "indices 2;2", candidate45(2, 2), 0x07042345, "fits", "INFERRED"),
            ("45 hypothesis", "indices 0..31", 0, 0x070B0345, "no matching pair", "REJECTED-AS-COMPLETE"),
        ]
        for label, inputs, actual, expected, result, confidence in rows:
            writer.writerow([label, inputs, f"0x{actual:08x}" if actual else "", f"0x{expected:08x}", result, confidence])


if __name__ == "__main__":
    main()
