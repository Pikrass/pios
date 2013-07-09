.equ pattern_ok,    0b00011111
.equ pattern_error, 0b00010101
.equ delay, 0x400000

.equ logo_w, 487
.equ logo_h, 211
.equ termoffset, logo_h * 1024 * 3

.data
	.align 2
	welcome: .ascii "Welcome to Pios, the little program wishing to become a "
	         .ascii "full operating system some day. But for now it barely "
	         .ascii "knows how to print a string, so... here it is!\n\n"
	         .ascii "This must read 0x1234abcd : 0x%x\n"
	         .ascii "This must read 0x5678 : 0x%x\n"
	         .ascii "This must read 0xbeef : 0x%x\n\n"
	         .asciz "Framebuffer is at : 0x%x"
	logo:    .incbin "logo.bin"

.section .text
.globl _start
.extern led_pattern
.extern fb_request
.extern fb_draw_image
.extern term_init
.extern term_create
.extern term_printf
.extern hodor_init

_start:
	mov    sp, #0x8000
	sub    sp, #24

	mov    r0, #1024
	mov    r1, #768
	mov    r2, #24
	bl     fb_request
	cmp    r0, #0
	beq    error
	str    r0, [sp]

	mov    r1, #1024
	mov    r2, #768
	mov    r3, #3
	bl     term_init


	ldr    r0, [sp]
	ldr    r1, =[logo]
	ldr    r2, =logo_w
	ldr    r3, =logo_h
	bl     fb_draw_image

	ldr    r0, [sp]
	ldr    r1, =termoffset
	add    r0, r0, r1
	mov    r1, #100
	mov    r2, #20
	add    r3, sp, #4
	bl     term_create

	add    r0, sp, #4
	ldr    r1, [sp]
	push   {r1}
	ldr    r1, =0xbeef
	push   {r1}
	ldr    r1, =[welcome]
	ldr    r2, =0x1234abcd
	ldr    r3, =0x5678
	bl     term_printf
	add    sp, sp, #8

	ldr    r0, [sp]
	bl     hodor_init

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
