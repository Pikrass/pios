.equ pattern_ok,    0b00011111
.equ pattern_error, 0b00010101
.equ delay, 0x400000

.equ logo_w, 487
.equ logo_h, 211

.data
	.align 2
	welcome: .ascii "Welcome to Pios, the little program wishing to become a "
	         .ascii "full operating system some day. But for now it barely "
	         .ascii "knows how to print a string, so... here it is!\n\n"
	         .asciz "... You can turn off the Pi now."
	logo:    .incbin "logo.bin"

.section .text
.globl _start
.extern led_pattern
.extern term_init
.extern term_create
.extern term_printf

.macro get_color
	mul    r3, r1, r4     // r3 = line * width
	add    r3, r3, r2     // r3 += col
	mla    r3, r3, r6, r7 // r3 = r3 * 3 + logo_base
	ldr    r3, [r3]       // r3 = *r3 (32 bits, but we'll only use the lower 24)
.endm

_start:
	mov    sp, #0x8000

	mov    r0, #1024
	mov    r1, #768
	mov    r2, #24
	bl     fb_request
	cmp    r0, #0
	beq    error

	mov    r1, #1024
	mov    r2, #768
	mov    r3, #3
	bl     term_init

	ldr    r4, =logo_w
	ldr    r5, =logo_h
	mov    r6, #3
	ldr    r7, =[logo]
	mov    r1, #0      // Current line
	line:
		mov    r2, #0  // Current column
		col:
			mov    r3, #0
			cmp    r1, r5
			cmplo  r2, r4
			bhs    store_color

			get_color

			store_color:
			strb   r3, [r0]
			lsr    r3, #8
			strb   r3, [r0, #1]
			lsr    r3, #8
			strb   r3, [r0, #2]

			add    r0, r0, #3
			add    r2, r2, #1
			cmp    r2, #1024
			bne    col
		add      r1, r1, #1
		cmp      r1, r5
		bne      line

	sub    sp, sp, #20
	mov    r1, #100
	mov    r2, #20
	mov    r3, sp
	bl     term_create

	mov    r0, sp
	ldr    r1, =[welcome]
	bl     term_printf
			
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
