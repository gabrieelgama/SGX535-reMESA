<div align="center">

# SGX535-reMESA Project

### Reconstructing the PowerVR SGX535, one surviving piece of evidence at a time.

**Reverse engineering · Linux · Mesa**

![Status](https://img.shields.io/badge/status-research%20%2F%20bring--up-orange)
![GPU](https://img.shields.io/badge/GPU-PowerVR%20SGX535-blue)
![Platform](https://img.shields.io/badge/platform-Intel%20Poulsbo-lightgrey)
![Linux](https://img.shields.io/badge/Linux-gma500-yellow)
![Mesa](https://img.shields.io/badge/Mesa-long--term%20goal-purple)

> **We don't have the SGX535 programming manual (AND MODERN DRIVER).**
>
> **So we're reconstructing one. (including the driver, YES)**

</div>

---
## Actual phase
**Actual phase:** Phase 7 — PSB DRI reverse engineering / hardware-accelerated frozen-triangle bring-up (FG-02, PDS backing-provider proof).

## 🎯 What is SGX535-reMESA?

**SGX535-reMESA** is a reverse-engineering, documentation and
hardware-preservation project for the **PowerVR SGX535**, initially
focused on the **Intel Poulsbo / GMA 500** implementation.

The long-term objective is slightly ridiculous:

### 🐧 PowerVR SGX535 + Mesa

The original programming documentation required to implement a modern
open-source driver is not publicly available in sufficient detail.

So this project attempts to reconstruct that knowledge from surviving
public source code, historical drivers, documentation and carefully
controlled observations of real hardware.

### REAL TIME UPDATE
The original Poulsbo hardware has now answered directly:

**CORE_ID:** `0x01130000`  
**CORE_REVISION:** `0x00010201`

The observed `CORE_REVISION` decodes to:

- Major: `1`
- Minor: `2`
- Maintenance: `1`

**Yes — the physical Poulsbo SGX535 is rev121. 🎉**

These values were obtained from controlled MMIO observations on original
Intel Poulsbo / GMA 500 hardware.

edit: YESSSSSSSS IT'S REV121
---

## 💡 Why?

Intel Poulsbo systems — including Atom Z5xx machines — contain a
**PowerVR SGX535** GPU.

Linux still supports their display hardware through the `gma500`
DRM/KMS driver.

What it does **not** have is a modern open-source SGX535 3D driver.

> The machines still exist.  
> The GPUs still exist.  
> **The knowledge required to use them should not disappear.**

---

## 🏺 Digital archaeology

Historical investigation has recovered substantially more information
than initially expected.

| Area | Status |
|---|:---:|
| SGX535 register definitions | ✅ |
| Poulsbo DDK target | ✅ |
| SGX535 rev121 historical target | ✅ |
| MMU / BIF | ✅ Partial |
| Power / reset | 🟡 Research |
| Interrupts | 🟡 Research |
| USE / USSE | 🟡 Partial |
| PDS | 🟡 Partial |
| Command submission | 🟡 Partial |
| Shader ISA | ❓ |
| Microkernel | ❓ |
| Safe active hardware bring-up | 🔒 Blocked |
| First triangle | ⏳ |
| Mesa driver | 🌌 Long-term |

Historical evidence includes:

- 📜 `sgx535defs.h`
- 🖥️ `pc_i686_poulsbo_d0_linux`
- 🧩 `services4/system/poulsbo`
- 🧠 SGX535 register definitions
- 🗺️ MMU/BIF information
- ⚙️ USE/USSE and PDS definitions
- 📬 host ↔ SGX communication structures
- 🚚 historical command-submission infrastructure
- 🐧 correlations with Linux `gma500`

---

## 🔎 Evidence policy

> **Evidence beats assumptions.**

Undocumented hardware makes it extremely easy to turn a plausible
guess into fake documentation.

Every important finding is therefore classified as:

| | Classification | Meaning |
|---|---|---|
| 🟢 | **CONFIRMED** | Directly supported by traceable evidence |
| 🟡 | **INFERRED** | Strongly suggested, but not directly established |
| ⚫ | **UNKNOWN** | Insufficient evidence — nothing is invented |

Where possible, claims reference the exact **repository, commit, file,
line range, document revision or hardware observation**.

### `UNKNOWN` is a valid result.

Conflicting historical implementations remain documented as conflicts
until evidence explains them.

---

## 🚦 Current status

**Research / early bring-up**

> ⚠️ **SGX535-reMESA is not yet a functional Mesa driver.**

```text
Phase 1     ████████████████████  ✓
Phase 2     ████████████████████  ✓
Phase 3     ████████████████████  ✓
Phase 4     ████████████████████  ✓
Phase 5     ████████████████████  ✓
Phase 6     ████████████████████  ✓
Phase 7     ██████████████████░░
Phase 8     ░░░░░░░░░░░░░░░░░░░░
````

OBS: phase 8 is mesa
---

## 🖥️ Initial target

|    | Component        | Target             |
| -- | ---------------- | ------------------ |
| 🧩 | Platform         | Intel Poulsbo      |
| 💾 | Chipset          | Intel US15W family |
| 🎮 | Graphics         | Intel GMA 500      |
| 🔷 | GPU              | PowerVR SGX535     |
| ⚙️ | CPU family       | Intel Atom Z5xx    |
| 🐧 | Operating system | Linux              |

> **Important:** historical DDK evidence identifies a Poulsbo target
> configured for **SGX535 rev121**. This does not by itself prove that
> every physical Poulsbo SGX535 contains that revision.

---

## 📚 Documentation

The reconstructed technical reference lives in [`docs/`](docs/).

| Document                                                     | Subject                     |
| ------------------------------------------------------------ | --------------------------- |
| 🧠 [`architecture.md`](docs/architecture.md)                 | SGX architecture            |
| 🗃️ [`registers.md`](docs/registers.md)                      | Register map                |
| 🗺️ [`mmu-bif.md`](docs/mmu-bif.md)                          | MMU / BIF                   |
| 🚚 [`command-submission.md`](docs/command-submission.md)     | Command submission          |
| ⚙️ [`usse.md`](docs/usse.md)                                 | USE / USSE / PDS            |
| 🤖 [`firmware.md`](docs/firmware.md)                         | Firmware / microkernel      |
| 🐧 [`gma500-current-state.md`](docs/gma500-current-state.md) | Modern Linux                |
| 🏺 [`poulsbo-evidence.md`](docs/poulsbo-evidence.md)         | Historical Poulsbo evidence |
| 🧩 [`sgx535-missing-files.md`](docs/sgx535-missing-files.md) | Missing components          |
| ❓ [`unknowns.md`](docs/unknowns.md)                          | Open questions              |

---

## 🗺️ Roadmap

* [x] 🏺 Recover historical SGX535 material
* [x] 🔍 Identify the historical Poulsbo SGX target
* [x] 🧠 Reconstruct major register/MMU/BIF information
* [x] 🧩 Reconstruct Poulsbo-specific integration
* [x] 📚 Build a traceable evidence model
* [x] 🖥️ Begin passive validation on real hardware
* [x] 🔓 Establish the first demonstrably safe SGX register read
* [x] 🔬 Identify the physical SGX revision
* [ ] ⚡ Controlled SGX bring-up
* [ ] 🚚 Command submission
* [ ] 🧪 First userspace GPU workload
* [ ] 🔺 **First triangle**
* [ ] 🐧 Minimal Mesa/Gallium driver
* [ ] 🎮 Expand OpenGL support

---

## 🧬 Why `reMESA`?

After years without modern open-source Linux 3D acceleration, the
ridiculous long-term objective is:

<div align="center">

### SGX535

### ↓

### Mesa

### ↓

### 🔺

**Yes. The triangle.**

</div>

---

## 🤝 Contributing

Historical documentation is extremely valuable.

Contributions involving SGX535, Poulsbo, PSB, EMGD, PowerVR Services,
Mesa/Gallium, ISA research, provenance checking and documentation
translation are welcome.

**Please distinguish evidence from inference.**

---

## ⚖️ Disclaimer

This is an independent preservation and reverse-engineering research
project.

It is not affiliated with or endorsed by Imagination Technologies,
Intel, Texas Instruments, Mesa, or the Linux kernel project.

Third-party source code remains subject to its respective licenses.

## ⚠️ important observation
We currently use AI to assist with some research and documentation creation, but all facts are human-checked.


---

<details>
<summary><b>📱 Wait... where is this being developed?</b></summary>
PRoot Is Not a Neofetch Screenshot Anymore

This project is being developed entirely from a **Samsung Galaxy Tab S7**, running an ARM64 Linux userspace through **PRoot**, with **Anland-Termux/Wayland** on Android.

PRoot used to be the thing you installed to run `neofetch`, take a screenshot, and say:

> “Look, Linux on Android.”

Not anymore.

This environment is now being used for:

- PowerVR SGX535 reverse engineering
- PSB DRI reconstruction
- Linux graphics-driver development
- command-stream and hardware-state analysis
- PDS/USE investigation
- experimental 3D hardware bring-up
- attempting to produce a hardware-accelerated frozen triangle on a GPU from 2008

The host is an **ARM64 Android tablet**.

The target is an **i686 Intel Atom/Poulsbo machine**.

The development environment is **PRoot**.

**PRoot is a workstation now.**

bro.
</details>
