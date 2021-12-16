---
title: Porting the luajit dynasm code to RV64I
author: Ross Alexander
date: December 16, 2021
---

# Precis

The RISC-V ISA is now 10 years old and while I majority of C code runs
on Linux on the RV64I instruction set there currently is no support
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

Code  Description
----  -------------------
D     rd (bits [11:7])
N     rs1 (bits [19:15])
M     rs2 (bits [24:20])
I     12-bit immediate (I-type instruction)

Additionally the dynasm instruction IMM_I and STOP needed to be implemented
in dasm_put, dasm_link and dasm_encode.

# BF

Taken from the dasm-a64 example code.  These are the ARMv8 register
allocation.  In ARMv8 the size of the operation (32-bit vs 64-bit) is
based on register name.

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

Registers x19-28 are callee saved registers, s2-11 are the closest
approximation.  Argument registers are a0-a7, with a0 the return
register.  The stack pointer is sp (x2).

~~~
     |.define aPtr, s2
     |.define aState, s3
     |.define aTapeBegin, s4
     |.define aTapeEnd, s5
     |.define TMP, s6
     |.define TMP1, s7
     |.define TMPw, s6
     |.define TMP1w, s6
     |.define cArg1w, a0
     |.define cArg2w, a1
     |.define cRetw, a0
     |.define cArg1, a0
     |.define cArg2, a1
     |.define cRet,  a0
~~~

The original function prologue from the Arm-v8 stores various callee
saved register onto the stack.  This uses the ARMv8 store pair, which
doesn't exist in RV32I or RV64I.

~~~
    |.macro prologue
     | sub sp, sp, #0x40
     | stp aPtr, aState, [sp]
     | stp aTapeBegin, aTapeEnd, [sp, #0x10]
     | stp TMP, TMP1, [sp, #0x20]
     | str lr, [sp, #0x30]
     | mov aState, cArg1
    |.endmacro
~~~

The subi operation does not exist as all immediate values are signed,
so the immediate is negated and added instead.

~~~
    |.macro prologue
     | addi sp, sp, #-0x40
     | sd aPtr, 0(sp)
     | sd aState 8(sp)
     | sd aTapeBegin, 16(sp)
     | sd aTapeEnd, 24(sp)
     | sd TMP, 32(sp)
     | sd TMP1, 40(sp)
     | sd ra, 48(sp)
     | addi aState, cArg1, #0
    |.endmacro
~~~

The load/store instructions (LOAD/STORE) are designed to be as simple
as possible to implement in hardware as possible, so LOAD uses rs1 +
immediate as the source address and stores the result in rd, while the
SAVE uses the same rs1 + immediate but uses rs2 as the source value.

In both cases the immediate value is a 12-bit signed value and the
format of the instruction is n(rx) where n is the immediate and rx is
the indexing register.  All LOAD/STORE instructions have a 3-bit field
for length, giving LB, LH, LW and LD for 8,16,32 and 64 bits
respectively.  All values are sign extended upon load.

The machinary for implementing LD is straight forward, just using the
D for destination and changing the immediate to be [31:20].
