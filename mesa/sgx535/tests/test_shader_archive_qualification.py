#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Independent negative tests for actual Mesa archive qualification guards.

The original Mesa cache and archive are never modified. Each case uses a
shadow cache and must stop before compilation or execution. A Ninja dry-run
tests a genuinely dirty target with an observable command that must not run.
"""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import unittest


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


p = argparse.ArgumentParser()
p.add_argument('--cache', type=Path, required=True)
p.add_argument('--output', type=Path, required=True)
a = p.parse_args()
a.output.mkdir(parents=True, exist_ok=False)
repo = Path(__file__).resolve().parents[3]
qualifier = repo / 'mesa/sgx535/tests/qualify_shader_program.py'
archive_relative = Path('src/gallium/drivers/sgx535/libremesa_loader.a')
original_archive = a.cache / 'build-i386' / archive_relative
initial_hash = digest(original_archive)
reads = {str(qualifier), str(original_archive), str(Path(__file__).resolve())}
results = []


class ArchiveQualificationGuards(unittest.TestCase):
    def shadow(self, name, archive=True, source=True, stale=False):
        cache = a.output / name / 'cache'
        build = cache / 'build-i386'
        driver = cache / 'mesa-25.2.0/src/gallium/drivers/sgx535'
        build.mkdir(parents=True)
        driver.mkdir(parents=True)
        for relative in ['build-environment.json', 'build-i386/compile_commands.json']:
            original = a.cache / relative
            reads.add(str(original))
            target = cache / relative
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copyfile(original, target)
        if archive:
            target = build / archive_relative
            target.parent.mkdir(parents=True, exist_ok=True)
            target.symlink_to(original_archive)
        if source:
            original = repo / 'mesa/sgx535/remesa_program_registers.c'
            reads.add(str(original))
            shutil.copyfile(original, driver / original.name)
        marker = a.output / name / 'unexpected-build-marker'
        if stale:
            # Phony input is always dirty, even though the output archive
            # exists. The rule would touch the marker if executed; -n must
            # merely report it. No command writes the real archive.
            (build / 'build.ninja').write_text(
                'rule would_touch\n'
                '  command = touch ' + str(marker) + '\n'
                'build force_dirty: phony\n'
                'build ' + str(archive_relative) + ': would_touch force_dirty\n')
        return cache, marker

    def reject(self, name, cache, marker, expected):
        output = a.output / name / 'qualification'
        command = [sys.executable, str(qualifier), '--cache', str(cache),
                   '--output', str(output), '--archive-only']
        env = dict(os.environ)
        env.pop('NINJA_STATUS', None)
        completed = subprocess.run(command, capture_output=True, text=True,
                                   timeout=60, env=env)
        (a.output / name / 'stdout.log').write_text(completed.stdout)
        (a.output / name / 'stderr.log').write_text(completed.stderr)
        self.assertEqual(completed.returncode, 1, completed.stdout + completed.stderr)
        self.assertIn(expected, completed.stderr)
        self.assertFalse(marker.exists(), 'Ninja dry-run executed a command')
        self.assertEqual(digest(original_archive), initial_hash, 'Original archive changed')
        for forbidden in ['actual-archive-i386', 'actual-archive-compile.log',
                          'actual-archive-run.log', 'native', 'ubsan', 'i386', 'result.json']:
            self.assertFalse((output / forbidden).exists(), 'Preflight reached compilation/execution')
        results.append({'case': name, 'status': 'PASS', 'command': command,
                        'expected_exit': 1, 'actual_exit': completed.returncode,
                        'stop_reason': completed.stderr.strip(),
                        'archive_sha256_unchanged': initial_hash,
                        'compile_or_execution_reached': False,
                        'would_touch_marker_created': False})
        return output

    def test_missing_driver_archive(self):
        cache, marker = self.shadow('missing-driver-archive', archive=False)
        output = self.reject('missing-driver-archive', cache, marker,
                             'Actual Mesa driver archive missing')
        self.assertFalse((output / 'archive-freshness.log').exists())

    def test_staged_source_mismatch(self):
        cache, marker = self.shadow('staged-source-mismatch')
        path = cache / 'mesa-25.2.0/src/gallium/drivers/sgx535/remesa_program_registers.c'
        path.write_text(path.read_text() + '\n/* Independent negative fixture: changed staged source. */\n')
        output = self.reject('staged-source-mismatch', cache, marker,
                             'Actual Mesa archive source mismatch: remesa_program_registers.c')
        self.assertFalse((output / 'archive-freshness.log').exists())

    def test_dirty_ninja_archive_target(self):
        cache, marker = self.shadow('dirty-ninja-archive', stale=True)
        output = self.reject('dirty-ninja-archive', cache, marker,
                             'Actual Mesa archive is stale; qualify a rebuilt artifact first')
        freshness = (output / 'archive-freshness.log').read_text()
        self.assertIn('touch ' + str(marker), freshness)
        self.assertNotIn('no work to do', freshness)


suite = unittest.defaultTestLoader.loadTestsFromTestCase(ArchiveQualificationGuards)
outcome = unittest.TextTestRunner(verbosity=2).run(suite)
(a.output / 'access.json').write_text(json.dumps({
    'scope': 'Explicit original source/metadata reads and archive hashes, not syscall audit',
    'files': sorted(reads), 'original_artifacts_modified': False,
    'proprietary_shader_material_accessed': False, 'hardware_contact': False}, indent=2) + '\n')
(a.output / 'result.json').write_text(json.dumps({
    'status': 'PASS' if outcome.wasSuccessful() else 'FAIL',
    'python_cases': outcome.testsRun, 'negative_processes': len(results),
    'failures': len(outcome.failures), 'errors': len(outcome.errors), 'cases': results,
    'test_source_sha256': digest(Path(__file__)),
    'qualifier_sha256': digest(qualifier),
    'original_archive_sha256_before': initial_hash,
    'original_archive_sha256_after': digest(original_archive),
    'sgx_submissions': 0}, indent=2) + '\n')
raise SystemExit(0 if outcome.wasSuccessful() else 1)
