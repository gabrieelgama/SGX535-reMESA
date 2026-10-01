HOSTCC := /usr/bin/gcc-14
.PHONY: all
all: fixdep
fixdep: /tmp/sgx535-antix-source-6/source/scripts/basic/fixdep.c
	$(HOSTCC) -O2 -Wall -Wextra -Werror $< -o $@
