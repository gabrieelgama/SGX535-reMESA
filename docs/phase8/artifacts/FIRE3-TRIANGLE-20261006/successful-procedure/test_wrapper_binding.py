import unittest,ast,json
import controller as c
class TestWrapper(unittest.TestCase):
 def test_first_owner_actual_child_args_and_guard_preservation(self):
  s=c.early('first-owner');w=c.capture_wrapper(s,'experimental','first-owner');t=ast.parse(w)
  call=next(n for n in ast.walk(t) if isinstance(n,ast.Call) and isinstance(n.func,ast.Attribute) and n.func.attr=='run')
  a=ast.literal_eval(call.args[0]);self.assertEqual(a[-2:],['--prior-boot',c.B['stock_boot']]);self.assertEqual(a[9],s)
  self.assertIn('LIMIT = 1200',w);self.assertIn('same_boot_timing_pass',w);self.assertNotIn('sys.argv=',s)
 def test_no_child_argument_fallback(self):
  with self.assertRaises(ValueError):c.capture_wrapper('print(1)','experimental','incoming')
  w=c.capture_wrapper('print(1)','stock_preparation');self.assertIn('LIMIT = None',w)
if __name__=='__main__':unittest.main(verbosity=2)
