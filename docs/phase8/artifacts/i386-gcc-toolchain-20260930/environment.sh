# Source only for the isolated OFFLINE build environment; no target operations.
# Global packages are unchanged. HOSTCC is native; CC/CROSS_COMPILE/LD/AS are target tools.
export PATH=/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/bin:/usr/bin:/bin
export LD_LIBRARY_PATH=/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/lib/aarch64-linux-gnu
export BISON_PKGDATADIR=/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/share/bison
export M4=/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/bin/m4
export LC_ALL=C
export HOSTCC=/usr/bin/gcc-14
export CC='/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/bin/i686-linux-gnu-gcc-14 --sysroot=/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root'
export CROSS_COMPILE=/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/bin/i686-linux-gnu-
export LD=/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/bin/i686-linux-gnu-ld.bfd
export AS=/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/bin/i686-linux-gnu-as
export HOSTCFLAGS='-I/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/include -I/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/include/aarch64-linux-gnu'
export HOSTLDFLAGS=-L/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/lib/aarch64-linux-gnu
