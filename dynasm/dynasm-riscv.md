---
title: Porting the luajit dynasm code to RV64I
author: Ross Alexander
date: December 16, 2021
---

# Working Notes

## 2021-12-23

1. Refactor load/save parsing into a single function (parse_load_save)
   to reduce load/saves to a single template entry per load/save size.
   Required fixing DASM_IMM_S handling.  IMM_I and IMM_S are almost
   extractly the same except for the location of the lower 5 bits in
   the instruction.
2. There is still code for VREG but I could not find any instance it
   being used in the luajit code so not really done anything with it.

## 2021-12-18

With the BF interpreter working with the mandelbrot.bf example there is now
a break point.  There are a number of directions to go from here.

1. Remove non RISCV code from dynasm\_riscv64.lua & dynasm\_riscv.h.
2. Add missing instructions.
3. Figure out how to implement LI pseudo-instruction.
4. Add immediate for shift.
5. Note that lui takes a 20-bit unsigned number.
6. Rename registers N & M to 1 & 2 for RS1 & RS2 respectively.  D is
for RD in the R format instructions.  For load it is D <- (RS1+IMM=L).
For store it is 2 -> (RS1+IMM=S).  L & S both have the format of R,
(R) or IMM(R) where -2048 <= IMM < 2048 (signed 12-bit immediate).

# Precis

The RISC-V ISA is now 10 years old and while a majority of Linux C
code runs on the RV64I instruction set there currently is no support
for luajit.

The first thing to migrate luajit is to add RV64I support to dynasm.
As the RV64I ISA is a classic RISC ISA either MIPS64 or AARCH64 (aka
ARM64) is a good base to work from.

## MIPS64 vs ARM64 base

The MIPS64 was chosen by the porters of the Javascript V8 engine.
There is a large amount of documentation around the MIPS64 ISA as it
was the primary RISC architecture for teaching.  However the
availability and long term support for MIPS64 hardware is much less
than ARM64.  As AARCH64 was almost a new ISA rather than an extension
of the existing ARMv7 ISA, with many of its quirks removed, was is a
good starting point.

# Getting started

The original starting point
was [jitdemo](https://github.com/haberman/jitdemo).  This is based
around x86_64 but has an basic C example, a trivial dynasm example and
Brainf*ck interpreter.  This was a good introduction to dynasm but it
was solely for X86.

A critical source was [dasm-a64](https://github.com/zenkj/dasm-a64),
an early port of dynasm to AARCH64 ISA.  This isn't completely
identical to the current ARM64 dynasm now in luajit but was close
enough that the Brainf*ck example was trivial to fix.

# RV64I assembly

The Assembly Programmer's Manual is in some of the unprivileged spec
documents but otherwise can be found
in
[RISC-V Assembly Programmer's Manual](https://github.com/riscv-non-isa/riscv-asm-manual/blob/master/riscv-asm.md).
This contains the register names, assembly forms and pseudo
instruction details.

## RV64I opcodes

The RV64I ISA contains the base RV32I instruction set with 15
additional instructions.  The majority of RISCV implementations that
support Linux are based on the RV64IMAFD.

The RV64I ISA only uses word aligned 32-bit instructions, stored
little endian in memory.  This means the first byte read are the lower
8 bits [7:0].  This contains the opcode prefix [1:0].  For RV64I this
is always 0x3.  The next five bits [6:2] contain the instruction
format.  Depending on the format additional bits may be required for
the final instruction.  All the unprivileged instructions in RV64IMAFD
are use the base [6:0] opcodes.

The encoding used ARM64 for special dynasm instructions does not fit
well with the RV64I encoding.  Instead the custom-0 opcode [0x0b] is
used.  Given the relatively small number od dynasm instructions 5 bits
have been assigned to them, given dynasm special instructions are (ins >> 7) && 0x1f.
This leaves 20 bits of space available for additional fields.

# Examples

## JIT1

Based on the example in jitdemo jit1.c, which involves loading the
return value register with an immediate value, then returning from
the caller.

Due to all instructions being 32-bits long it not possible to load a
full immediate into a register.  With the RV64I arithmetric immediates
are signed 12-bit values.  To load a 32-bit value the LUI [Load Upper
Immediate] instruction takes a 20-bit value and loads it into the
[31:12] bits of the target register, and clears the lower bits.  The
RV64I LUI sign extends the 32-bit result to 64-bits.

RV64I doesn't have a load immediate instruction, instead it relies on
ADDI and register zero (x0).  All 12-bit immediate values are sign
extended before being added.  Below is an example of loading the
return value register (a0) with the immedate value 6, then returning.
Note that ret is a pseudo-instruction for jalr x0, 0(x1).

Because the immediate in the addi instruction is sign extended before
the additional if it is "negative" then the upper immediate value
needs +4096 added to it.  In RV64I assembly the pseudo-instruction
LI takes care of this.

## JIT2

    addi a0, zero, #num
    ret

The resulting action list is

    static const unsigned int actions[4] = {
    0x00000513,
    0x0000050b,
    0x00008067,
    0x0000000b
    };

The final code is below.  The LI instruction is Load Immediate, which
is a pseudo instruction.  Because RV has two varients it is necessary
to specify riscv:rv64 for objdump to recognise RV64I specific
instructions, such as LD or SW (load double word, store double word).

~~~
objdump -D -b binary -mriscv:rv64 /tmp/jitcode

/tmp/jitcode:     file format binary


Disassembly of section .data:

0000000000000000 <.data>:
   0:   00700513                li      a0,7
   4:   00008067                ret
~~~

## Template codes

The above code requires two instruction templates, the first with rs1,
rd and I-type immediate, the second without any operands.

One of the specific design criteria of the RISCV ISA was that the
registers are always in the same place in the instruction and that any
immediates are swirled around them as necessary.

| Code | Description |
| ---- | ------------------- |
| D    | rd (bits [11:7]) |
| N    | rs1 (bits [19:15]) |
| M    | rs2 (bits [24:20]) |
| I    | 12-bit immediate (I-type instruction) |

Additionally the dynasm instruction IMM_I and STOP needed to be implemented
in dasm_put, dasm_link and dasm_encode.

# Brainf*ck

Brainf*ck, reference as BF here after, is an esoteric language.  It is
based on an string of commands and a "tape", normally of 8-bit cells,
and a current cell.


| Char | Description |
| ---- | --- |
| >    | Move to next cell on the right |
| <    | Move to next cell of the left |
| +    | Increase value of current cell by 1 |
| -    | Decrease value of current cell by 1 |
| .    | Output value of current cell as a character |
| ,    | Accept character from input and put in current cell |
| [    | If cell is zero then jump past matching ] |
| ]    | If cell is non-zero, jump back to after matching [ |


The original dynasm example took a BF interpreter and converted it to
X86 dynasm.  The dasm-a64 took that example and converted it to ARM64.
The jit3.dasc example also has an X86 BF implementation but is simpler
in design.  A new example was created combining various features of
both programs.

1. A state structure was created with tape pointer, put_ch and get_ch
   function pointers.  Trying to call a function directly from dynasm
   in ARM64 or RV64I is problematic but calling a function pointer
   which is a member of structure pointed at by a register is straight
   forward.

2. X86 and ARM64 code was split out into .if and .endif blocks.  The
   Makefile was modified to create machine specific .h files an
   executables (jit5-x86 and jit5-a64 respectively).

3. Additional code was added to support both passing the bf program as
   a string on the command line and passing a file path.  The original
   jit3 example uses argv[1] as the program code but this caused
   problems with the mandelbrot.bf program.


Taken from the dasm-a64 example code.  These are the ARMv8 register
allocation.  In ARMv8 the size of the operation (32-bit vs 64-bit) is
based on register name.  Some of the registers, such as aTapeBegin,
aTapeEnd, TMP1, cRet and cRetw, ended up not being used.

~~~
     |.define aPtr, x19
     |.define aState, x20
     |.define aTapeBegin, x21
     |.define aTapeEnd, x22
     |.define TMP, x23
     |.define TMP1, x24
     |.define TMPw, w23
     |.define TMP1w, w24
     |.define cArg1w, w0
     |.define cArg2w, w1
     |.define cRetw, w0
     |.define cArg1, x0
     |.define cArg2, x1
     |.define cRet, x0
~~~

The ARM64 registers x19-28 are callee saved registers, the RV64 s2-11
are the closest approximation.  In RV64 the argument registers are
a0-a7, with a0 the return register.  The stack pointer is sp (x2).  It
should be noted that in the ISA the registers are x0-x31 but in GNU as
the ABI names should be used.

~~~
     |.define aState, s2
     |.define aPtr, s3
     |.define TMP, t1
     |.define cArg1, a0
     |.define cArg2, a1
     |.define cRet,  a0
~~~

The original function prologue from the Arm-v8 stores various callee
saved register onto the stack.  The original used the ARMv8 store
pair, which doesn't exist in RV32I or RV64I, so str and ldr are used
instead, to keep the code as similar as possible.

~~~
    |.macro prologue
     | sub sp, sp, #32
     | str aPtr, [sp, #16]
     | str aState,[sp, #8]
     | str lr, [sp, #0]
     | mov aState, cArg1
     | ldr aPtr, state->tape
    |.endmacro
~~~

The subi operation does not exist as all immediate values are signed,
so the immediate is negated and added instead.  The mv opcode is a
pseudo-instruction for addi rd, rs1, #0.  Code for the state->tape
needed to be modified.  This emits an IMM_I dynasm opcode.

~~~
    |.macro prologue
     | addi sp, sp, #-32
     | sd aPtr, 16(sp)
     | sd aState 8(sp)
     | sd ra, 0(sp)
     | mv aState, cArg1
	 | ld aPtr, state->tape
    |.endmacro
~~~

The load/store instructions (LOAD/STORE) are designed to be as simple
as possible to implement in hardware, so LOAD uses rs1 + immediate as
the source address and stores the result in rd, while the SAVE uses
the same rs1 + immediate but uses rs2 as the source value.  Because of
this the immediate bits are swirled around and split between [31:25]
and [11:7].

In both cases the immediate value is a 12-bit signed value and the
format of the instruction is n(rx) where n is the immediate and rx is
the indexing register.  All LOAD/STORE instructions have a 3-bit field
for length, giving LB, LH, LW and LD for 8,16,32 and 64 bits
respectively.  All values are sign extended upon load.  There are
corresponding unsigned load instructions, which do not do the sign
extension.

The machinary for implementing LD is straight forward, just using the
D for destination and changing the immediate to be [31:20].

Below is a decode from the generated output.

~~~
   0:   fe010113                addi    sp,sp,-32
   4:   01313823                sd      s3,16(sp)
   8:   01213423                sd      s2,8(sp)
   c:   00113023                sd      ra,0(sp)
  10:   00050913                mv      s2,a0
  14:   00093983                ld      s3,0(s2)
  18:   01013983                ld      s3,16(sp)
  1c:   00813903                ld      s2,8(sp)
  20:   00013083                ld      ra,0(sp)
  24:   02010113                addi    sp,sp,32
  28:   00008067                ret
~~~

## Get and put characters

The next two bf commands implemented were , and ., which reads and
writes a character (8-bit only) from and to stdin and stdout
respectively.

In ARM64 these are

    | ldrb cArg2w, [aPtr]
    | mov cArg1, aState
    | ldr TMP, state->put_ch
    | blr TMP

and

    | mov cArg1, aState
    | ldr TMP, state->get_ch
    | blr TMP
    | strb cRetw, [aPtr]

for put_ch and get_ch respectively.  In RV64I an unconditional jump to
an address in a register is Jump And Link Register [JALR].  This jumps
to the address in rs1 + IMM12 while putting the return address (PC+4)
into the register specified in rd, normally ra (x1).  Only jalr_1 was
implemented, where the immediate value is set to 0 and rd is defaulted
to x1.

    |  lb cArg2, (aPtr)
    |  mv cArg1, aState
    |  ld TMP, state->put_ch
    |  jalr TMP

    | mv cArg1, aState
    | ld TMP, state->get_ch
    | jalr TMP
    | sb cRet, (aPtr)

For the test program the + command is also implemented to confirm the
cell is being written to, updated and then re-read.

    | ld TMP, (aPtr)
    | addi TMP, TMP, #1
    | sb TMP, (aPtr)

With the following test program

~~~
echo 'X' | ./jit5-r64 ',+.' ; echo
Y
~~~

the following code (with prologue and epilogue removed) as generated.

~~~
  1c:   01093303                ld      t1,16(s2)
  20:   000300e7                jalr    t1
  24:   00a98023                sb      a0,0(s3)
  28:   0009b303                ld      t1,0(s3)
  2c:   00130313                addi    t1,t1,1
  30:   00698023                sb      t1,0(s3)
  34:   00098583                lb      a1,0(s3)
  38:   00090513                mv      a0,s2
  3c:   00893303                ld      t1,8(s2)
  40:   000300e7                jalr    t1
  44:   01013983                ld      s3,16(sp)
~~~

The <, > and - commands straight forward and didn't require any
additional instructions, just leaving just [ and ].  The jit3.dasc
implementation using dynamic labels and array as stack very clean.
The ARM64 implementation was simple.

Branching in RV64I is paradoxically both simpler and more complex.
The RV32I/RV64I ISA does not have condition codes, instead it has
fused check and branch instructions, where rs1 and rs2 and the check
are encoded into the branch, along with a 12-bit immediate.  The
immediate is a left shifted offset to the PC, allowing a branch with
+/- 4K of the instruction.

This lead to an issue where small bf programs ran successfully but
larger programs, such as mandelbrot, failed.  To work around this
limitation there is a Jump and Link [JAL] instruction, which has a
20-bit immediate.  This immediate is again left shifted one to allow
jumps to addresses +1/- 1MB from PC.  The branch is then reversed to
skip over the unconditional branch.  Fortunately the placement of the
dynamic labels meant no additional labels were required.


    | lb TMP, (aPtr)
    // | beq TMP, zero, =>(maxpc-2)
    | bne TMP, zero, =>(maxpc-1)
    | jal zero, =>(maxpc-2)
    |=>(maxpc-1):

and

    | lb TMP, (aPtr)
    // | bne TMP, zero, =>(*top-1)
    | beq TMP, zero, =>(*top-2)
    | jal zero, =>(*top-1)
    |=>(*top-2):

This BNE and JAL instructions required two additional dynasm
instruction, PC_REL_B and PC_REL_J.  Each of these finds the relative
instruction offset (in bytes) then encodes them in the B and J
immediate format.  These formats have an number of bits swirled to
reduce the number of multiplexors required in the data path.

# Testing on HiFive Unmatched from SiFive

The test platform was a HiFive Unmatched board from SiFive, which has
a U74-MC core, with four U74 cores running @ 1.2 GHz, 32KB I-cache and
2MB L2-cache.  A second test platform was running QEMU 6.1 on an AMD
Ryzen 5 2600X @ 3.6 GHz.

Because luajit cannot run natively instead lua 5.1.5 + luabitop 1.0.2
was manually compiled and installed.

| System              | Time      | Code Size |
| ---                 | ---       | ---       |
| Ryzen 5 2600X (X64) | 0m2.533s  | 42528     |
| QEMU (RV64I)        | 0m5.939s  | 67140     | 
| Unmatched           | 3m10.040s | 67140     |
| QEMU (AARCH64)      | 0m6.664s  | 67140     |

I'm not sure why the unmatched is so much slower than it running under
emulation.  It might be an I-cache issue given the Ryzen has 32KB
I-cache (per core), 3MB L2 cache and 32MB of L3 cache.

# Luajit

## Add architecture defines

Confirm which defines are set to determine the target is RISCV.

~~~
[root@stage4 jitdemo]# gcc -E -dM defines.c | grep -i riscv
#define __riscv 1
#define __riscv_atomic 1
#define __riscv_cmodel_medlow 1
#define __riscv_fdiv 1
#define __riscv_float_abi_double 1
#define __riscv_mul 1
#define __riscv_muldiv 1
#define __riscv_xlen 64
#define __riscv_fsqrt 1
#define __riscv_m 2000000
#define __riscv_a 2000000
#define __riscv_c 2000000
#define __riscv_d 2000000
#define __riscv_f 2000000
#define __riscv_i 2000000
#define __riscv_zicsr 2000000
#define __riscv_compressed 1
#define __riscv_flen 64
#define __riscv_arch_test 1
#define __riscv_div 1
~~~

In lj_arch.h

Add new ARCH and check.

~~~
#define LUAJIT_ARCH_RISCV64     8
#define LUAJIT_ARCH_riscv64     8

#elif defined(__riscv) && (__riscv_xlen == 64)
#define LUAJIT_TARGET  LUAJIT_ARCH_RISCV64
~~~

## Registers

~~~
|// ARM64 registers and the AAPCS64 ABI 1.0 at a glance:
|//
|// x0-x17 temp, x19-x28 callee-saved, x29 fp, x30 lr
|// x18 is reserved on most platforms. Don't use it, save it or restore it.
|// x31 doesn't exist. Register number 31 either means xzr/wzr (zero) or sp,
|// depending on the instruction.
|// v0-v7 temp, v8-v15 callee-saved (only d8-d15 preserved), v16-v31 temp
|//
|// x0-x7/v0-v7 hold parameters and results.
~~~

For RISCV a0-a7, t0-t6 are temp, s0-s12 are callee-saved, s0 [x8] fp,
x1 lr, x2 sp.  x0 is always zero.

fs0-fs11 are saved FP registers.  ft0-ft11 are FP temporaries.

a0-x7 / fa0-fa7 hold arguments / results.

~~~
|// The following must be C callee-save.
|.define BASE,		x19	// Base of current Lua stack frame.
|.define KBASE,		x20	// Constants of current Lua function.
|.define PC,		x21	// Next PC.
|.define GLREG,		x22	// Global state.
|.define LREG,		x23	// Register holding lua_State (also in SAVE_L).
|.define TISNUM,	x24	// Constant LJ_TISNUM << 47.
|.define TISNUMhi,	x25	// Constant LJ_TISNUM << 15.
|.define TISNIL,	x26	// Constant -1LL.
|.define fp,		x29	// Yes, we have to maintain a frame pointer.

|.define ST_INTERP,	w26	// Constant -1.
~~~

For RV64I use s2-s9 and s0 for fp.


