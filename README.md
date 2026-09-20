<div align="center">

# SGX535-reMESA Project

### Reconstructing the PowerVR SGX535, one surviving piece of evidence at a time.

**Reverse engineering · Hardware preservation · Linux · Mesa**

![Status](https://img.shields.io/badge/status-research%20%2F%20bring--up-orange)
![GPU](https://img.shields.io/badge/GPU-PowerVR%20SGX535-blue)
![Platform](https://img.shields.io/badge/platform-Intel%20Poulsbo-lightgrey)
![Linux](https://img.shields.io/badge/Linux-gma500-yellow)
![Mesa](https://img.shields.io/badge/Mesa-long--term%20goal-purple)

> **We don't have the SGX535 programming manual.**
>
> **So we're reconstructing one.**

</div>

---

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
Source archaeology         ████████████████████  ✓
Poulsbo reconstruction     ████████████████████  ✓
Passive hardware probe     ████████████████████  ✓
Safe MMIO validation       ░░░░░░░░░░░░░░░░░░░░
Active SGX bring-up        ░░░░░░░░░░░░░░░░░░░░
Command submission         ░░░░░░░░░░░░░░░░░░░░
First triangle             ░░░░░░░░░░░░░░░░░░░░
Mesa                       ░░░░░░░░░░░░░░░░░░░░
````

A real Poulsbo machine has reached the passive hardware-inventory
stage. (Dell Inspiron 1210)

**No undocumented MMIO write is considered acceptable simply because
an address looks plausible.**

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
* [ ] 🔓 Establish the first demonstrably safe SGX register read
* [ ] 🔬 Identify the physical SGX revision
* [ ] ⚡ Controlled SGX bring-up
* [ ] 🚚 Command submission
* [ ] 🧪 First userspace GPU workload
* [ ] 🔺 **First triangle**
* [ ] 🐧 Minimal Mesa/Gallium driver
* [ ] 🎮 Expand OpenGL support

---

## 🌎 Language

Most early archaeology and technical documentation is currently written
in **Brazilian Portuguese (pt-BR)**.

🇧🇷 **Português:** current primary documentation
🇬🇧 **English:** planned / contributions welcome

Technical identifiers remain unchanged.

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

<br>

Entirely from a **Samsung Galaxy Tab S7** running an ARM64 Linux
userspace through **PRoot**, using **Anland/Wayland** on Android.

Yes.

The development machine trying to resurrect a 2008 PowerVR GPU
is an Android tablet.

**bro.**

</details>
