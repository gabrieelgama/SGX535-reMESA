<div align="center">

# SGX535-reMESA

### Reverse engineering PowerVR Series5 GPUs because apparently nobody stopped me after the first one.

**Reverse engineering · Linux · Mesa · old GPU suffering**

![Status](https://img.shields.io/badge/status-research%20%2F%20bring--up-orange)
![GPU](https://img.shields.io/badge/GPU-PowerVR%20SGX535-blue)
![Platform](https://img.shields.io/badge/platform-Intel%20Poulsbo-lightgrey)
![Linux](https://img.shields.io/badge/Linux-gma500-yellow)
![Mesa](https://img.shields.io/badge/Mesa-eventually™-purple)

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

## Current phase

**Phase 8 — SGX535 first-load qualification / frozen-triangle bring-up**

*(blame the kernel for this phase existing)*

Current status:

```text
experimental first boot:  userspace reached
privileged capture:       not established
stock recovery:           passed
Gate B:                   BLOCKED
whitelist:                []
SGX execution:            not attempted
triangle:                 not attempted
sanity:                   sudo -v
```

Unfortunately:

```text
"it probably works"
        ≠
"we proved it works"
```

The evidence policy wins again.

damn.

The first experimental boot reached normal userspace, but the privileged
capture did not run. Experimental boot ID, loaded-module identity and
hook trace therefore remain unestablished.

Stock recovery completed successfully and the original graphics state
was verified.

The next step is one separately authorized non-SGX first-load / recovery
cycle using the corrected capture procedure.

No fixed ioctl, SGX execution or triangle submission is currently
authorized.

```text
Experimental kernel:
boots

Evidence:
where

sudo:
authentication required

reMESA:
you have got to be kidding me
```

---

# What is this?

**SGX535-reMESA** is a reverse-engineering and driver-development project
for the **PowerVR SGX535**, initially focused on the version inside
**Intel Poulsbo / GMA 500** systems.

The current objective:

```text
SGX535
   ↓
Linux
   ↓
Mesa
   ↓
  🔺
```

Yes.

We're doing all of this for a triangle.

The SGX535 documentation needed to write a modern open-source driver is
not publicly available in enough detail, so the project is reconstructing
it from:

- historical source code
- old drivers
- surviving documentation
- register definitions
- Linux sources
- command/state analysis
- real hardware
- questionable amounts of `grep`
- files from 2009 that somehow still exist
- pure refusal to let this GPU die

SGX535 / Poulsbo is the current target.

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

# Why?

Poulsbo machines contain an actual **PowerVR SGX535**.

Linux still supports the display side through `gma500`.

3D acceleration, however:

```text
Linux:   display works 👍
SGX535:  hello
Mesa:    who are you
```

There is no modern open-source SGX535 3D driver.

So that's what this project is trying to fix.

Eventually™.

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
| First-load qualification | 🔒 |
| Shader ISA | ❓ |
| Microkernel | ❓ |
| Safe active bring-up | 🔒 |
| Triangle | where |
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

One rule survives every GPU:

> **Evidence first. Guessing second.**
>
> *(This rule becomes extremely annoying when you really want the triangle.)*

---

# Current status

> **This is not a working Mesa driver yet. Do not install it expecting Minecraft.**

```text
Phase 1   ████████████████████   ✓
Phase 2   ████████████████████   ✓
Phase 3   ████████████████████   ✓
Phase 4   ████████████████████   ✓
Phase 5   ████████████████████   ✓
Phase 6   ████████████████████   ✓
Phase 7   ████████████████████   ✓
Phase 8   ███░░░░░░░░░░░░░░░░░   first-load qualification
Phase 9   ░░░░░░░░░░░░░░░░░░░░   Mesa 💀
```

### Phase 8

Also known as:

```text
the phase that exists because the kernel said no
```

Phase 7 already produced:

- real hardware MMIO access
- `CORE_ID`
- `CORE_REVISION`
- physical rev121 confirmation
- historical PDS reconstruction
- reproducible PDS program words
- launch-state evidence reconstruction
- fixed one-shot bring-up tooling
- considerably more knowledge about Poulsbo than was probably healthy

Phase 8 has already produced:

- an experimental first-load kernel
- a successful experimental boot to normal userspace
- a verified recovery to the original stock graphics state
- a new appreciation for `sudo -v`

Current blocker:

```text
The experimental kernel reached userspace.

Did we capture enough evidence from that boot
to prove exactly what was loaded and what happened?

no.
```

The missing experimental boot ID, loaded-module identity and hook trace
cannot be reconstructed from the current stock boot.

So:

```text
Experimental kernel:  boots
Stock recovery:       works
SGX execution:        none
Triangle:             untouched
sudo password:        final boss
```

The next step is another separately authorized non-SGX first-load /
recovery cycle using the corrected capture procedure.

Until that evidence exists:

# no funny writes.

The GPU:

```text
bro you literally booted it
```

reMESA:

```text
DID WE CAPTURE THE TRACE?
```

The GPU:

```text
no
```

reMESA:

```text
THEN NO.
```

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
| Will to live | unknown |

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

**so are we.**

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
- [ ] Capture and qualify experimental first-load evidence
- [ ] Clear Gate B
- [ ] Controlled SGX execution
- [ ] Fixed command submission
- [ ] Make GPU do literally anything useful
- [ ] 🔺 **TRIANGLE**
- [ ] Stare at triangle for an unreasonable amount of time
- [ ] Phase 9
- [ ] Mesa/Gallium driver
- [ ] OpenGL
- [ ] Minecraft on GMA 500 because it would be funny
- [ ] Declare victory
- [ ] Rest
- [ ] Notice another SGX exists
- [ ] Say "hmm"
- [ ] Make another terrible decision
- [ ] Repeat until Series5 stops answering

---

# Phase 9

Assuming Phase 8 eventually stops being held hostage by kernel bring-up:

# Mesa.

Yes.

Actual Mesa.

The thing in the repository name.

```text
Phase 1-7:
reverse engineer GPU

Phase 8:
argue with kernel

Phase 9:
remember why project is called reMESA
```

The rough objective:

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

One problem at a time.

First:

# 🔺

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

The scientific objective is:

```text
            SGX535
               │
               ▼
             Mesa
               │
               ▼
              🔺
```

## THE TRIANGLE

There will be no ray tracing.

There will be no path tracing.

There will probably not even be a texture at first.

There will be:

# three vertices.

And we will be happy.

---

# Wait... there are MORE SGXs?

Unfortunately.

SGX535 / Poulsbo is the current target and remains the main focus.

But the longer-term scope may expand to other **PowerVR Series5 GPUs
and platforms**.

Current situation:

```text
SGX535:     CURRENT VICTIM
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
>
> *(This rule becomes extremely annoying when you really want the triangle.)*

---

# Sponsor the triangle

Yes.

This project has **GitHub Sponsors**.

We have somehow reached the point where an attempt to make three vertices
appear on a PowerVR GPU from 2008 has a funding button.

<div align="center">

### ❤️ [Sponsor SGX535-reMESA](https://github.com/sponsors/gabrieelgama)

**Fund old GPU archaeology.**

**Fund unreasonable amounts of `grep`.**

**Fund the triangle.**

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

Most importantly:

```text
Sponsor:
What does my support get?

reMESA:
More archaeology.

Sponsor:
And the triangle?

reMESA:
Eventually™
```

Please note that sponsoring the project does **not** bypass the evidence
policy.

```text
Sponsor:
I donated.

Evidence policy:
thank you

Sponsor:
Can we send the command now?

Evidence policy:
is the launch state proven?

Sponsor:
...

Evidence policy:
DENIED
```

Money cannot bribe `UNKNOWN` into becoming `CONFIRMED`.

We checked.

The triangle remains subject to peer review.

```text
GitHub Sponsors:
$ ❤️

SGX535:
...

reMESA:
YOU HAVE A BUDGET NOW, DO SOMETHING
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
```

The triangle now has an economy.

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

---

# Frequently asked questions

### Does the driver work?

Not yet.

### Does the GPU work?

Yes.

### Does 3D acceleration work?

We're working on that part.

### Does it support OpenGL?

That is quite literally why we're here.

### Vulkan?

bro.

### OpenGL 4.6?

BRO.

### Ray tracing?

Please observe the current objective:

# 🔺

### Can it run Crysis?

We are currently negotiating with three vertices.

### Can it run Minecraft?

Eventually this question will become extremely funny.

### Why not just use a modern GPU?

Because the modern GPU already has a driver.

Where is the fun in that?

### Why SGX535?

At this point it's personal.

### Why other Series5 GPUs?

At this point it's becoming a collection.

### Can I sponsor the project?

Apparently yes.

### Will sponsoring make the triangle appear faster?

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

It doesn't even exist yet.

If it eventually exists:

please still do not sue the triangle.

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

---

# Thanks

I need to thank **Simon Fenney** for one seemingly small tip that ended
up moving this project forward a lot:

**Intel had its own PowerVR-based graphics drivers through EMGD.**

That sounded like a small lead.

It was not.

Thanks, Simon.

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

Now it's:

```text
"why does this 2008 PowerVR PDS allocation
have nine dwords with no selected producer"
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
First load:     evidence incomplete
Stock recovery: passed
Gate B:         BLOCKED
SGX execution:  not attempted
Triangle:       waiting impatiently
Evidence:       mandatory
sudo -v:        unexpectedly important
Sponsors:       somehow real
Budget:         triangle economy
Other SGXs:     watching nervously
```

The project began as:

```text
maybe I can understand GMA 500
```

It is currently:

```text
PowerVR Series5 reverse-engineering project
        │
        ├── real hardware
        ├── reconstructed documentation
        ├── driver development
        ├── collaboration with other SGX developers
        ├── GitHub Sponsors
        ├── kernel negotiations
        └── still no triangle
```

Excellent.

Everything is going according to plan.

There was no plan.

---

<div align="center">

## SGX535-reMESA

**Current objective:**

# 🔺

*how hard could three vertices possibly be*

---

**Current technical obstacle:**

# KERNEL

*blame it.*

---

**Current financial objective:**

# ❤️ → 🔺

*fund the triangle*

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

**THE TRIANGLE HAS A BUDGET NOW**

</div>