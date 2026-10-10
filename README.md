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

## Development setup

Most development is done on a Galaxy Tab S7 running Linux through Termux/PRoot. Builds for the Mini 12 are cross-compiled for i686.

It wasn't supposed to become my main development machine but here we are.

## Contributing

If you have old Poulsbo/SGX535 documentation, DDK files, drivers or hardware, feel free to open an issue.

Information about other PowerVR Series5 GPUs is useful too.

## Thanks

Thanks to Simon Fenney for pointing me towards Intel EMGD. That helped a lot.
## Special Thanks 
Thanks 0x07000345 aka Xpsb_emit_pixel_shader i hate you
20 days of reverse engineering.
Countless solver runs.
One mysterious hexadecimal.
And after all that...
ALWAYS
the real 0x07000345 is the friends we made along the way🥀