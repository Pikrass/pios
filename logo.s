.section .data
	logo: .incbin "logo.bin"

.section .text
.globl get_logo

get_logo:
	ldr    r0, =logo
	bx     lr
