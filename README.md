<div align="center">

# SGX535-reMESA

### Reverse engineering PowerVR Series5 GPUs because apparently nobody stopped me after the first one.

**Reverse engineering · Linux · Mesa · old GPU suffering**

![Status](https://img.shields.io/badge/status-Phase%209%20%2F%20Mesa-purple)
![GPU](https://img.shields.io/badge/GPU-PowerVR%20SGX535-blue)
![Platform](https://img.shields.io/badge/platform-Intel%20Poulsbo-lightgrey)
![Linux](https://img.shields.io/badge/Linux-gma500-yellow)
![Triangle](https://img.shields.io/badge/triangle-ESTABLISHED-brightgreen)
![Mesa](https://img.shields.io/badge/Mesa-now™-purple)

> **We don't have the SGX535 programming manual.**
>
> **We also don't have a modern driver.**
>
> So we're figuring it out ourselves.
>
> yes, including the driver.
>
> apparently one SGX was not enough either.

</div>

---

# Current phase

## **Phase 9 — MESA**

Current status:

```text
experimental boot:        passed
privileged capture:       established
first-owner evidence:     established
old Gate B:               PASSED
first fixed ioctl:        REACHED

TA submission:            YESSSSSSSSSSS
rasterization:            YESSSSSSSSSSSSSSS
triangle:                 FUCKING YESSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
                          2:51 AM BRT

FIRE #3:                  exactly once
client exit:              0
ioctl return:             0
operation errno:          0
final phase:              RETIRED

readback:                 32x32
magenta pixels:           120
zero pixels:              904
triangle:                 ESTABLISHED

first live blocker:       -EBUSY
root cause:               GPU VA allocator
root cause status:        identified
allocator fix:            qualified

corrected module:         qualified
corrected initramfs:      qualified
corrected Gate B:         PASS

Phase 8:                  DONE
Phase 9:                  MESA

sanity:                   no more
```

Yes.

# THE TRIANGLE HAPPENED.

At approximately **2:51 AM BRT**, FIRE #3 submitted the fixed diagnostic
workload exactly once.

The client returned successfully.

The operation reached retirement.

The sealed 32×32 readback contained:

```text
total pixels:     1024
magenta:           120
zero:              904

unique values:

0xffff00ff
0x00000000
```

The 120 foreground pixels occupied:

```text
bounding box:

(8,8) -> (22,22)
```

with rows:

```text
███████████████
██████████████
█████████████
████████████
███████████
██████████
█████████
████████
███████
██████
█████
████
███
██
█
```

Exactly:

```text
15 + 14 + 13 + ... + 1 = 120
```

The complete readback matched the expected filled triangle with
**zero whole-image reference mismatches**.

TA completion was observed.

End-render was observed.

3D-memory-free was observed.

Retirement was observed.

The response, closed evidence capsule and complete 4096-byte readback
all agreed on the same attributable operation.

So:

```text
Triangle:
ESTABLISHED
```

No retry was required.

No second FIRE #3 invocation occurred.

The authorization was consumed after the single invocation.

The diagnostic triangle does not need another execution to establish it.

After approximately 20 days of reverse engineering:

# 🔺

there it is.

---

# What is this?

**SGX535-reMESA** is a reverse-engineering and driver-development project
for the **PowerVR SGX535**, initially focused on the version inside
**Intel Poulsbo / GMA 500** systems.

The original objective looked like this:

```text
SGX535
   ↓
Linux
   ↓
???
   ↓
  🔺
```

The triangle happened.

So the objective has changed.

Now:

```text
SGX535
   ↓
Linux
   ↓
Mesa / Gallium
   ↓
OpenGL
   ↓
???
   ↓
Minecraft?
```

Yes.

We actually have to write the driver now.

The SGX535 documentation needed to write a modern open-source driver is
not publicly available in enough detail, so the project has been
reconstructing it from:

- historical source code
- old drivers
- surviving documentation
- register definitions
- Linux sources
- command/state analysis
- real hardware
- controlled experiments
- readback evidence
- questionable amounts of `grep`
- files from 2009 that somehow still exist
- pure refusal to let this GPU die

SGX535 / Poulsbo is the current focus.

**Current.**

Because apparently there are more of these things.

We'll get to that later.

---

# THE GPU SPOKE

Historical Poulsbo DDK material pointed strongly toward **SGX535 rev121**.

But eventually we stopped asking old source code and asked the actual GPU.

Controlled MMIO reads on original Poulsbo hardware returned:

```text
CORE_ID       = 0x01130000
CORE_REVISION = 0x00010201
```

Decode:

```text
Major       = 1
Minor       = 2
Maintenance = 1
```

So the tested physical SGX535 is:

# rev121

yes.

**YESSSSSSS IT'S REV121**

Several days of archaeology were defeated by two `readl()`s.

The GPU had the answer the entire time.

It just wasn't asked.

> This confirms rev121 on the tested machine. It does not automatically
> mean every Poulsbo SGX535 ever manufactured is rev121.

Because:

```text
one laptop
   ≠
every SGX535 produced on Earth
```

Evidence policy strikes again.

---

# THE GPU DREW SOMETHING

Reading the identity registers was one thing.

Making the GPU actually process reconstructed 3D state was another.

A considerably more annoying thing.

After controlled first-load qualification, evidence capture, Gate B
review, fixed-scene construction and multiple diagnostic stages, the
project reached a one-shot SGX invocation.

The final diagnostic operation produced:

```text
TA submission:       confirmed
end-render:          confirmed
3D memory free:      confirmed
retirement:          confirmed

readback bytes:      4096
pixels:              1024
magenta pixels:      120
background pixels:   904
```

And those pixels were not randomly scattered.

They formed the expected triangle.

```text
               █
              ██
             ███
            ████
           █████
          ██████
         ███████
        ████████
       █████████
      ██████████
     ███████████
    ████████████
   █████████████
  ██████████████
 ███████████████
```

The spatial comparison reported zero mismatches against the expected
whole-image reference.

Classification:

```text
CONSTANT_FRAGMENT_HYPOTHESIS_SUPPORTED
Triangle ESTABLISHED
```

The fragment result strongly supports the diagnostic hypothesis that led
to FIRE #3.

It does **not** magically turn every earlier hypothesis into confirmed
architecture.

Because even after finally getting the triangle:

> **Evidence first. Guessing second.**

Yes.

The evidence policy survived the triangle.

Unfortunately.

---

# Why?

Poulsbo machines contain an actual **PowerVR SGX535**.

Linux still supports the display side through `gma500`.

Historically, the 3D situation looked approximately like:

```text
Linux:   display works 👍
SGX535:  hello
Mesa:    who are you
```

There is no modern open-source SGX535 3D driver.

So that's what this project is trying to fix.

The first reconstructed diagnostic triangle means the project has now
crossed an important boundary:

```text
"can we make this thing execute reconstructed 3D work?"

YES.
```

That is not the same as having a Mesa driver.

Not even close.

But it means Phase 9 can finally begin for real.

```text
2008:

Intel:
here is GMA 500

2026:

reMESA:
fine, I'll do it myself
```

---

# Digital archaeology

We have recovered a surprising amount of stuff.

| Area | Status |
|---|:---:|
| SGX535 register definitions | ✅ |
| Poulsbo DDK target | ✅ |
| Historical rev121 target | ✅ |
| Physical rev121 observation | ✅ |
| MMU / BIF | 🟡 |
| Power / reset | 🟡 |
| Interrupts | 🟡 |
| USE / USSE | 🟡 |
| PDS | 🟡 |
| Command submission | 🟡 |
| First-load qualification | ✅ |
| Controlled SGX execution | ✅ |
| TA submission | ✅ |
| Rasterization | ✅ |
| Diagnostic triangle | 🔺 |
| Shader ISA | ❓ |
| Microkernel | ❓ |
| Mesa driver | 💀 |

Useful historical material includes:

- `sgx535defs.h`
- `pc_i686_poulsbo_d0_linux`
- `services4/system/poulsbo`
- SGX register definitions
- MMU / BIF definitions
- PDS / USE material
- host ↔ SGX structures
- command-submission code
- Linux `gma500`
- historical PSB / Poulsbo userspace
- EMGD-related material
- various ancient files that somehow survived until 2026

The general research experience looks approximately like this:

```text
grep
 ↓
header
 ↓
another header
 ↓
dead link
 ↓
old tarball
 ↓
2009 source tree
 ↓
interesting dword
 ↓
why
 ↓
three weeks later
 ↓
🔺
```

---

# Evidence policy

The main rule is:

> **If we don't know, we don't know.**

No `0xDEADBEEF` archaeology where somebody looks at three bits and declares:

> obviously this means `ENABLE_TRIANGLE_ENGINE`

Findings are classified as:

| | Classification | Meaning |
|---|---|---|
| 🟢 | **CONFIRMED** | Evidence actually says this |
| 🟡 | **INFERRED** | Probably, but calm down |
| ⚫ | **UNKNOWN** | ¯\\_(ツ)_/¯ |

Claims should ideally point to an exact:

- repository
- commit
- file
- line
- document
- experiment
- hardware observation
- captured operation
- sealed artifact

### `UNKNOWN` is allowed.

Making something up because it would make the documentation look more
complete is not.

If an unexplained dword works, it remains:

```text
mysterious_dword_that_works
```

until we actually know what it does.

This policy is very useful.

It is also incredibly annoying when you really want the triangle.

```text
Researcher:
It should work.

Evidence policy:
source?

Researcher:
trust me bro

Evidence policy:
DENIED
```

Eventually:

```text
Researcher:
I have the readback.

Evidence policy:
hashes?

Researcher:
yes

Evidence policy:
provenance?

Researcher:
yes

Evidence policy:
spatial comparison?

Researcher:
zero mismatches

Evidence policy:
operation attributable?

Researcher:
yes

Evidence policy:
fine.

Triangle:
🔺
```

One rule survives every GPU:

> **Evidence first. Guessing second.**

Apparently it works.

---

# The `-EBUSY` incident

Before the successful triangle, the first live fixed ioctl reached the
kernel and returned:

```text
errno:    -EBUSY
outcome:  1
phase:    0
events:   0x00000000
```

No TA or raster work was submitted during that failed attempt.

The failure was traced to the private GPU virtual-address allocator.

GPU virtual resources were tagged with `IORESOURCE_MEM`, causing the x86
resource allocator to apply CPU E820 reservations to addresses that were
actually SGX virtual addresses.

Result:

```text
PDS VA window:
0x20000000 - 0x2fffffff

CPU RAM:
"nice address space you have there"
```

The allocator rejected the first PDS BO before the fixed SGX workload
could begin.

The minimal correction removed the physical-memory resource semantics
from the private GPU VA allocator while preserving its bounds, alignment,
overlap checks and GTT exclusion.

Offline qualification reported:

```text
PDS allocation:           0x20000000 - 0x2001ffff
GTT overlap rejection:    PASS
occupied-range rejection: PASS

target ABI imports:       232 / 232
CRC mismatches:           0
module_layout:            0xb84efb99

corrected module:         reproducible
corrected initramfs:      reproducible
```

At the time:

```text
offline qualified
       ≠
live qualified
```

The evidence policy won again.

damn.

That blocker was eventually cleared through the controlled qualification
process.

And then:

```text
-EBUSY
   ↓
allocator investigation
   ↓
fix
   ↓
qualification
   ↓
Gate B
   ↓
FIRE
   ↓
TA
   ↓
raster
   ↓
🔺
```

So `-EBUSY` is no longer the current blocker.

It is now archaeology about the archaeology.

Excellent.

---

# Current status

> **This is still not a working Mesa driver. Do not install it expecting Minecraft.**

```text
Phase 1   ████████████████████   ✓
Phase 2   ████████████████████   ✓
Phase 3   ████████████████████   ✓
Phase 4   ████████████████████   ✓
Phase 5   ████████████████████   ✓
Phase 6   ████████████████████   ✓
Phase 7   ████████████████████   ✓
Phase 8   ████████████████████   ✓  🔺
Phase 9   █░░░░░░░░░░░░░░░░░░░   Mesa
```

### Phase 8

Also known as:

```text
the phase that existed because the kernel said no
```

Phase 8 eventually produced:

- an experimental first-load kernel
- successful experimental boots
- verified recovery to the original stock graphics state
- privileged evidence capture
- first-owner evidence
- deterministic fixed-scene tooling
- qualified module transitions
- Gate B review
- an actual non-empty whitelist
- fixed SGX invocation
- TA submission
- rasterization
- complete readback
- a triangle
- considerably less sanity

The important transition was:

```text
Gate B:
BLOCKED
```

becoming:

```text
Gate B:
PASS
```

for a specifically reviewed operation.

The corresponding one-shot authorization was then consumed exactly once.

No retry occurred.

The resulting diagnostic triangle was preserved and classified:

```text
Triangle: ESTABLISHED
```

So Phase 8 can finally stop holding the repository hostage.

Thank you.

Please leave.

---

# Phase 9

# Mesa.

Yes.

Actual Mesa.

The thing in the repository name.

```text
Phase 1-7:
reverse engineer GPU

Phase 8:
argue with kernel until triangle

Phase 9:
remember why project is called reMESA
```

The rough objective is now:

```text
SGX535
   │
   ▼
kernel / DRM
   │
   ▼
userspace driver
   │
   ▼
Mesa / Gallium
   │
   ▼
OpenGL
   │
   ▼
Minecraft?
```

The first triangle was never the final driver.

It was the proof that this path can reach real SGX535 execution and
produce a known raster result.

Now the hard part becomes turning reconstructed low-level knowledge into
something a normal graphics stack can actually use.

You know.

The driver.

The thing we said we were making.

---

# Initial target

| Component | Target |
|---|---|
| Platform | Intel Poulsbo |
| Chipset | Intel US15W |
| Graphics | Intel GMA 500 |
| GPU | PowerVR SGX535 |
| Revision | rev121 on tested hardware |
| CPU | Intel Atom Z5xx |
| OS | Linux |
| Age | ancient |
| Will to live | apparently yes |

---

# Documentation

Actual technical documentation lives in [`docs/`](docs/).

| Document | Subject |
|---|---|
| [`architecture.md`](docs/architecture.md) | SGX architecture |
| [`registers.md`](docs/registers.md) | Registers |
| [`mmu-bif.md`](docs/mmu-bif.md) | MMU / BIF |
| [`command-submission.md`](docs/command-submission.md) | Command submission |
| [`usse.md`](docs/usse.md) | USE / USSE / PDS |
| [`firmware.md`](docs/firmware.md) | Firmware / microkernel |
| [`gma500-current-state.md`](docs/gma500-current-state.md) | Current Linux situation |
| [`poulsbo-evidence.md`](docs/poulsbo-evidence.md) | Poulsbo archaeology |
| [`sgx535-missing-files.md`](docs/sgx535-missing-files.md) | Things the internet ate |
| [`unknowns.md`](docs/unknowns.md) | ??? |

If you're looking for the serious research:

**it's in there.**

If you're looking for the triangle:

**WE FOUND IT.**

---

# Roadmap

- [x] Find ancient SGX535 stuff
- [x] Find more ancient SGX535 stuff
- [x] Realize there is a *lot* of ancient SGX535 stuff
- [x] Identify Poulsbo DDK target
- [x] Reconstruct registers
- [x] Reconstruct MMU / BIF
- [x] Reconstruct Poulsbo integration
- [x] Build evidence tracking
- [x] Touch real hardware
- [x] Read SGX registers without exploding anything
- [x] Read `CORE_ID`
- [x] Read `CORE_REVISION`
- [x] Discover that yes, it really is rev121
- [x] Reconstruct the historical launch-state evidence envelope
- [x] Build fixed one-shot bring-up tooling
- [x] Reach userspace with the experimental first-load kernel
- [x] Recover cleanly to stock
- [x] Capture and qualify experimental first-load evidence
- [x] Clear Gate B
- [x] Controlled SGX execution
- [x] Fixed command submission
- [x] Make GPU do literally anything useful
- [x] 🔺 **TRIANGLE**
- [x] Stare at triangle for an unreasonable amount of time
- [x] Phase 9
- [ ] Mesa/Gallium driver
- [ ] OpenGL
- [ ] Put the triangle through the real graphics stack
- [ ] Minecraft on GMA 500 because it would be funny
- [ ] Declare victory
- [ ] Rest
- [ ] Notice another SGX exists
- [ ] Say "hmm"
- [ ] Make another terrible decision
- [ ] Repeat until Series5 stops answering

---

# Why `reMESA`?

The naming process was highly sophisticated:

```text
reverse engineering
        +
       Mesa
        =
     reMESA
```

Years of branding expertise were involved.

Approximately 14 seconds.

Originally, the scientific objective was:

```text
            SGX535
               │
               ▼
             Mesa
               │
               ▼
              🔺
```

Then we got impatient and made the triangle before Mesa.

So now:

```text
            SGX535
               │
               ▼
              🔺
               │
               ▼
             Mesa
               │
               ▼
         normal software
               │
               ▼
              ???
```

## THE TRIANGLE

There was no ray tracing.

There was no path tracing.

There was not even a texture.

There were:

# three vertices.

And we were happy.

At 2:51 AM.

---

# Wait... there are MORE SGXs?

Unfortunately.

SGX535 / Poulsbo is the current focus.

But the longer-term scope may expand to other **PowerVR Series5 GPUs
and platforms**.

Current situation:

```text
SGX535:     TRIANGLE ACHIEVED 🔺
other SGXs: 👀
```

The important part is that some of the archaeology may be useful more
than once.

Research around:

- PDS
- USE / USSE
- MMU / BIF
- Services / DDK structures
- command submission
- firmware / microkernel interfaces
- historical PowerVR userspace
- Linux integration
- reverse-engineering methodology

may help investigations of related Series5 hardware.

**May.**

Related does not mean identical.

Every GPU, revision, SoC, chipset, platform and sufficiently cursed
binary still needs evidence of its own.

We are not doing this:

```text
SGX535 does X
      ↓
therefore every PowerVR GPU since the dawn of time does X
```

No.

That is how technical documentation becomes fan fiction.

---

# Working on another SGX?

If you're researching another **PowerVR SGX / Series5 GPU**, you don't
necessarily have to start completely alone.

If our work overlaps, we can compare:

- register research
- PDS / USE / USSE findings
- MMU / BIF behavior
- DDK structures
- command streams
- historical drivers
- hardware observations
- tooling and experiments
- weird binaries
- ancient PowerVR mysteries

And if your project fits the scope:

# **you can also integrate your project with SGX535-reMESA.**

The goal is not to pretend every Series5 GPU is identical.

The goal is to avoid five developers independently discovering the same
terrible register definition in five different 2009 source trees.

```text
your SGX project
       │
       ├── independent project
       │
       └── integrate with reMESA
                    │
                    ▼
          shared Series5 research
                    │
          ┌─────────┼─────────┐
          ▼         ▼         ▼
        SGX535    another    another
          │         SGX       SGX
          └─────────┼─────────┘
                    │
                    ▼
                  Linux
                    │
                    ▼
                  Mesa?
                    │
                    ▼
                   🔺
```

If you're working on another SGX and want to collaborate:

**operatingsystemsdepression@gmail.com**

yes.

that is the actual email.

Example:

```text
Developer:
I have an SGX540 project.

reMESA:
interesting

Developer:
Can I integrate it?

reMESA:
show me the evidence

Developer:
I have register dumps,
DDK material,
hardware observations,
and the actual machine.

reMESA:
COME IN
```

One rule survives every GPU:

> **Evidence first. Guessing second.**

Now we can add:

> **Triangles are also acceptable evidence.**

---

# Sponsor the triangle

Yes.

This project has **GitHub Sponsors**.

We somehow reached the point where an attempt to make three vertices
appear on a PowerVR GPU from 2008 had a funding button.

And then the triangle actually appeared.

<div align="center">

### ❤️ [Sponsor SGX535-reMESA](https://github.com/sponsors/gabrieelgama)

**Fund old GPU archaeology.**

**Fund unreasonable amounts of `grep`.**

**Fund Mesa.**

🔺

</div>

Sponsorship can help support things around the project such as:

- acquiring old hardware for testing
- replacement parts for old hardware that has discovered mortality
- adapters and test equipment
- shipping ancient computers across unreasonable distances
- additional Series5 hardware
- infrastructure and development costs
- the continuing search for documentation the internet ate
- turning one extremely important triangle into an actual driver

Most importantly:

```text
Sponsor:
What does my support get?

reMESA:
More archaeology.

Sponsor:
And the triangle?

reMESA:
we have one now

Sponsor:
wait what

reMESA:
🔺
```

Please note that sponsoring the project does **not** bypass the evidence
policy.

```text
Sponsor:
I donated.

Evidence policy:
thank you

Sponsor:
Can we call this register confirmed?

Evidence policy:
source?

Sponsor:
...

Evidence policy:
DENIED
```

Money cannot bribe `UNKNOWN` into becoming `CONFIRMED`.

We checked.

The triangle survived peer review.

```text
GitHub Sponsors:
$ ❤️

SGX535:
🔺

reMESA:
OH MY GOD IT ACTUALLY DID SOMETHING
```

And yes:

```text
2008 GPU reverse-engineering project
            ↓
     GitHub Sponsors
            ↓
          money
            ↓
       old hardware
            ↓
          research
            ↓
           🔺
            ↓
          Mesa
```

The triangle has an economy.

This has gotten out of hand.

---

# Contributing

Useful things include:

- SGX535 documentation
- Poulsbo / PSB material
- old PowerVR DDK files
- EMGD material
- PDS / USE / USSE research
- command streams
- shader ISA research
- Mesa
- `gma500`
- old hardware
- provenance checking
- research on other PowerVR Series5 GPUs
- register dumps with known provenance
- weird Intel / PowerVR hardware that refuses to die
- someone somehow having the SGX535 programming manual in a drawer since 2009

Most important rule:

> **Evidence first. Guessing second.**

If your conclusion is:

```text
I have absolutely no idea what this register does
```

that's useful information too.

Seriously.

`UNKNOWN` is significantly better than confidently documenting nonsense.

Meanwhile:

```text
Eurasia.3D Input Parameter Format.1.3.37a.SGX535 1.2.External.pdf

2009:
exists

2026:
lol no
```

If you have this document:

```text
hello

we should talk
```

Immediately.

Please.

The triangle did not magically make the missing documentation come back.

---

# Frequently asked questions

### Does the driver work?

Not as a Mesa driver yet.

### Does the GPU work?

# YES.

### Does reconstructed 3D execution work?

We have a controlled diagnostic operation with confirmed TA submission,
rasterization, retirement and triangle readback.

So:

**yes, in that specifically established scope.**

### Did it actually draw a triangle?

# YESSSSSSSSSSSSSSSSS.

120 magenta pixels.

Zero reference mismatches.

We checked.

A lot.

### Does it support OpenGL?

Not yet.

That is Phase 9.

### Vulkan?

bro.

### OpenGL 4.6?

BRO.

### Ray tracing?

We have exactly one very important triangle.

Please behave.

### Can it run Crysis?

We only recently stopped negotiating with three vertices.

### Can it run Minecraft?

This question has officially advanced from:

```text
lol
```

to:

```text
technically now we have to find out
```

### Why not just use a modern GPU?

Because the modern GPU already has a driver.

Where is the fun in that?

### Why SGX535?

At this point it's personal.

### Why other Series5 GPUs?

At this point it's becoming a collection.

### Can I sponsor the project?

Apparently yes.

### Will sponsoring make Mesa appear faster?

```text
Evidence:
UNKNOWN
```

---

# Disclaimer

This is an independent reverse-engineering and preservation project.

It is not affiliated with or endorsed by Imagination Technologies, Intel,
Texas Instruments, Mesa, or the Linux kernel project.

Third-party material remains subject to its respective licenses.

Please do not sue the triangle.

It exists now.

It is only 120 pixels.

It has suffered enough.

---

# AI usage

AI is used to help with:

- source archaeology
- searching large trees
- comparing code
- organizing findings
- documentation
- experimental scaffolding
- asking why an unexplained dword exists for the 48th time
- calling me an "operator" for some reason
- staring at enormous source trees without blinking
- discovering that the answer is sometimes still `UNKNOWN`
- spending an unreasonable amount of compute proving that 120 magenta
  pixels are, in fact, a triangle

But:

```text
AI said it
   ≠
hardware documentation
```

Important claims still need evidence from source material, analysis, or
actual hardware.

Sometimes the correct AI-assisted research result is still:

```text
UNKNOWN
```

good.

Sometimes:

```text
AI:
"This probably means..."

Evidence policy:
PROBABLY?
```

and we go look again.

The AI:

```text
likely
```

reMESA:

```text
SOURCE
```

AI:

```text
inferred from...
```

reMESA:

```text
SOURCE
```

AI:

```text
UNKNOWN
```

reMESA:

```text
GOOD.
```

And eventually:

```text
AI:
Triangle ESTABLISHED.

reMESA:
AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
```

---

# Thanks

I need to thank **Simon Fenney** for one seemingly small tip that ended
up moving this project forward a lot:

**Intel had its own PowerVR-based graphics drivers through EMGD.**

That sounded like a small lead.

It was not.

Thanks, Simon.

The triangle exists now.

---

## Special thanks: `0x07000345`

For wasting everyone's time and giving me deep dark circles under my eyes.

You were supposed to be a dword.

Somehow you became a character.

```text
0x07000345:
hello

reMESA:
not you again
```

Researchers:

```text
What is 0x07000345?
```

reMESA:

```text
excellent question
```

Researchers:

```text
And?
```

reMESA:

```text
excellent question
```

The triangle was eventually rendered anyway.

Nice try.

---

<details>

<summary><b>📱 Wait... you're developing a GPU driver WHERE?</b></summary>

<br>

On this:

**Samsung Galaxy Tab S7.**

The main development environment is an ARM64 Linux userspace running
through **PRoot on Android**.

Meanwhile the target is:

```text
Dell / Intel Atom machine
        ↓
      i686
        ↓
     Poulsbo
        ↓
    GMA 500
        ↓
     SGX535
        ↓
      2008
```

So yes:

```text
ARM64 Android tablet
        │
      PRoot
        │
      Linux
        │
        ├──── reverse engineering
        ├──── driver development
        ├──── PDS analysis
        ├──── command-stream analysis
        └──── cross compiling
                     │
                     ▼
             Intel Atom Z5xx
                     │
                     ▼
                  SGX535
                     │
                     ▼
                    🔺
```

PRoot used to be:

```console
$ neofetch
```

> look guys ubuntu on my phone

Then it became:

```text
"why does this 2008 PowerVR PDS allocation
have nine dwords with no selected producer"
```

And eventually:

```text
Triangle ESTABLISHED
```

**PRoot is a workstation now.**

bro.

At some point an Android tablet became the development workstation
for reverse engineering a PowerVR GPU inside an Intel Atom machine
from 2008.

Nobody planned this.

```text
Samsung:
We made a tablet.

reMESA:
GPU DRIVER DEVELOPMENT WORKSTATION

Samsung:
what

SGX535:
🔺

Samsung:
WHAT
```

</details>

---

# Project summary

```text
Hardware:       2008
Development:    2026
Documentation:  missing
Registers:      increasingly less mysterious
PDS:            considerably less mysterious
Kernel:         boots
First load:     established
Stock recovery: passed
Gate B:         PASSED
SGX execution:  YES
TA submission:  YES
Rasterization:  YES
FIRE #3:        one invocation
Readback:       sealed
Magenta pixels: 120
Triangle:       ESTABLISHED
Phase 8:        DONE
Phase 9:        MESA
Evidence:       mandatory
Sponsors:       somehow real
Budget:         triangle economy
Other SGXs:     watching nervously
Sanity:         no more
```

The project began as:

```text
maybe I can understand GMA 500
```

Then became:

```text
PowerVR Series5 reverse-engineering project
        │
        ├── real hardware
        ├── reconstructed documentation
        ├── driver development
        ├── collaboration with other SGX developers
        ├── GitHub Sponsors
        ├── kernel negotiations
        └── where triangle
```

Then, at approximately 2:51 AM BRT:

```text
PowerVR Series5 reverse-engineering project
        │
        ├── real hardware
        ├── reconstructed documentation
        ├── driver development
        ├── controlled SGX execution
        ├── TA submission
        ├── rasterization
        └── 🔺
```

Excellent.

Everything is going according to plan.

There was no plan.

---

<div align="center">

## SGX535-reMESA

**Previous objective:**

# 🔺

~~*how hard could three vertices possibly be*~~

**Answer: approximately 20 days.**

---

**Result:**

# 🔺

**120 pixels.**

**Triangle: ESTABLISHED.**

---

**Current technical objective:**

# MESA

*oh right, the actual driver*

---

**Current financial objective:**

# ❤️ → Mesa

*fund increasingly unreasonable old GPU archaeology*

---

**Possible future objective:**

# 🔺 🔺 🔺

*oh god there are more SGXs*

---

**PowerVR Series5 developers:**

Bring evidence.

Bring hardware.

Bring ancient files.

### COME IN.

---

**Want to support the archaeology?**

### ❤️ [GitHub Sponsors](https://github.com/sponsors/gabrieelgama)

*your contribution may be converted into suspiciously old computers*

---

# 🔺

## THE TRIANGLE EXISTS NOW

### PHASE 9 — MESA

*apparently we actually have to write the driver*

</div>