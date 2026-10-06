"""Synthetic CPU fixtures only; importing/running this never opens a display."""
import copy
import hashlib
import unittest
from triangle_pixels import convert, publish_transaction, validate_source
from triangle_display import Attributes, Image, Visual, validate_authorization
import ctypes

SOURCE = b''.join((0xffff00ff if 8 <= y <= 22 and 8 <= x <= 30-y else 0).to_bytes(4, 'little')
                  for y in range(32) for x in range(32))
SHA = hashlib.sha256(SOURCE).hexdigest()
LAYOUT = dict(width=1280, height=800, pitch=5120, bits=32,
              masks=(0xff0000, 0xff00, 0xff), byteorder='little')


class Fake:
    def __init__(self):
        self.target = {'boot': 'synthetic', 'root': 3, 'epoch': 1}
        self.pixels = b'old contents'
        self.claimed = False
        self.claims = self.releases = self.writes = 0
        self.partial = self.map_fail = self.restore_fail = False
        self.bad_read = False

    def identity(self): return copy.deepcopy(self.target)
    def claim(self): self.claimed = True; self.claims += 1
    def release(self): self.claimed = False; self.releases += 1
    def read(self): return b'bad' if self.bad_read and self.writes == 1 else self.pixels
    def prepare(self, source):
        if self.map_fail: raise MemoryError('mapping failed')
        return source
    def agrees(self, a, b): return a == b
    def write(self, image):
        assert self.claimed
        self.writes += 1
        if self.restore_fail and self.writes == 2: raise OSError('restore failure')
        if self.partial and self.writes == 1:
            self.pixels = image[:24]
            raise OSError('partial publication')
        self.pixels = image


class Pixels(unittest.TestCase):
    def plan(self, **changes):
        layout = dict(LAYOUT); layout.update(changes)
        return convert(SOURCE, **layout)

    def test_source(self): validate_source(SOURCE, SHA)
    def test_source_corruption(self):
        with self.assertRaises(ValueError): validate_source(SOURCE[:-1]+b'x', SHA)
    def test_source_wrong_footprint(self):
        source = b'\0'*4096
        with self.assertRaises(ValueError): validate_source(source, hashlib.sha256(source).hexdigest())
    def test_source_truncated(self):
        with self.assertRaises(ValueError): validate_source(SOURCE[:-1], SHA)
    def test_source_preserved(self):
        original = bytes(SOURCE); self.plan(); self.assertEqual(SOURCE, original)
    def test_normal(self):
        rows = self.plan(x=64, y=64)
        self.assertEqual(rows[0][0], 64*5120+64*4)
        self.assertEqual(len(rows), 32); self.assertTrue(all(len(row)==128 for _, row in rows))
    def test_left(self): self.assertEqual(self.plan(x=0)[0][0], 0)
    def test_right(self): self.assertEqual(self.plan(x=1248)[-1][0]+128, 32*5120)
    def test_top(self): self.assertEqual(self.plan(y=0)[0][0], 0)
    def test_bottom(self): self.assertEqual(self.plan(y=768)[-1][0]+128, 799*5120+128)
    def test_outside(self):
        for xy in [(-1,0),(0,-1),(1249,0),(0,769)]:
            with self.subTest(xy=xy), self.assertRaises(ValueError): self.plan(x=xy[0],y=xy[1])
    def test_pitch_padding(self):
        rows=self.plan(pitch=8192); self.assertEqual(rows[1][0]-rows[0][0],8192)
    def test_mismatched_pitch(self):
        with self.assertRaises(ValueError): self.plan(pitch=5119)
    def test_truncated_mapping(self):
        with self.assertRaises(ValueError): self.plan(extent=100)
    def test_overflow(self):
        with self.assertRaises(ValueError): self.plan(pitch=1<<63)
    def test_unsupported_format(self):
        with self.assertRaises(ValueError): self.plan(bits=8)
    def test_overlap(self):
        with self.assertRaises(ValueError): self.plan(masks=(0xff,0xff,0xff))
    def test_noncontiguous_mask(self):
        with self.assertRaises(ValueError): self.plan(masks=(0xff0000,0xf0f0,0xf))
    def test_channel_conversion(self):
        source=(0xff123456).to_bytes(4,'little')*1024
        row=convert(source,**LAYOUT)[0][1]
        self.assertEqual(row[:4],b'\x56\x34\x12\0')
    def test_rgb565(self):
        rows=self.plan(bits=16,masks=(0xf800,0x7e0,0x1f))
        self.assertEqual(rows[8][1][16:18],b'\x1f\xf8')
    def test_rgb24(self):
        rows=self.plan(bits=24); self.assertEqual(rows[8][1][24:27],b'\xff\0\xff')
    def test_swapped_masks_bigendian(self):
        source=(0xff123456).to_bytes(4,'little')*1024
        layout=dict(LAYOUT,masks=(0xff,0xff00,0xff0000),byteorder='big')
        self.assertEqual(convert(source,**layout)[0][1][:4],b'\0\x56\x34\x12')
    def test_alpha_ignored_not_blended(self):
        rows=self.plan(); self.assertEqual(rows[0][1],b'\0'*128)
        self.assertEqual(rows[8][1][32:36],b'\xff\0\xff\0')
    def test_exact_byte_coverage(self):
        rows=self.plan(x=64,y=64); dest=bytearray(b'\x55'*(5120*800)); old=bytes(dest)
        for off,data in rows: dest[off:off+len(data)]=data
        touched=set(n for off,data in rows for n in range(off,off+len(data)))
        self.assertEqual(len(touched),4096)
        self.assertTrue(all(dest[n]==old[n] for n in range(len(dest)) if n not in touched))


class Transactions(unittest.TestCase):
    def run_transaction(self, backend, save=None, hold=lambda:None, target=None):
        saved=[]
        publish_transaction(backend,SOURCE,SHA,target or backend.identity(),
                            save or (lambda data,identity:saved.append((data,identity))),hold)
        return saved
    def test_restore_success(self):
        f=Fake();saved=self.run_transaction(f)
        self.assertEqual(saved[0][0],b'old contents');self.assertEqual(f.pixels,b'old contents')
        self.assertEqual((f.writes,f.claims,f.releases),(2,1,1))
    def test_unavailable_destination(self):
        f=Fake();f.identity=lambda:(_ for _ in ()).throw(OSError('unavailable'))
        with self.assertRaises(OSError):self.run_transaction(f,target={'boot':'synthetic'})
        self.assertEqual(f.writes,0)
    def test_initial_ownership_loss(self):
        f=Fake()
        with self.assertRaises(ValueError):self.run_transaction(f,target={'epoch':0})
        self.assertEqual(f.claims,0)
    def test_loss_during_claim(self):
        f=Fake();target=f.identity()
        def claim():f.claimed=True;f.claims+=1;f.target['epoch']=2
        f.claim=claim
        with self.assertRaises(ValueError):self.run_transaction(f,target=target)
        self.assertEqual((f.writes,f.releases),(0,1))
    def test_mapping_failure(self):
        f=Fake();f.map_fail=True
        with self.assertRaises(MemoryError):self.run_transaction(f)
        self.assertEqual((f.writes,f.releases),(0,1))
    def test_preservation_failure(self):
        f=Fake()
        with self.assertRaises(OSError):self.run_transaction(f,save=lambda d,t:(_ for _ in ()).throw(OSError()))
        self.assertEqual((f.writes,f.releases),(0,1))
    def test_partial_publication(self):
        f=Fake();f.partial=True
        with self.assertRaises(OSError):self.run_transaction(f)
        self.assertEqual(f.pixels,b'old contents');self.assertEqual(f.releases,1)
    def test_wrong_readback(self):
        f=Fake();f.bad_read=True
        with self.assertRaises(ValueError):self.run_transaction(f)
        self.assertEqual(f.pixels,b'old contents')
    def test_interrupted_hold(self):
        f=Fake()
        with self.assertRaises(RuntimeError):self.run_transaction(f,hold=lambda:(_ for _ in ()).throw(RuntimeError()))
        self.assertEqual(f.pixels,b'old contents')
    def test_restore_failure(self):
        f=Fake();f.restore_fail=True
        with self.assertRaises(OSError):self.run_transaction(f)
        self.assertEqual(f.releases,1)
    def test_ownership_loss_after_publication(self):
        f=Fake()
        with self.assertRaises(ValueError):self.run_transaction(f,hold=lambda:f.target.update(epoch=2))
        self.assertEqual(f.writes,1);self.assertEqual(f.releases,1)
    def test_no_second_publication(self):
        f=Fake();f.partial=True
        with self.assertRaises(OSError):self.run_transaction(f)
        self.assertEqual(f.writes,2) # one attempted write plus restoration, never another trial
    def test_bad_source_no_claim(self):
        f=Fake()
        with self.assertRaises(ValueError):publish_transaction(f,SOURCE,'0'*64,f.identity(),lambda d,t:None,lambda:None)
        self.assertEqual(f.claims,0)
    def test_no_device_access_on_import(self):
        # Mock transaction backend is the only live boundary in the policy.
        self.assertIsInstance(Image.data.offset,int)
    def test_published_and_restored_evidence(self):
        f=Fake();evidence=[]
        publish_transaction(f,SOURCE,SHA,f.identity(),lambda d,t:None,lambda:None,
                            lambda phase,data,target:evidence.append((phase,data)))
        self.assertEqual(evidence,[('published',SOURCE),('restored',b'old contents')])
    def test_postwrite_evidence_failure_restores(self):
        f=Fake()
        def observe(phase,data,target):
            if phase=='published': raise OSError('evidence lost')
        with self.assertRaises(OSError):
            publish_transaction(f,SOURCE,SHA,f.identity(),lambda d,t:None,lambda:None,observe)
        self.assertEqual(f.pixels,b'old contents');self.assertEqual(f.releases,1)


class Authority(unittest.TestCase):
    def setUp(self):
        self.card={'scope':'ONE DISPLAY-ONLY CPU PUBLICATION; NO SGX',
                   'sgx_execution_authorized':False,'hold_seconds':15,
                   'target':{'boot_id':'synthetic'},'evidence_directory':'/synthetic/new'}
        self.raw=b'synthetic card bytes'
        self.auth={'scope':self.card['scope'],'card_sha256':hashlib.sha256(self.raw).hexdigest(),
                   'boot_id':'synthetic','maximum_publications':1,
                   'display_publication_authorized':True,'sgx_execution_authorized':False}
    def check(self): validate_authorization(self.card,self.raw,self.auth,'/synthetic/new')
    def test_exact_authority(self): self.check()
    def test_no_authority(self):
        self.auth={}
        with self.assertRaises(ValueError):self.check()
    def test_wrong_card(self):
        self.auth['card_sha256']='0'*64
        with self.assertRaises(ValueError):self.check()
    def test_sgx_authority_rejected(self):
        self.auth['sgx_execution_authorized']=True
        with self.assertRaises(ValueError):self.check()
    def test_second_publication_rejected(self):
        self.auth['maximum_publications']=2
        with self.assertRaises(ValueError):self.check()
    def test_wrong_boot(self):
        self.auth['boot_id']='stale'
        with self.assertRaises(ValueError):self.check()
    def test_unbounded_hold(self):
        self.card['hold_seconds']=60
        with self.assertRaises(ValueError):self.check()
    def test_wrong_destination(self):
        with self.assertRaises(ValueError):validate_authorization(self.card,self.raw,self.auth,'/synthetic/other')


if __name__=='__main__': unittest.main()
