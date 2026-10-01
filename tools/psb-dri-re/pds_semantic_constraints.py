#!/usr/bin/env python3
"""Reclassify the retained PDS corpus and test CPU-side bit hypotheses.

This reads only the existing curated CSV. It neither loads historical ELFs nor
decodes GPU behavior. Annotations are tied to the ELF entry addresses named in
the accompanying report; UNKNOWN is deliberate.
"""

import csv
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
DOC = ROOT / "docs/phase7/psb-dri-re"
UNKNOWN = "UNKNOWN"

# CPU writes are dword indices relative to each builder's program start.
# Optional writes are separated because mutually exclusive paths share emitters.
BUILDERS = {
    "0x00025970": (12, "0,1,8", "2,3,9,10 if descriptor present", "0", "fragment primary"),
    "0x00039ac4": (12, "0,1,8", "", "0", "clear fragment"),
    "0x00039c5c": (12, "0,1,2,3,8,9,10", "", "0,10", "texture-replace fragment"),
    "0x000287ff": (12, "0,1,2,3,10", "8,9 if two-part result", "0,2,8", "fragment secondary"),
    "0x00029385": (12, "0,1,2,3,10", "8,9 if two-part result", "0,2,8", "fragment secondary"),
    "0x00039ea0": (16, "0,1,2,3,10", "8,9 if two-part result", "0,2,8", "fragment secondary"),
    "0x0003faa3": (0, "", "", "", "standalone secondary"),
    "0x00040355": (12, "0,1,4,5,8,9", "2,3 if two-part result", "0,4", "vertex primary"),
    "0x0004252d": (12, "0,1,4,5,8,9", "2,3 if two-part result", "0,4", "vertex primary variant"),
    "0x000392ab": (16, "0,1,2,3,4,5,8,9,10,11,12,13,14", "", "0,2,4", "mixed event block"),
    "0x000082c0": (12, "0,1,8", "texture slots if param_6 > 0", "0", "Xpsb pixel shader; tail is separate"),
    "0x00007eb0": (16, "0,1,2,3,8,9,10,11,12", "", "0,2", "Xpsb pixel event"),
}

FORMULAS = {
    "0x070400a2|fields": "0x070400a2 | ((DS1_index>>1)<<13) | (((DS1_index&1)+2)*0x800)",
    "0x07000064|fields": "0x07000064 | ((DS0_index>>1)<<18) | ((DS1_index>>1)<<13) | ((DS0_index&1)<<11) | (((DS0_index+1)&1)<<9) | (((DS1_index&1)+2)*0x80)",
}

KNOWN_DATA = {
    "0x00025970": "+00=linked USE relocation;+04=0 selected;+20=0x20",
    "0x00039ac4": "+00=USE relocation;+04=0;+20=0x20",
    "0x00039c5c": "+00=USE relocation;+04=0;+08=0x1e0092;+0c=target-dependent;+20=0x20;+24=0xf800;+28=relocation",
    "0x000287ff": "+00=relocation;+04=dynamic;+08=USE relocation;+0c=0;+28=0;optional +20=relocation/+24=dynamic",
    "0x00029385": "+00=relocation;+04=dynamic;+08=USE relocation;+0c=0;+28=0;optional +20=relocation/+24=dynamic",
    "0x00039ea0": "+00=relocation;+04=dynamic;+08=USE relocation;+0c=dynamic;+28=0;optional +20=relocation/+24=dynamic",
    "0x000392ab": "+00=relocation;+04=0;+08=relocation;+0c=0x80000003;+10=relocation;+14=0;+20=8;+24=0;+28=2;+2c=0x10000000",
    "0x000082c0": "+00=USE relocation;+04=metadata-derived;+20=0x20 in zero-texture primary",
    "0x00007eb0": "+00=relocation;+04=0x80000003;+08=USE relocation;+0c=0;+20=2;+24=0x10000000",
}

def emit(name, fields, rows):
    with (DOC / name).open("w", newline="") as f:
        w = csv.DictWriter(f, fieldnames=fields)
        w.writeheader()
        w.writerows(rows)

def bits(x):
    return ",".join(str(i) for i in range(32) if x & (1 << i)) or "none"

def a2(ds1):
    return 0x070400a2 | ((ds1 >> 1) << 13) | (((ds1 & 1) + 2) * 0x800)

def op64(ds0, ds1):
    return (0x07000064 | ((ds0 >> 1) << 18) | ((ds1 >> 1) << 13)
            | ((ds0 & 1) << 11) | (((ds0 + 1) & 1) << 9)
            | (((ds1 & 1) + 2) * 0x80))

def candidate45(ds0_pair, ds1_pair, residual):
    # A FITTING CPU-word decomposition, not a GPU decoder/read-set assertion.
    return 0x07000345 | (ds0_pair << 18) | (ds1_pair << 13) | (residual << 16)

def main():
    with (DOC / "pds-instruction-corpus.csv").open(newline="") as f:
        source = list(csv.DictReader(f))
    assert len(source) == 42

    semantic = []
    for i, row in enumerate(source, 1):
        extent, written, optional, reloc, role = BUILDERS[row["emitter_va"]]
        word = row["word"]
        terminal = word == "0xaf000000"
        separate_tail = row["program"].startswith("pixel_shader_tail_")
        written_indices = [int(x) for x in written.split(",") if x] if not separate_tail else []
        semantic.append({
            "row": i, "elf": row["elf"], "emitter_va": row["emitter_va"],
            "store_va": row["store_va"] or UNKNOWN, "program": row["program"],
            "program_role": role, "code_offset": row["code_offset"],
            "emission_context": row["context"],
            "word": word, "construction": row["construction"],
            "exact_cpu_formula": FORMULAS.get(word, "literal" if row["construction"] == "literal" else UNKNOWN),
            "data_prefix_dwords": UNKNOWN if separate_tail else extent,
            "producer_written_dwords": UNKNOWN if separate_tail else (written if extent else "none"),
            "ds0_written_indices": UNKNOWN if separate_tail else
                (",".join(str(x) for x in written_indices if x < 8) or "none"),
            "ds1_written_indices": UNKNOWN if separate_tail else
                (",".join(str(x-8) for x in written_indices if x >= 8) or "none"),
            "conditional_written_dwords": UNKNOWN if separate_tail else (optional or "none"),
            "relocation_dwords": UNKNOWN if separate_tail else (reloc or "none"),
            "known_cpu_data_values": UNKNOWN if separate_tail else KNOWN_DATA.get(row["emitter_va"], UNKNOWN),
            "temporary_store_initialization": UNKNOWN,
            "cpu_input_affecting_word": "none; literal" if row["construction"] == "literal" else
                "DS index from local allocation counters; inspect emitter for branch bounds",
            "sequence_role": "CPU-final in its subprogram" if terminal else UNKNOWN,
            "gpu_operand_count": UNKNOWN, "gpu_read_set": UNKNOWN,
            "source_evidence": row["evidence"] + "; decompile /tmp/sgx535-" +
                ("xpsb" if row["elf"] == "Xpsb.so" else "psb") + "-decompile/" +
                row["emitter_va"][2:] + ".txt; original corpus row " + str(i),
            "cpu_evidence_class": row["code_class"],
        })
    sf = list(semantic[0])
    emit("pds-semantic-corpus.csv", sf, semantic)

    # Held-out tests explicitly do not equate a matching CPU word with a GPU read.
    pairs = [
        ("45-primary-to-secondary", 0x07000345, 0x07042345, "DRI 0x25970/0x39ac4 to 0x287ff/0x29385", "written triplet +00/+04/+20 to +08/+0c/+28", "cross-builder, not single-input control"),
        ("45-primary-to-event", 0x07000345, 0x070b0345, "DRI 0x25970 to 0x392ab", "event block writes DS0[4,5]; residual bits 16,17 remain unassigned", "mixed event block; not controlled"),
        ("vertex-pair", 0x2f030343, 0x2f070343, "DRI 0x40355/0x4252d", "optional DS0[2,3] pair", "same builder conditional, different word class"),
        ("63-primary-to-event", 0x07000363, 0x07042363, "Xpsb 0x7eb0 to DRI 0x392ab", "event family; DS layout differs", "cross-ELF, not controlled"),
        ("45-to-e5", 0x07000345, 0x070003e5, "DRI 0x25970 to 0x392ab", "low-byte class bits differ", "different purpose; no operand isolation"),
        ("A2-formula-holdout", a2(1), 0x070418a2, "DRI 0x25970 formula to 0x39c5c literal", "DS1 index 1", "exact CPU formula check"),
        ("64-formula-holdout", op64(2,2), 0x07042364, "DRI 0x25970 formula to 0x39c5c literal", "DS0/DS1 index 2", "exact CPU formula check"),
    ]
    differential = [{"comparison": name, "left": f"0x{a:08x}", "right": f"0x{b:08x}",
                     "xor": f"0x{a^b:08x}", "changed_bits": bits(a^b), "builders": builders,
                     "cpu_side_change": change, "control_quality": quality,
                     "gpu_read_inference": UNKNOWN} for name,a,b,builders,change,quality in pairs]
    emit("pds-differential-matrix.csv", list(differential[0]), differential)

    # Formula influence is checked by varying a CPU-side index within 0..7.
    # This is not an observed architectural field width or ISA decode.
    influence = {}
    for name, fn, n in [("A2_DS1_index", lambda x:a2(x), 1),
                         ("64_DS0_index", lambda x:op64(x,0), 1),
                         ("64_DS1_index", lambda x:op64(0,x), 1)]:
        base = fn(0)
        mask = 0
        for x in range(1,8): mask |= base ^ fn(x)
        influence[name] = mask
    corpus_words = [int(r["word"],16) for r in source if r["construction"] == "literal"]
    rows = []
    for bit in range(32):
        set_count = sum(bool(w & (1<<bit)) for w in corpus_words)
        rows.append({"bit":bit, "literal_set_count":set_count,
                     "literal_count":len(corpus_words),
                     "cpu_formula_variables_0_to_7": ";".join(k for k,m in influence.items() if m & (1<<bit)) or "none",
                     "pair_45_changed": "yes" if 0x00042000 & (1<<bit) else "no",
                     "event_45_residual": "yes" if 0x00030000 & (1<<bit) else "no",
                     "architectural_field":UNKNOWN, "gpu_read_relevance":UNKNOWN})
    emit("pds-bit-influence.csv", list(rows[0]), rows)

    assert candidate45(0,0,0) == 0x07000345
    assert candidate45(1,1,0) == 0x07042345
    assert candidate45(2,0,3) == 0x070b0345
    assert a2(1) == 0x070418a2 and op64(2,2) == 0x07042364
    print("42 semantic rows, 7 differential comparisons, 32 bit-influence rows; CPU constraints only")

if __name__ == "__main__":
    main()
