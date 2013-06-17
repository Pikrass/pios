.section .rodata
	.equ gpio_ctrl, 0x20200000

.section .text
.globl _start
_start:
	mov    sp, #0x10000

	ldr    r0, =gpio_ctrl

	mov    r1, #1
	lsl    r1, #18
	str    r1, [r0, #4]

	mov    r1, #1
	lsl    r1, #16
	str    r1, [r0, #0x28]

loop:
	b      loop
