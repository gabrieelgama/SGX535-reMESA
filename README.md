<div align="center">

# SGX535-reMESA

### Reverse engineering a PowerVR SGX535 because apparently nobody stopped me.

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

</div>

---

## Current phase

**Phase 7 — PSB DRI reverse engineering / frozen-triangle bring-up**

Current problem:

```text
PDS program:     reproducible
PDS data/state:  pain
GPU:             alive
triangle:        not yet
```

Current focus: **FG-02 — PDS backing-provider proof.**

The historical PDS program can now be reproduced, but some launch-state details still need to be accounted for before doing anything funny on the real GPU.

---

# What is this?

**SGX535-reMESA** is a reverse-engineering and driver-development project for the **PowerVR SGX535**, initially focused on the version inside **Intel Poulsbo / GMA 500** systems.

The objective:

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

The SGX535 documentation needed to write a modern open-source driver is not publicly available in enough detail, so the project is reconstructing it from:

- historical source code
- old drivers
- surviving documentation
- register definitions
- Linux sources
- command/state analysis
- real hardware
- questionable amounts of `grep`

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

> This confirms rev121 on the tested machine. It does not automatically mean every Poulsbo SGX535 ever manufactured is rev121.

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

Making something up because it would make the documentation look more complete is not.

If an unexplained dword works, it remains:

```text
mysterious_dword_that_works
```

until we actually know what it does.

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
Phase 7   ███████████████░░░░░   doing GPU archaeology
Phase 8   ░░░░░░░░░░░░░░░░░░░░   Mesa 💀
```

Things Phase 7 has already produced:

- real hardware MMIO access
- `CORE_ID`
- `CORE_REVISION`
- physical rev121 confirmation
- historical PDS reconstruction
- reproducible PDS program words
- considerably more knowledge about Poulsbo than was probably healthy

Current blocker:

```text
Can we account for the relevant launch state
before telling a 2008 GPU to execute something?
```

Until the answer is sufficiently convincing:

**no funny writes.**

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
- [ ] Finish launch-state reconstruction
- [ ] Controlled SGX bring-up
- [ ] Command submission
- [ ] Make GPU do literally anything useful
- [ ] 🔺 **TRIANGLE**
- [ ] Mesa/Gallium driver
- [ ] OpenGL
- [ ] Minecraft on GMA 500 because it would be funny

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

**three vertices.**

And we will be happy.

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
- someone somehow having the SGX535 programming manual in a drawer since 2009

Most important rule:

> **Evidence first. Guessing second.**

If your conclusion is:

```text
I have absolutely no idea what this register does
```

that's useful information too.

meanwhile
Eurasia.3D Input Parameter Format.1.3.37a.SGX535 1.2.External.pdf

2009: exists
2026: lol no
---

# Disclaimer

This is an independent reverse-engineering and preservation project.

It is not affiliated with or endorsed by Imagination Technologies, Intel, Texas Instruments, Mesa, or the Linux kernel project.

Third-party material remains subject to its respective licenses.

Please do not sue the triangle.

---

# AI usage

AI is used to help with:

- source archaeology
- searching large trees
- comparing code
- organizing findings
- documentation (the AI keeps calling me an "operator" for some reason)
- experimental scaffolding

But:

```text
AI said it
   ≠
hardware documentation
```

Important claims still need evidence from source material, analysis, or actual hardware.

Sometimes the correct AI-assisted research result is still:

```text
UNKNOWN
```

good.

# Thanks 
I need to thank **Simon Fenney** for one seemingly small tip that ended
up moving this project forward a lot:

**Intel had its own PowerVR-based graphics drivers through EMGD.**

That sounded like a small lead.

It was not.

Thanks, Simon.

Special thanks:
0x07000345

for wasting everyone's time
and give me deep dark circles under my eyes.
<details>

<summary><b>📱 Wait... you're developing a GPU driver WHERE?</b></summary>

<br>

On this:

**Samsung Galaxy Tab S7.**

The main development environment is an ARM64 Linux userspace running through **PRoot on Android**.

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

</details>

---

<div align="center">

## SGX535-reMESA

**Current objective:**

# 🔺

*how hard could three vertices possibly be*

</div>
