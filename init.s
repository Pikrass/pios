.data
	.align 2
	logo:    .incbin "logo.bin"

.section .text
.globl _start
.globl get_logo
.extern main

_start:
	mov    sp, #0x8000
	bl     main

loop:
	b      loop

get_logo:
	ldr    r0, =logo
	bx     lr
