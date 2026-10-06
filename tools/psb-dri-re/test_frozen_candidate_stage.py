"""Offline staging preparation tests; synthetic records are not hardware evidence."""
import ast
import hashlib
import io
import importlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest
from unittest.mock import patch

HERE = Path(__file__).resolve().parent


class StageTests(unittest.TestCase):
    def setUp(self):
        self.tool = importlib.import_module('frozen_candidate_stage')
        self.bundle = self.tool.prepare()

    def test_unique_entry_preserves_exact_historical_prefix(self):
        old = self.tool.prior_config()
        entry = self.bundle['diagnostic-entry.proposed']
        merged = self.bundle['custom.cfg.proposed']
        self.assertEqual(merged, old + entry)
        self.assertEqual(merged.count(b'menuentry '), 4)
        self.assertEqual(entry.count(self.tool.IMAGE.encode()), 1)
        self.assertEqual(entry.count(self.tool.ENTRY_ID.encode()), 1)
        self.assertNotEqual(self.tool.IMAGE, self.tool.OLD_IMAGE)
        for forbidden in [b'savedefault', b'save_env']:
            self.assertNotIn(forbidden, merged)

    def test_all_programs_compile_and_contain_no_gpu_or_recovery_commands(self):
        for name, data in self.bundle.items():
            if name.endswith('.py'):
                with self.subTest(name=name):
                    ast.parse(data.decode())
                    for forbidden in ["['modprobe'", "['insmod'", "['rmmod'", "['reboot'", 'fcntl.ioctl', "open('/dev/dri/"]:
                        self.assertNotIn(forbidden, data.decode())

    def test_stage_defaults_to_passive_and_requires_fresh_stock_binding(self):
        source = self.bundle['stage-root.py'].decode()
        self.assertLess(source.index('if not args.apply'), source.index('create_file(IMG,ib)'))
        self.assertIn('--stock-boot', source)
        self.assertIn("fresh['boot_id'] != args.stock_boot", source)
        self.assertIn("fresh['classification'] != 'PASS'", source)
        self.assertNotIn("ID='d78d349e", source)
        # Parse-only failure must occur before even a passive target read.
        cp = subprocess.run([sys.executable, '-I', '-B', '-S', '-c', source,
                             '--apply'], capture_output=True, text=True)
        self.assertNotEqual(cp.returncode, 0)
        self.assertIn('fresh STOCK UUID', cp.stderr)

    def test_default_bundle_cli_makes_no_directory_or_connection(self):
        with tempfile.TemporaryDirectory() as directory:
            cp = subprocess.run([sys.executable, '-B', str(HERE/'frozen_candidate_stage.py')],
                                cwd=directory, capture_output=True, text=True)
            self.assertEqual(cp.returncode, 0, cp.stderr)
            self.assertEqual(list(Path(directory).iterdir()), [])
            self.assertFalse(json.loads(cp.stdout)['sgx_invocation_available'])

    def test_preflight_refuses_new_path_collisions_before_writes(self):
        source = self.bundle['stock-preflight-root.py'].decode()
        for path in [self.tool.IMAGE, self.tool.BACKUP, self.tool.PENDING, self.tool.INCOMING]:
            self.assertIn(repr(path), source)
        self.assertIn('new path absent', source)
        self.assertNotIn('os.replace(', source)

    def test_staging_pins_every_historical_image_and_both_old_backups(self):
        source = self.bundle['stage-root.py'].decode()
        for literal in [self.tool.OLD_IMAGE, 'custom.cfg.pre-diagnostic-01',
                        'custom.cfg.cycle06-preserved', 'sgx535-firstload-corrected-01']:
            self.assertIn(literal, source)
        self.assertIn(self.tool.IMAGE_SHA, source)
        self.assertIn('merged.count(b\'menuentry \')==4', source)

    def functions(self):
        tree = ast.parse(self.bundle['stage-root.py'].decode())
        selected = [n for n in tree.body if isinstance(n, (ast.Import, ast.ImportFrom, ast.FunctionDef))]
        ns = {}; exec(compile(ast.Module(body=selected, type_ignores=[]), 'stage-functions', 'exec'), ns)
        ns['result'] = {'operations': []}
        return ns

    def test_zero_write_stops_and_retains_partial_inode_receipt(self):
        ns = self.functions()
        with tempfile.TemporaryDirectory() as directory:
            dest = str(Path(directory)/'partial')
            with patch.object(ns['os'], 'write', return_value=0):
                with self.assertRaisesRegex(RuntimeError, 'zero/invalid write'):
                    ns['create_file'](dest, b'abc')
            row = ns['result']['operations'][0]
            self.assertEqual(row['phase'], 'created')
            self.assertEqual(Path(dest).stat().st_ino, row['ino'])
            self.assertEqual(Path(dest).read_bytes(), b'')
            with self.assertRaises(FileExistsError): ns['create_file'](dest, b'abc')

    def test_failed_preflight_or_boot_mismatch_cannot_enter_mutating_body(self):
        boot='11111111-1111-4111-8111-111111111111'
        for status, observed in [('BLOCKED', boot), ('PASS', '22222222-2222-4222-8222-222222222222')]:
            preflight='result='+repr(dict(classification=status,boot_id=observed))
            source=self.tool.staging(b'entry',b'merged',preflight)
            prefix=source[:source.index('import os,stat,hashlib')]
            ns={}
            with patch.object(sys, 'argv', ['stage','--apply','--stock-boot',boot]), patch('sys.stdout',new=io.StringIO()):
                with self.assertRaises(SystemExit) as caught:exec(prefix+'\nmutating_body_entered=True\n',ns)
            self.assertEqual(caught.exception.code,1)
            self.assertNotIn('mutating_body_entered',ns)

    def test_nofollow_reader_rejects_fifo_without_waiting_and_closes_fd(self):
        ns=self.functions()
        with tempfile.TemporaryDirectory() as directory:
            fifo=Path(directory)/'fifo';ns['os'].mkfifo(fifo)
            self.assertIn('os.O_NONBLOCK', self.bundle['stage-root.py'].decode())
            with self.assertRaisesRegex(RuntimeError,'regular single-link'):
                ns['read_nofollow'](str(fifo))

    def test_completed_rename_receipt_survives_sync_failure(self):
        ns=self.functions()
        with tempfile.TemporaryDirectory() as directory:
            pending=Path(directory)/'pending';config=Path(directory)/'custom.cfg'
            pending.write_bytes(b'new');config.write_bytes(b'old');inode=pending.stat().st_ino
            with patch.dict(ns, parent_sync=lambda path: (_ for _ in ()).throw(OSError('sync failed'))):
                with self.assertRaisesRegex(OSError,'sync failed'):
                    ns['publish_config'](str(pending),str(config))
            self.assertEqual(config.read_bytes(),b'new')
            self.assertEqual(ns['result']['publication']['phase'],'renamed')
            self.assertEqual(ns['result']['publication']['ino'],inode)
            self.assertEqual(ns['result']['publication']['destination'],str(config))

    def test_noncanonical_prior_boot_binding_rejects_before_passive_reads(self):
        for name in ['first-owner-root.py','stock-recovery-root.py']:
            cp=subprocess.run([sys.executable,'-I','-B','-S','-c',self.bundle[name].decode(),
                               '--prior-boot','22222222222242228222222222222222'],capture_output=True,text=True)
            self.assertNotEqual(cp.returncode,0)
            self.assertIn('canonical',cp.stderr)

    def test_successful_exclusive_publication_and_no_overwrite(self):
        ns = self.functions()
        with tempfile.TemporaryDirectory() as directory:
            dest = str(Path(directory)/'published')
            # Exercise file IO as this user; target ownership is separately guarded.
            with patch.object(ns['os'], 'fchown'):
                if ns['os'].geteuid() == 0:
                    ns['create_file'](dest, b'abc')
                else:
                    with self.assertRaisesRegex(RuntimeError, 'metadata'):
                        ns['create_file'](dest, b'abc')
            self.assertEqual(Path(dest).read_bytes(), b'abc')
            self.assertEqual(ns['result']['operations'][0]['sha256'], hashlib.sha256(b'abc').hexdigest())
            with self.assertRaises(FileExistsError): ns['create_file'](dest, b'def')
            self.assertEqual(Path(dest).read_bytes(), b'abc')

    def test_first_owner_captures_new_note_and_both_images_without_incoming_dependency(self):
        source = self.bundle['first-owner-root.py'].decode()
        self.assertIn(self.tool.NOTE_SHA, source)
        self.assertIn(self.tool.IMAGE_SHA, source)
        self.assertIn(self.tool.OLD_IMAGE, source)
        self.assertNotIn('os.listdir(incoming)', source)
        self.assertIn("cfg_new.count(b'menuentry ')==4", source)

    def test_incoming_default_has_no_target_reads_or_writes(self):
        source=self.bundle['incoming-user.py'].decode()
        cp=subprocess.run([sys.executable,'-I','-B','-S','-c',source],capture_output=True,text=True)
        self.assertEqual(cp.returncode,0,cp.stderr)
        self.assertEqual(json.loads(cp.stdout)['mode'],'PREPARATION ONLY')
        self.assertLess(source.index('if not args.prepare'),source.index('os.mkdir('))
        cp=subprocess.run([sys.executable,'-I','-B','-S','-c',source,'--prepare'],capture_output=True,text=True)
        self.assertNotEqual(cp.returncode,0)
        self.assertIn('canonical fresh STOCK UUID',cp.stderr)

    def test_local_bundle_is_exclusive_and_capture_sources_are_bound(self):
        with tempfile.TemporaryDirectory() as directory:
            dest=Path(directory)/'bundle'
            argv=[sys.executable,'-B',str(HERE/'frozen_candidate_stage.py'),'--output',str(dest)]
            cp=subprocess.run(argv,capture_output=True,text=True)
            self.assertEqual(cp.returncode,0,cp.stderr)
            saved=(dest/'custom.cfg.proposed').read_bytes()
            cp=subprocess.run(argv,capture_output=True,text=True)
            self.assertNotEqual(cp.returncode,0)
            self.assertEqual((dest/'custom.cfg.proposed').read_bytes(),saved)
        for phase in ['stock-preflight','poststage','first-owner','stock-recovery']:
            argv=[sys.executable,'-B',str(HERE/'frozen_candidate_stage.py'),'--capture-source',phase]
            if phase!='stock-preflight':argv+=['--boot','11111111-1111-4111-8111-111111111111']
            cp=subprocess.run(argv,capture_output=True,text=True)
            self.assertEqual(cp.returncode,0,cp.stderr)
            compile(cp.stdout,'passive-wrapper','exec')
            self.assertIn('timeout=budget',cp.stdout)


if __name__ == '__main__': unittest.main()
