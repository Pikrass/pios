.equ char_w, 10 // if modified, font_draw_char must be modified accordingly
.equ char_h, 16
.equ char_line, char_w * 3
.equ img_w, 940 * 3

.data
font_img: .incbin "font.bin"

.section .text
.globl font_draw_char

/* Get the starting address of the specified char
 * r0: ascii char
 */
font_get_char:
	// Select only the first 8 bits
	and    r0, r0, #0xff

	// Check if 0x21 <= r0 <= 0x7e: if not, return the address of '?'
	cmp    r0, #0x21
	movlo  r0, #0x3f
	cmp    r0, #0x7e
	movhi  r0, #0x3f

	// Compute the address
	mov    r1, #char_line
	ldr    r2, =[font_img]
	sub    r0, r0, #0x21
	mla    r0, r0, r1, r2
	bx     lr

/* Copy the character image to a framebuffer
 * Assumes a color depth of 3 bytes.
 *
 * r0: base address of the framebuffer
 * r1: number of columns in the framebuffer
 * r2: ascii char
 */
font_draw_char:
	push   {r4, r5, r6 - r10, lr}
	mov    r4, r0
	mov    r3, #3
	mul    r5, r1, r3

	mov    r0, r2
	bl     font_get_char

	mov    r1, #char_h

	font_draw_char$copy:
	ldmia  r0!, {r2, r3, r6 - r10} // Copy 28 bytes
	stmia  r4!, {r2, r3, r6 - r10}
	ldrh   r2, [r0]                // Copy 2 bytes
	strh   r2, [r4]
	sub    r0, r0, #28
	sub    r4, r4, #28
	ldr    r2, =img_w
	add    r0, r0, r2
	add    r4, r4, r5
	subs   r1, r1, #1
	bne    font_draw_char$copy

	pop    {r4, r5, r6 - r10, pc}
