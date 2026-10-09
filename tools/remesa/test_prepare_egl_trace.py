#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Source-overlay tests; these do not create a DRM device or GPU evidence."""
import argparse
import importlib.util
import json
from pathlib import Path
import sys
import unittest

SPEC = importlib.util.spec_from_file_location("prepare_egl_trace", Path(__file__).with_name("prepare_egl_trace.py"))
trace = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(trace)
SOURCE = None


class TraceOverlay(unittest.TestCase):
    def test_all_real_source_anchors(self):
        for relative in trace.PATCHES:
            original = (SOURCE / relative).read_text()
            modified = trace.instrument(relative, original)
            self.assertIn(trace.HEADER, modified)
            self.assertEqual(modified.count("return false;"), original.count("return false;"))
            self.assertEqual(modified.count("return EGL_FALSE;"), original.count("return EGL_FALSE;"))
            self.assertEqual(modified.count("return EGL_TRUE;"), original.count("return EGL_TRUE;"))
            self.assertEqual(modified.count("return _eglError(EGL_NOT_INITIALIZED, err);"), original.count("return _eglError(EGL_NOT_INITIALIZED, err);"))

    def test_missing_anchor_rejected(self):
        relative = next(iter(trace.PATCHES))
        original = (SOURCE / relative).read_text()
        anchor = trace.PATCHES[relative][0][0]
        with self.assertRaises(ValueError):
            trace.instrument(relative, original.replace(anchor, "/* absent */"))

    def test_duplicate_anchor_rejected(self):
        relative = next(iter(trace.PATCHES))
        original = (SOURCE / relative).read_text()
        with self.assertRaises(ValueError):
            trace.instrument(relative, original + trace.PATCHES[relative][0][0])

    def test_double_instrumentation_rejected(self):
        relative = next(iter(trace.PATCHES))
        modified = trace.instrument(relative, (SOURCE / relative).read_text())
        with self.assertRaises(ValueError):
            trace.instrument(relative, modified)

    def test_foreign_source_rejected(self):
        with self.assertRaises(ValueError):
            trace.instrument("src/unknown.c", "")

    def test_record_parse_ignores_unrelated_logs(self):
        record = {"function": "dri2_create_screen", "event": "driCreateNewScreen3_nonnull", "value": 0}
        self.assertEqual(trace.trace_records("ordinary Mesa log\n" + trace.PREFIX + json.dumps(record)), [record])

    def test_invalid_record_type_rejected(self):
        for value in ("0", None, False):
            record = {"function": "f", "event": "e", "value": value}
            with self.assertRaises(ValueError):
                trace.trace_records(trace.PREFIX + json.dumps(record))

    def test_invalid_record_schema_rejected(self):
        for record in ({"function": "f", "value": 0},
                       {"function": "f", "event": "e", "value": 0, "success": True},
                       {"function": 2, "event": "e", "value": 0}):
            with self.assertRaises(ValueError):
                trace.trace_records(trace.PREFIX + json.dumps(record))


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("--mesa-source", required=True, type=Path)
    args, extra = parser.parse_known_args()
    SOURCE = args.mesa_source.resolve()
    unittest.main(argv=[sys.argv[0], *extra])
