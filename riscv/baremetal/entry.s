/* ----------------------------------------------------------------------
--
-- 2026-03-30: Copy hello and remove hello code
--
---------------------------------------------------------------------- */	

	.global _boot_a0
	.global _boot_a1
	.global _uart_base
	.global _entry

/* Align code to 32-bit boundary */
	
	.align 3
	
_entry:
	
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

	/*	la	t2, _payload_start */

/* Hardwire the uart address into _uart_base */
	
/*	lui t0, 0x10010 /* Load 0x10010 into upper 20 bits for sifive_u (uart0) */
	lui	t0, 0x10000 /* Load 0x10000 into upper 20 bits for virt uart = 0x8000000 */
	la	t1, _uart_base
	sw	t0, 0(t1)

/* Set the stack pointer to payload_end + 0x2000 */

	la	t2, _payload_end
	li	t3, 0x2000
	add	sp, t2, t3

/* call loop from C with value of _uart_base in a0 */

	la	a3, _boot_a0
	lw	a0, 0(a3)
	la	a3, _boot_a1
	lw	a1, 0(a3)
	call	_start

/* Horrible hack to shutdown using the sifive,test0 (ays skscon) device */
	
	lui	a3, 0x00100
	li	t0, 0x5555
	sw	t0, 0(a3)
	
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
