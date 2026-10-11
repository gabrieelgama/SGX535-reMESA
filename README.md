# SGX535-reMESA

Reverse engineering the PowerVR SGX535 used in Intel Poulsbo / GMA 500.

I started working on this on September 16, 2026. The goal is to document enough of the GPU to eventually draft a Mesa driver for it.

There isn't much useful public documentation for the SGX535, so most of the work so far has involved old drivers, DDK files, register definitions and testing things on real hardware.

## Current status

The test machine is a Dell Inspiron Mini 12 with an Atom Z520 and SGX535 rev121.

On October 6 I got the first triangle back from the GPU.

The next test used two indexed triangles and produced a 16x16 square.

So far this is low-level experimental code, not a usable graphics driver.

I'm working on 3D next, then I'll start moving this towards Mesa.

## Documentation

Most of the technical details are in [`docs/`](docs/).

Reproduction notes:

- [Triangle](docs/reproduction/TRIANGLE.md)
- [Square](docs/reproduction/SQUARE.md)
- [Display](docs/reproduction/DISPLAY.md)

The raw results and hashes are in the repository as well.

## Driver OpenGL
The driver under development will support OpenGL 2.1 Desktop and OpenGL 3.0 Experimental. 
Based on my analysis, the SGX535 likely supports OpenGL 3.0. 

## Development setup

Most development is done on a Galaxy Tab S7 running Linux through Termux/PRoot. Builds for the Mini 12 are cross-compiled for i686.

It wasn't supposed to become my main development machine but here we are.

## Recent Findings — USSE Compiler Reverse Engineering

I've been digging deeper into the original SGX535 userspace shader compiler, trying to understand how it builds and encodes USSE instructions.

What started with investigating the mysterious "0x07000345" word ended up revealing quite a bit more about the compiler itself.

I managed to trace parts of the instruction encoding process, including register field placement, MOV instruction handling, and parts of the compiler's finalization process. This was enough to start writing my own restricted USSE encoder and decoder, rather than relying on proprietary shader binaries.

One of the more recent findings is that assembler flag bit 0 maps to bit 23 of the second encoded instruction word. The mapping is visible in the CPU-side compiler logic, although its exact meaning when executed by the GPU is still unknown.

I also traced the compiler's cleanup callback, stored at offset "+0x630", through "_mesa_free" to the imported "free" function. The identity-MOV removal routine releases 11 referenced allocations and the instruction node itself. Two additional cleanup lists appear to manage recycled auxiliary records, but I haven't yet established whether their allocations are completely independent of instructions that remain alive.

I'm currently tracing allocation ownership and instruction lifetimes to resolve that question.

The independently written codec has passed 447 offline test cases in each of three environments: native Linux, UBSan, and i386/QEMU. Additional evidence checks and symbolic constraint tests have also passed.

This doesn't mean the USSE instruction set is fully decoded. Several important details, including hardware register banks, instruction termination, PDS fetch behavior, and the full meaning of "0x07000345", still need verification.

For now, the findings are being used to build and validate an original implementation offline. No new GPU shader submissions have been enabled.

The interesting part is that investigating a single 32-bit word has gradually exposed much of the surrounding compiler machinery. There's still a lot to understand, but the picture is getting clearer.

## Contributing

If you have old Poulsbo/SGX535 documentation, drivers, hardware, or information about historical DDK releases, feel free to open an issue.

A note on previous SGX535 driver work

I was recently contacted by email about another project that had reportedly achieved a working Mesa/Gallium driver for Poulsbo.

While looking into the project, I came across references to some files whose origins I couldn't independently verify. One example was "sgxdefs.h", which appeared to contain over 8,000 lines, compared to the roughly 106-line version included in a historical distribution I had access to.

That difference caught my attention, although it doesn't establish where the larger file came from or whether it was distributed with permission.

For SGX535-reMESA, I'm taking a different approach: independently reconstructing the necessary hardware behavior and writing original driver code, without incorporating proprietary DDK implementations of uncertain provenance.

I'm happy to learn about previous efforts and collaborate where possible, but I want to keep the project's code origins clear and verifiable.
Information about other PowerVR Series5 GPUs is useful too, Our goal is to get to the mainline MESA and Mainline Kernel Patch. 

## Thanks

Thanks to Simon Fenney for pointing me towards Intel EMGD. That helped a lot.
## Special Thanks 
Thanks, "0x07000345", aka Xpsb_emit_pixel_shader I hate you, 20 days of reverse engineering, countless solver runs, one mysterious hexadecimal. And after all that... ALWAYS. The real "0x07000345" was the friends we made along the way 🥀
edit: A funny fact: I didn't discover error 0x07000345 using the Tab S7, but rather using my cell phone; the final boss couldn't get it😂💀
