	.globl _boot_a0
	.globl _boot_a1
	.global _uart_base
	.global _start

/* Align code to 32-bit boundary */
	
	.align 3


_start:
	
/*
Pick one hart to run the main boot sequence
This requires -march=risc32ia at at minimum for the atomic instructions
*/

	la	a3, _hart_lottery
	li	a2, 1
	amoadd.w a3, a2, (a3)
	bnez	a3, _start_hang

	/* Save a0 and a1 */

	la	a3, _boot_a0
	sw	a0, 0(a3)
	la	a3, _boot_a1
	sw	a1, 0(a3)

/* Hardwire the uart address into _uart_base */
	
/*	lui t0, 0x10010 /* Load 0x10010 into upper 20 bits for sifive_u (uart0) */
	lui	t0, 0x10000 /* Load 0x10000 into upper 20 bits for virt uart */
	la	t1, _uart_base
	sw	t0, 0(t1)

/* Write "hello" to uart */

	li t1, 72
	sw t1, 0(t0)
	li t1, 101
	sw t1, 0(t0)
	li t1, 108
	sw t1, 0(t0)
	li t1, 108
	sw t1, 0(t0)
	li t1, 111
	sw t1, 0(t0)

	/* linefeed */
	li t1, 10
	sw t1, 0(t0)

	/*	la	t2, _payload_start */
/* Set the stack pointer to payload_end + 0x2000 */

	la	t2, _payload_end
	li	t3, 0x2000
	add	sp, t2, t3

/* call loop from C with value of _uart_base in a0 */

	la	a0, _uart_base
	lw	a0, 0(a0)
	call	loop

finish:
    beq t1, t1, finish

	.globl _start_hang
_start_hang:
	wfi
	j	_start_hang

	.align 3

_hart_lottery:
	.word	0
_uart_base:
	.word	0
_boot_a0:
	.word	0
_boot_a1:
	.word	0
