"""Synthetic procedure-revision tests, never hardware qualification evidence."""
import copy
import ast
import importlib
import unittest

from test_frozen_first_owner_session import fixture as runtime_fixture


def staging_fixture(tool):
    records,witness=runtime_fixture()
    for row in records:row['boot_id']=witness['prior_boot_id']
    records[0]['mode']=records[2]['mode']='stock_preparation'
    root=records[1]
    root.update(classification='PASS',module_initstate='live',
                loaded_module_note_sha256=tool.ORIGINAL_NOTE)
    def file(path,size,sha):
        return dict(path=path,size=size,sha256=sha,regular=True,symlink=False,
                    uid=0,gid=0,mode='0o644',nlink=1,dev=2049,ino=123)
    root['files']={'candidate_image':file(tool.IMAGE,50805231,tool.IMAGE_SHA),
                   'custom_cfg':file('/boot/grub/custom.cfg',4132,tool.CONFIG_SHA)}
    root['grubenv']=dict(exit_code=0,stderr='',stdout='saved_entry='+tool.STOCK_ID+'\n')
    return records,dict(exit_status=0,host_duration_seconds=3),''


def menu_fixture(tool,stage):
    return dict(procedure_id=tool.PROCEDURE_ID,evidence_kind='direct_operator_report',
                reported_entry_text=tool.TITLE,candidate_build_id=tool.BUILD_ID,
                candidate_visibly_present=True,stock_visibly_present=True,
                photograph_supplied=False,staged_image_path=tool.IMAGE,
                staged_image_sha256=tool.IMAGE_SHA,staged_image_size=50805231,
                staging_boot_id=stage[1]['boot_id'],candidate_boots_before_selection=0,
                operator_report_reference='saved operator text, synthetic fixture')


class OperatorReceiptTests(unittest.TestCase):
    def setUp(self):
        self.tool=importlib.import_module('frozen_first_owner_operator_receipt')
        self.stage,self.command,self.stderr=staging_fixture(self.tool)
        self.menu=menu_fixture(self.tool,self.stage)

    def test_explicit_operator_receipt_is_prospective_and_never_photo_or_sgx(self):
        out=self.tool.validate_menu(self.menu,self.stage,self.command,self.stderr)
        self.assertFalse(out['photograph_supplied'])
        self.assertFalse(out['legacy_v2_satisfied'])
        self.assertFalse(out['sgx_authorized'])
        self.assertFalse(out['boot_authorized'])

    def test_every_observer_candidate_field_is_required_and_exact(self):
        mutations={'procedure_id':'v2','evidence_kind':'photograph','reported_entry_text':'old entry',
                   'candidate_build_id':'594030ac153ce3c6c7dec748025142c92d90dd0c',
                   'candidate_visibly_present':False,'stock_visibly_present':False,
                   'photograph_supplied':True,'staged_image_path':'old image',
                   'staged_image_sha256':'0'*64,'staged_image_size':50805273,
                   'staging_boot_id':'other','candidate_boots_before_selection':1,
                   'operator_report_reference':''}
        for key,value in mutations.items():
            for missing in [False,True]:
                receipt=copy.deepcopy(self.menu)
                if missing:del receipt[key]
                else:receipt[key]=value
                with self.subTest(key=key,missing=missing),self.assertRaises(ValueError):
                    self.tool.validate_menu(receipt,self.stage,self.command,self.stderr)

    def test_boolean_and_numeric_aliases_fail_closed(self):
        for key,value in [('photograph_supplied',0),('candidate_visibly_present',1),
                          ('candidate_boots_before_selection',False),('staged_image_size',50805231.0)]:
            receipt=copy.deepcopy(self.menu);receipt[key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):
                self.tool.validate_menu(receipt,self.stage,self.command,self.stderr)

    def test_independent_staging_image_and_configuration_mismatches_reject(self):
        for name,key,value in [('candidate_image','sha256','0'*64),('candidate_image','path','old'),
                               ('candidate_image','symlink',True),('candidate_image','mode','0o666'),
                               ('candidate_image','ino',0),('candidate_image','uid',False),
                               ('custom_cfg','sha256','0'*64),('custom_cfg','size',3042)]:
            stage=copy.deepcopy(self.stage);stage[1]['files'][name][key]=value
            with self.subTest(name=name,key=key),self.assertRaises(ValueError):
                self.tool.validate_menu(self.menu,stage,self.command,self.stderr)

    def test_stock_default_or_capture_disagreement_rejects(self):
        for key,value in [('exit_code',False),('stdout','saved_entry=experimental\n'),('stderr','unexpected')]:
            stage=copy.deepcopy(self.stage);stage[1]['grubenv'][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):
                self.tool.validate_menu(self.menu,stage,self.command,self.stderr)
        stage=copy.deepcopy(self.stage);stage[1]['guards'][0]['pass']=False
        with self.assertRaises(ValueError):self.tool.validate_menu(self.menu,stage,self.command,self.stderr)
        for command,stderr in [(dict(exit_status=False,host_duration_seconds=3),''),(self.command,None),(self.command,'oops')]:
            with self.assertRaises(ValueError):self.tool.validate_menu(self.menu,self.stage,command,stderr)

    def test_new_receipt_cannot_redeem_an_already_performed_candidate_boot(self):
        receipt=copy.deepcopy(self.menu);receipt['candidate_boots_before_selection']=1
        with self.assertRaises(ValueError):self.tool.validate_menu(receipt,self.stage,self.command,self.stderr)

    def test_successor_accepts_no_photo_while_original_validator_still_rejects(self):
        old=importlib.import_module('frozen_first_owner_session')
        records,witness=runtime_fixture();witness['selection_photo']=False
        with self.assertRaises(ValueError):old.validate(records,witness)
        original=copy.deepcopy((records,witness,self.menu,self.stage))
        out=self.tool.validate(records,witness,self.menu,self.stage,self.command,self.stderr)
        self.assertFalse(out['photograph_supplied']);self.assertFalse(out['legacy_v2_satisfied'])
        self.assertFalse(out['triangle_established']);self.assertFalse(out['sgx_authorized'])
        self.assertEqual((records,witness,self.menu,self.stage),original)

    def test_all_runtime_and_physical_guards_remain_required(self):
        for key,value in [('module_state','coming'),('loaded_note_sha256','old'),('hook_log',''),
                          ('vtcon1',0),('kernel_log','BUG: fault'),('driver_module','other')]:
            records,witness=runtime_fixture();witness['selection_photo']=False;records[1][key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):
                self.tool.validate(records,witness,self.menu,self.stage,self.command,self.stderr)

        for key,value in [('local_sudo_succeeded',False),('operator_recovery_confirmed',False),
                          ('experimental_boots',2),('sgx_actions',1),('hot_module_actions',1),
                          ('userspace_seen_before_boot_watch',False),('selection_photo',True)]:
            records,witness=runtime_fixture();witness['selection_photo']=False;witness[key]=value
            with self.subTest(key=key),self.assertRaises(ValueError):
                self.tool.validate(records,witness,self.menu,self.stage,self.command,self.stderr)

    def test_candidate_boot_cannot_reuse_verified_stock_staging_boot(self):
        records,witness=runtime_fixture();witness['selection_photo']=False
        staging_boot=self.stage[1]['boot_id']
        for row in records:row['boot_id']=staging_boot
        witness['expected_boot_id']=staging_boot
        witness['prior_boot_id']='33333333-3333-4333-8333-333333333333'
        with self.assertRaisesRegex(ValueError,'staging boot'):
            self.tool.validate(records,witness,self.menu,self.stage,self.command,self.stderr)

    def test_later_stock_prior_boot_can_differ_from_staging_boot(self):
        records,witness=runtime_fixture();witness['selection_photo']=False
        witness['prior_boot_id']='33333333-3333-4333-8333-333333333333'
        out=self.tool.validate(records,witness,self.menu,self.stage,self.command,self.stderr)
        self.assertEqual(out['classification'],
                         'CONSISTENT SUPPLIED FIRST-OWNER OPERATOR-OBSERVATION RECORDS')
        self.assertEqual(out['boot_id'],records[1]['boot_id'])
        self.assertFalse(out['sgx_authorized'])

    def test_preboot_refresh_is_read_only_reuses_image_hash_and_binds_creation_inodes(self):
        code=self.tool.preboot_source(self.stage,self.command,self.stderr)
        ast.parse(code)
        self.assertIn(self.tool.ORIGINAL_NOTE,code)
        self.assertIn('reused_staging_image_sha256',code)
        self.assertIn("'ino': 123",code)
        for forbidden in ['os.replace','os.mkdir','os.unlink','fcntl.ioctl',"open('/dev/dri",'50805231,IMAGE_SHA']:
            self.assertNotIn(forbidden,code)
        self.assertIn('small configuration hash',code)
        self.assertIn('expected new STOCK boot',code)



if __name__=='__main__':unittest.main()
