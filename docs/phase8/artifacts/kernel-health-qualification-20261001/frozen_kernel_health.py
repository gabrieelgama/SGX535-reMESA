"""Operational kernel-log guard; known diagnostics remain visible, never generic fault exemptions.

The five bounded diagnostic forms derive from the preserved stock capture, not
an arbitrary caller-provided whitelist. Fault-bearing/repeated/changed variants
fail closed. This is an operational predicate, not architectural health proof.
"""
import re
from collections import Counter

# Strip only dmesg transport prefixes; retain the message and all diagnostic fields.
PREFIX = re.compile(r"^(?:<\d+>)?\s*(?:\[\s*\d+(?:\.\d+)?\]\s*)?(?:kernel:\s*)?", re.I)
FATAL = re.compile(r"^(?:BUG:)|\bkernel BUG at\b|\bOops:|\bKernel panic\b|\bgeneral protection(?: fault|:)"
                   r"|\bCall Trace:|\b(?:soft|hard)\s+LOCKUP\b|\bblocked for more than\b"
                   r"|\brcu[^\n]*\b(?:stall|stalls)\b", re.I)
WARNING = re.compile(r"\bWARNING:|\bWARN_ON\b|\bBUG:", re.I)
GRAPHICS = re.compile(r"\b(?:gma500|drm|psb|sgx|pvr)\b|0000:00:02\.0", re.I)
GRAPHICS_ERROR = re.compile(r"\b(?:BUG:|fault|error|failed|failure|fatal|timeout|timed out|hang|hung|WARN|warning)\b", re.I)
ACPI_DIAGNOSTICS = {
 'ACPI Warning: Could not enable fixed event - PowerButton (2) (20200925/evxface-618)': ('acpi_powerbutton_warning', 2),
 'ACPI Error: Could not enable PowerButton event (20200925/evxfevnt-182)': ('acpi_powerbutton_error', 2),
 'button: probe of LNXPWRBN:00 failed with error -22': ('powerbutton_probe', 1),
 'tiny-power-button: probe of LNXPWRBN:00 failed with error -22': ('tiny_powerbutton_probe', 1),
}
BACKLIGHT = re.compile(r"gma500 0000:00:02\.0: BL bug: Reg ([0-9a-fA-F]{8}) save ([0-9a-fA-F]{8})")

def classify_kernel_log(log):
 """Return visible classification receipt; reject on any selected adverse signal."""
 if not isinstance(log, str) or not log.strip():
  return {'classification': 'REJECT', 'faults': [{'reason': 'missing kernel log'}], 'stock_diagnostics': {}}
 counts = Counter(); faults = []
 for number, raw in enumerate(log.splitlines(), 1):
  message = PREFIX.sub('', raw, count=1)
  reason = None
  # Fatal reports are rejected even alongside a previously seen diagnostic.
  if FATAL.search(message): reason = 'kernel fault/lockup'
  else:
   known = ACPI_DIAGNOSTICS.get(message)
   backlight = BACKLIGHT.fullmatch(message)
   if backlight:
    if any(int(value, 16) != 0 for value in backlight.groups()): reason = 'changed backlight diagnostic fields'
    else: known = ('backlight_zero_register', 1)
   if known:
    label, limit = known; counts[label] += 1
    if counts[label] > limit: reason = 'repeated stock diagnostic: ' + label
   elif WARNING.search(message): reason = 'kernel warning'
   elif re.search(r'\bACPI (?:Error|Warning):', message, re.I): reason = 'unexpected ACPI diagnostic'
   elif GRAPHICS.search(message) and GRAPHICS_ERROR.search(message): reason = 'graphics fault/error'
  if reason: faults.append({'line': number, 'message': message, 'reason': reason})
 return {'classification': 'REJECT' if faults else 'PASS WITH BOUNDED STOCK DIAGNOSTICS',
         'faults': faults, 'stock_diagnostics': dict(counts)}
