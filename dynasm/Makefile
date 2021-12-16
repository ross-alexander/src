LUA=luajit
CFLAGS=-O3 -g -std=gnu11

MACH=$(shell uname -m)

# ----------------------------------------------------------------------
# Automagically select correct setting based on machine type (uname -m)
# ----------------------------------------------------------------------

ifeq ($(MACH), x86_64)
	DYNASM_DEFS=-D X64
	CFLAGS += -DX64
	SUFFIX = x64
	JIT_HEADER = dynasm/dasm_x86.h
	DYNASM_LUA = dynasm/dasm_x64.lua
endif
ifeq ($(MACH), aarch64)
	DYNASM_DEFS=-D AARCH64
	CFLAGS += -DAARCH64
	SUFFIX = a64
	JIT_HEADER = dynasm/dasm_arm64.h
	DYNASM_LUA = dynasm/dasm_arm64.lua
endif
ifeq ($(MACH), riscv64)
	DYNASM_DEFS=-D RISCV64
	CFLAGS += -DRISCV64
	SUFFIX = r64
	JIT_HEADER = dynasm/dasm_riscv64.h
	DYNASM_LUA = dynasm/dasm_riscv64.lua
	LUA=lua
endif


all: jit1 jit2-$(SUFFIX) jit3 jit4 jit5-$(SUFFIX)

jit1: jit1.c

jit2-$(SUFFIX): dynasm-driver.c jit2-$(SUFFIX).h dynasm/dasm_riscv64.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ dynasm-driver.c -DJIT=\"jit2-$(SUFFIX).h\"
jit2-$(SUFFIX).h: jit2.dasc
	$(LUA) dynasm/dynasm.lua $(DYNASM_DEFS) $< > $@

jit3: dynasm-driver.c jit3.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o jit3 dynasm-driver.c -DJIT=\"jit3.h\"
jit3.h: jit3.dasc
	$(LUA) dynasm/dynasm.lua jit3.dasc > jit3.h


jit4: dynasm-driver.c jit4.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ dynasm-driver.c -DJIT=\"jit4.h\"

jit4.h: jit4.dasc
	$(LUA) dynasm/dynasm.lua $(DYNASM_DEFS) $< > $@


jit5-$(SUFFIX): dynasm-driver.c jit5-$(SUFFIX).h $(JIT_HEADER)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ dynasm-driver.c -DJIT=\"jit5-$(SUFFIX).h\"

jit5-$(SUFFIX).h: jit5.dasc $(DYNASM_LUA)
	$(LUA) dynasm/dynasm.lua $(DYNASM_DEFS) $< > $@

jit2-rv: jit2-rv.c
	/opt/riscv/bin/riscv64-unknown-linux-gnu-gcc $< -o $@

jit2-rv.c : jit2-rv.dasc
	$(LUA) dynasm/dynasm.lua $< > $@
