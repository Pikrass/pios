.section .rodata
	.equ pattern, 0b00010101
	.equ delay, 0x400000

.section .text
.globl _start
.extern led_pattern
_start:
	mov    sp, #0x10000

	ldr    r0, =pattern
	mov    r1, #8
	ldr    r2, =delay
	mov    r3, #0
	bl     led_pattern

loop:
	b      loop
