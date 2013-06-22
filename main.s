.section .rodata
	.equ pattern_ok,    0b00011111
	.equ pattern_error, 0b00010101
	.equ delay, 0x400000

.section .text
.globl _start
.extern led_pattern
_start:
	mov    sp, #0x10000

	mov    r0, #1024
	mov    r1, #768
	mov    r2, #24
	bl     fb_request
	cmp    r0, #0
	beq    error

	mov    r1, #768
	mov    r3, #0   // Color
	mov    r5, #0   // Iterator for the color change
	line:
		cmp    r5, #3
		moveq  r5, #0
		addeq  r3, #1
		addne  r5, r5, #1

		mov    r2, #1024
		col:
			strb   r3, [r0]
			strb   r3, [r0, #1]
			strb   r3, [r0, #2]
			add    r0, r0, #3
			subs   r2, r2, #1
			bne    col
		subs     r1, r1, #1
		bne      line
			
	ldr    r0, =pattern_ok
	mov    r1, #8
	ldr    r2, =delay
	mov    r3, #0
	bl     led_pattern

	error:
	ldr    r0, =pattern_error
	mov    r1, #8
	ldr    r2, =delay
	mov    r3, #0
	bl     led_pattern

loop:
	b      loop
