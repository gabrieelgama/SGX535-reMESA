cmd_scripts/module.lds := /home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/bin/i686-linux-gnu-gcc-14 --sysroot=/home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root -E -Wp,-MMD,scripts/.module.lds.d -nostdinc -isystem /home/gama/sgx535-offline/gcc14-i686-qualification-20260930/root/usr/bin/../lib/gcc-cross/i686-linux-gnu/14/include -I/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/arch/x86/include -I./arch/x86/include/generated -I/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/include -I./include -I/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/arch/x86/include/uapi -I./arch/x86/include/generated/uapi -I/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/include/uapi -I./include/generated/uapi -include /home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/include/linux/kconfig.h -D__KERNEL__ -fmacro-prefix-map=/home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/=   -I /home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/scripts -I ./scripts -P -Ux86 -D__ASSEMBLY__ -DLINKER_SCRIPT -o scripts/module.lds /home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/scripts/module.lds.S

source_scripts/module.lds := /home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/scripts/module.lds.S

deps_scripts/module.lds := \
    $(wildcard include/config/lto/clang.h) \
  /home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/include/linux/kconfig.h \
    $(wildcard include/config/cc/version/text.h) \
    $(wildcard include/config/cpu/big/endian.h) \
    $(wildcard include/config/booger.h) \
    $(wildcard include/config/foo.h) \
  arch/x86/include/generated/asm/module.lds.h \
  /home/gama/sgx535-offline/antix-kbuild-preparation-20260930/source-unpack/linux-5.10.240-antix.1-486-smp/include/asm-generic/module.lds.h \

scripts/module.lds: $(deps_scripts/module.lds)

$(deps_scripts/module.lds):
