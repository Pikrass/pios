.equ fb_offset, 540 * 1024 * 3

.section .data
hodor_terminfo: .fill 20, 0x1
hodor_str: .asciz "HODOR "

.section .rodata
irq_handler: ldr    pc, [pc, #0x20]

.section .text
.globl hodor_init

/* Hodor the hodor
 * r0: hodor of the framebuffer
 */
hodor_init:
	push   {lr}
	// Get a terminfo
	ldr    r1, =fb_offset
	add    r0, r0, r1
	mov    r1, #100
	mov    r2, #5
	ldr    r3, =[hodor_terminfo]
	bl     term_create

	// Copy the interrupt handler
	ldr    r0, =irq_handler
	ldr    r0, [r0]
	mov    r1, #0x18
	str    r0, [r1]

	// Copy the address for the handler
	ldr    r0, =hodor_handle
	add    r1, r1, #0x20
	str    r0, [r1]

	// Setup the IRQ stack
	mrs    r0, cpsr
	eor    r1, r0, #0b11111
	orr    r1, r1, #0b10010
	msr    cpsr_c, r1
	mov    sp, #0x4000
	msr    cpsr_c, r0

	// Setup the timer
	ldr    r1, =0x20003004
	ldr    r0, [r1]
	add    r0, r0, #0xf0000
	ldr    r1, =0x20003010
	str    r0, [r1]

	sub    r1, r1, #0x10
	ldr    r0, [r1]
	orr    r0, r0, #0b10
	str    r0, [r1]

	// Set IRQ1
	ldr    r1, =0x2000b210
	mov    r0, #0b10
	str    r0, [r1]

	// Enable IRQs /o\
	mrs    r0, cpsr
	bic    r0, r0, #0x80
	msr    cpsr_c, r0
	pop    {pc}

hodor_handle:
	push   {r0 - r12, lr}

	ldr    r0, =[hodor_terminfo]
	ldr    r1, =[hodor_str]
	bl     term_printf

	// Increment the timer
	ldr    r1, =0x20003010
	ldr    r0, [r1]
	add    r0, r0, #0xf0000
	str    r0, [r1]

	// Clear the interrupt line
	sub    r1, r1, #0x10
	ldr    r0, [r1]
	orr    r0, r0, #0b10
	str    r0, [r1]

	pop    {r0 - r12, lr}
	subs   pc, lr, #4
