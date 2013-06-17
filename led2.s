.section .rodata
	.equ gpio_ctrl, 0x20200000
	.equ wait_iters, 0x29b000

.section .text
.globl _start
_start:
	mov    sp, #0x10000

	ldr    r4, =gpio_ctrl

	mov    r5, #1
	lsl    r5, #18
	str    r5, [r4, #0x04]

	mov    r5, #1
	lsl    r5, #16

loop:
	str    r5, [r4, #0x28]
	bl     wait
	str    r5, [r4, #0x1c]
	bl     wait
	b      loop


wait:
	ldr    r0, =wait_iters
	wait$1:
	sub    r0, r0, #1
	cmp    r0, #0
	bne    wait$1
	bx     lr
