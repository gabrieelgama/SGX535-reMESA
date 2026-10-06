import sys,hashlib,struct,json,copy
from pathlib import Path
sys.dont_write_bytecode=True
sys.path.insert(0,'/home/gama/sgx535-gfx/tools/display');import square_display_centered as s
W=Path(Path('/tmp/sgx535-square-continuation-workspace').read_text());source=(W/'one-authorized-square-call/originals/color.original.bin').read_bytes();checks=[]
def need(name,value):
 checks.append({'name':name,'pass':bool(value)});assert value,name
s.validate_square(source,s.SOURCE_SHA);need('sealed source accepted',True)
for name,b,sha in [('truncated',source[:-1],s.SOURCE_SHA),('wrong hash',source,'0'*64),('changed bytes',b'\0'*4096,s.SOURCE_SHA)]:
 try:s.validate_square(b,sha);passed=False
 except ValueError:passed=True
 need(name+' rejected',passed)
raw=s.centered.enlarge(source);need('bounded enlarged byte size',len(raw)==409600);words=struct.unpack('<102400I',raw);need('exact expected magenta count',words.count(0x00ff00ff)==25600);need('exact zero count',words.count(0)==76800)
need('exact centered square footprint',all(v==(0x00ff00ff if 80<=i%320<240 and 80<=i//320<240 else 0) for i,v in enumerate(words)))
rows=s.centered.bounds(1280,800,5120);need('320 bounded scanout row ranges',len(rows)==320 and all(0<=a<b<=5120*800 and b-a==1280 for a,b in rows));need('unmodified padding',all(a==y*5120+1920 and b==y*5120+3200 for y,(a,b) in zip(range(240,560),rows)))
for layout in [(1280,800,8192),(1279,800,5120),(1280,799,5120)]:
 try:s.centered.bounds(*layout);passed=False
 except ValueError:passed=True
 need('unsupported layout '+str(layout)+' rejected',passed)
class Backend:
 def __init__(self,fail=None):self.data=bytes([37])*s.BYTES;self.original=self.data;self.owned=False;self.writes=0;self.fail=fail
 def identity(self):return {'owner':'qualified-Xorg'} if self.fail!='ownership' else {'owner':'other'}
 def claim(self):self.owned=True
 def release(self):self.owned=False
 def read(self):
  if self.fail=='mapping':raise ValueError('mapping failure')
  return self.data
 def prepare(self,source):return s.centered.enlarge(source)
 def agrees(self,a,b):return a==b
 def write(self,image):
  self.writes+=1;self.data=image
  if self.fail=='partial' and self.writes==1:raise ValueError('partial write')
for fail in [None,'ownership','mapping','partial','backup']:
 b=Backend(fail);saved=[]
 def preserve(data,target):
  if fail=='backup':raise ValueError('backup save failed')
  saved.append(data)
 try:s.pixels.publish_transaction(b,source,s.SOURCE_SHA,{'owner':'qualified-Xorg'},preserve,lambda:None);success=True
 except ValueError:success=False
 need(str(fail)+' publication status',success==(fail is None));need(str(fail)+' cleanup/restore',b.data==b.original and not b.owned)
 if fail in ['ownership','mapping','backup']:need(str(fail)+' no publication before valid backup',b.writes==0)
need('source unchanged',hashlib.sha256(source).hexdigest()==s.SOURCE_SHA)
print(json.dumps({'scope':'CPU-only adapter construction and qualified transaction reuse; not hardware proof','passed':len(checks),'total':len(checks),'checks':checks,'sgx_invocations':0},indent=2))
