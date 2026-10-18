# SGX535-reMESA

<table>
<tr>
<td width="115" valign="middle">
<img src="assets/pipocahello.gif" width="100" alt="Pipoca saying hello">
</td>
<td valign="middle">

**Reverse engineering the PowerVR SGX535 used in Intel Poulsbo / GMA 500.**

Hello! I'm Pipoca, the official mascot of SGX535-reMESA!

Welcome to a project where a mysterious GPU, a 32-bit Atom processor, and an unreasonable amount of assembly somehow became a full reverse-engineering adventure.

</td>
</tr>
</table>

I started working on this on September 16, 2026. The goal is to independently reconstruct the GPU and build a Mesa/Gallium driver for it.

There isn't much useful public documentation for the SGX535, so most of the work so far has involved old drivers, DDK files, register definitions and testing things on real hardware.

## Current status

<img src="assets/pipocajumping.gif" align="right" width="105" alt="Pipoca jumping">

The test machine is a Dell Inspiron Mini 12 with an Atom Z520 and SGX535 rev121.

On October 6 I got the first triangle back from the GPU.

The next test used two indexed triangles and produced a 16x16 square.

So far this is low-level experimental code, not a usable graphics driver.

I'm already working in Mesa/Gallium now. The restricted NIR path creates and retains original shader candidates, with checked program storage, relocations, resource lifetime and draw preparation. An explicit, default-disabled laboratory path connects my NIR-generated fragment shader to the retained diagnostic job while keeping its historical VS/PDS carrier unchanged.

Those paths have passed offline qualification on native Linux, UBSan, i386/QEMU and the actual Mesa-built archive. That is real driver integration work, but it is not a new GPU execution result. My current original VS/PDS pipeline still needs hardware qualification, and I haven't demonstrated an accelerated OpenGL application through it.

I think SGX535-reMESA is getting close to release. That's my development estimate, not a release date: I'm still working on the remaining issues, validation and optimizations. A more detailed update is coming soon.

<br clear="all">

## The historic Git commit — 0x07000345

<img src="assets/pipocaohno.gif" align="right" width="110" alt="Pipoca reacting to 0x07000345">

After roughly 26 days arguing with that DWORD, I gave it a place in Git history too: [070003454e7f938661170abcc57a551ae68c931c](https://github.com/gabrieelgama/SGX535-reMESA/commit/070003454e7f938661170abcc57a551ae68c931c).

Yes, the actual commit SHA starts with `07000345`.

Apparently being a mysterious hexadecimal wasn't enough; now it has a branch too.

The commit preserves the independently written Mesa implementation and its offline qualification work. The hash is a tribute, not evidence of shader execution.

**ALWAYS. 🗿**

<br clear="all">

## Documentation

<img src="assets/pipocaconfering.gif" align="right" width="105" alt="Pipoca checking documentation">

Most of the technical details are in [`docs/`](docs/).

Reproduction notes:

- [Triangle](docs/reproduction/TRIANGLE.md)
- [Square](docs/reproduction/SQUARE.md)
- [Display](docs/reproduction/DISPLAY.md)

The raw results and hashes are in the repository as well. The [current technical update](docs/phase9/DEVELOPMENT-UPDATE.md) separates implementation, offline tests and hardware results; the [full historical triangle README](docs/history/README-TRIANGLE-07000345.md) is preserved separately.

<br clear="all">

## Driver OpenGL

<img src="assets/pipocarunningright.gif" align="right" width="105" alt="Pipoca running right">

My compatibility target is desktop OpenGL 2.1, with OpenGL 3.0 as an experimental research goal. Neither is verified compatibility yet.

Based on my analysis, OpenGL 3.0 may be possible, but I still need evidence for its requirements.

The first games I want to test are: Minecraft, which achieved an impressive average of 30 fps on the original Windows 7 driver, CS 1.6, Super Tux Kart, and browser games.

It will also have Wayland support (except for KDE Plasma; who uses that on an Atom that struggles to open 5 tabs?). In addition to extended driver support, REMESA aims to cover operating systems running on supported 32-bit Intel hardware, but don't expect to see me playing GTA SA on it because it was already a tortoise when it was born; even my cockatiel runs GTA SA faster.

And of course! Everything documented point by point (references, uff).

<br clear="all">

## Development setup

<img src="assets/pipocaidle.gif" align="right" width="105" alt="Pipoca resting">

Most development is done on a Galaxy Tab S7 running Linux through Termux/PRoot. Builds for the Mini 12 are cross-compiled for i686.

It wasn't supposed to become my main development machine but here we are.

<br clear="all">

## Recent Findings — USSE Compiler Reverse Engineering

<img src="assets/pipocathinking.gif" align="right" width="110" alt="Pipoca thinking about USSE instructions">

I've been digging deeper into the original SGX535 userspace shader compiler, trying to understand how it builds and encodes USSE instructions.

What started with investigating the mysterious `0x07000345` word ended up revealing quite a bit more about the compiler itself.

I managed to trace parts of the instruction encoding process, including register field placement, MOV instruction handling, and parts of the compiler's finalization process. This was enough to start writing my own restricted USSE encoder and decoder, rather than relying on proprietary shader binaries.

One of the more recent findings is that assembler flag bit 0 maps to bit 23 of the second encoded instruction word. The mapping is visible in the CPU-side compiler logic, although its exact meaning when executed by the GPU is still unknown.

I also traced the compiler's cleanup callback, stored at offset `+0x630`, through `_mesa_free` to the imported `free` function. The identity-MOV removal routine releases 11 referenced allocations and the instruction node itself. Two additional cleanup lists appear to manage recycled auxiliary records, but I haven't yet established whether their allocations are completely independent of instructions that remain alive.

That historical compiler ownership question remains unresolved. It isn't a prerequisite for qualifying my independently written restricted shader path, which is where I'm focusing now.

At that investigation checkpoint, the independently written codec passed **447 offline test cases** in each of three environments: native Linux, UBSan, and i386/QEMU. Additional evidence checks and symbolic constraint tests have also passed.

This doesn't mean the USSE instruction set is fully decoded. Several important details, including hardware register banks, instruction termination, PDS fetch behavior, and the full meaning of `0x07000345`, still need verification.

For now, the findings are being used to build and validate an original implementation offline. No new GPU shader submissions have been enabled.

The interesting part is that investigating a single 32-bit word has gradually exposed much of the surrounding compiler machinery. There's still a lot to understand, but the picture is getting clearer.

<br clear="all">

## Contributing

<img src="assets/pipocawaitingyou.gif" align="right" width="110" alt="Pipoca waiting for contributors">

If you have old Poulsbo/SGX535 documentation, drivers, hardware, or information about historical DDK releases, feel free to open an issue.

### A note on previous SGX535 driver work

I was recently contacted by email about another project that had reportedly achieved a working Mesa/Gallium driver for Poulsbo.

While looking into the project, I came across references to some files whose origins I couldn't independently verify. One example was `sgxdefs.h`, which appeared to contain over 8,000 lines, compared to the roughly 106-line version included in a historical distribution I had access to.

That difference caught my attention, although it doesn't establish where the larger file came from or whether it was distributed with permission.

For SGX535-reMESA, I'm taking a different approach: independently reconstructing the necessary hardware behavior and writing original driver code, without incorporating proprietary DDK implementations of uncertain provenance.

I'm happy to learn about previous efforts and collaborate where possible, but I want to keep the project's code origins clear and verifiable.

Information about other PowerVR Series5 GPUs is useful too. Our goal is to get to mainline Mesa and contribute suitable mainline kernel patches.

<br clear="all">

## Thanks

Thanks to Simon Fenney for pointing me towards Intel EMGD. That helped a lot.

## Special Thanks

<img src="assets/pipocajumping.gif" align="right" width="110" alt="Pipoca celebrating">

Thanks, `0x07000345`, aka Xpsb_emit_pixel_shader.

I hate you.

20 days of reverse engineering, countless solver runs, one mysterious hexadecimal.

And after all that...

**ALWAYS.**

The real `0x07000345` was the friends we made along the way. 🥀

Edit: A funny fact: I didn't discover error `0x07000345` using the Tab S7, but rather using my cell phone; the final boss couldn't get it. 😂💀

<br clear="all">

## Another thanks

<img src="assets/pipocarunningleft.gif" align="right" width="110" alt="Pipoca running left">

Thank you to everyone who is viewing the project!

Our path will be glorious, always adding humor to documentation that should be at a PhD level.

And of course, without using files with 8,000 lines!

<br clear="all">
