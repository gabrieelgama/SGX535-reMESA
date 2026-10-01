HOSTCC := /usr/bin/gcc-14
.PHONY: all
all: fixdep
fixdep: /tmp/sgx535-antix-source-6/source/scripts/basic/fixdep.c
	$(HOSTCC) -Wall -Wmissing-prototypes -Wstrict-prototypes -O2 -fomit-frame-pointer -std=gnu89 $< -o $@
