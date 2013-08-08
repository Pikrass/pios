.section .text
.globl banked_sp_init
banked_sp_init:
	mrs    r0, cpsr
	ldr    r1, =0xffff1000

	// Abort mode
	orr    r0, #0b10111
	msr    cpsr_c, r0
	mov    sp, r1

	// Return in Supervisor mode
	bic    r0, #0b00100
	msr    cpsr_c, r0

	bx     lr


.section .rodata
.globl vectors_start, vectors_end

vectors_start:
	b     _hang   // Reset
	b     _hang   // Undefined instructions
	b     _hang   // Software interrupt
	b     _iabort // Instruction abort
	b     _dabort // Data abort
	b     _hang   // Reserved
	b     _hang   // Interrupt
	b     _hang   // Fast interrupt

_hang:
    b     _hang

_iabort:
	sub    lr, lr, #4
	push   {r0-r12, lr}
	mov    r0, lr
	mrc    p15, 0, r1, c5, c0, 1
	ldr    pc, =inst_abort_handler

_dabort:
	sub    lr, lr, #8
	push   {r0-r12, lr}
	mov    r0, lr
	mrc    p15, 0, r1, c6, c0, 0
	mrc    p15, 0, r2, c5, c0, 0
	ldr    pc, =data_abort_handler

.ltorg

vectors_end:
