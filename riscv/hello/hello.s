    .global _start

_start:
	
/* Pick one hart to run the main boot sequence */

	la	a3, _hart_lottery
	li	a2, 1
	amoadd.w a3, a2, (a3)
	bnez	a3, _start_hang

/* Look 0x10010 into upper 20 bits */

    lui t0, 0x10010

    andi t1, t1, 0
    addi t1, t1, 72
    sw t1, 0(t0)

    andi t1, t1, 0
    addi t1, t1, 101
    sw t1, 0(t0)

    andi t1, t1, 0
    addi t1, t1, 108
    sw t1, 0(t0)

    andi t1, t1, 0
    addi t1, t1, 108
    sw t1, 0(t0)

    andi t1, t1, 0
    addi t1, t1, 111
    sw t1, 0(t0)

/* linefeed */
    andi t1, t1, 0
    addi t1, t1, 10
    sw t1, 0(t0)

	/* setup stack pointer */

	la	a3, _payload_end
	li	a4, 0x2000
	add	sp, a3, a4

	/* call loop */

	lui a0, 0x10010
	call	loop

finish:
    beq t1, t1, finish

	.globl _start_hang
_start_hang:
	wfi
	j	_start_hang

_hart_lottery:
	.word	0
