.section .data
_fb_info:
	.align 4
	.int 0 // fb_phy_width
	.int 0 // fb_phy_height
	.int 0 // fb_vir_width
	.int 0 // fb_vir_height
	.int 0 // fb_pitch
	.int 0 // fb_depth
	.int 0 // fb_x
	.int 0 // fb_y
	.int 0 // fb_addr
	.int 0 // fb_size

.section .text
.globl fb_request
.globl fb_draw_image
.extern mailbox_write
.extern mailbox_read

.macro barrier
	mcr    p15, 0, r3, c7, c10, 5
.endm

/* Request a framebuffer from the GPU
 * r0: width
 * r1: height
 * r2: depth (bits/pixel)
 */
fb_request:
	push   {r4, r5, lr}
	ldr    r4, =_fb_info
	str    r0, [r4]
	str    r1, [r4, #4]
	str    r0, [r4, #8]
	str    r1, [r4, #12]
	str    r2, [r4, #20]

	// Convert from virtual to physical
	bic    r5, r4, #0xc0000000
	add    r5, r5, #0x9000

	orr    r0, r5, #1
	barrier
	bl     mailbox_write
	barrier

	mov    r0, #1
	bl     mailbox_read
	barrier
	cmp    r0, #0
	movne  r0, #1
	popne  {r4, r5, pc}

	fb_request$wait_pointer:
	ldr    r0, [r4, #32]
	cmp    r0, #0
	beq    fb_request$wait_pointer

	pop    {r4, r5, pc}

/* Draw an image on screen
 * Assumes the fb width is 1024, and depth is 3 bytes per pixel
 *
 * r0: start address in framebuffer
 * r1: start address of the image
 * r2: width
 * r3: height
 */
fb_draw_image:
	push   {r4 - r9}
	mov    r4, #0 // Current line
	mov    r5, #0 // Current column
	mov    r7, #3
	mov    r8, #1024
	mul    r9, r2, r7

	fb_draw_image$copy:
	ldrb   r6, [r1]
	strb   r6, [r0]
	ldrb   r6, [r1, #1]
	strb   r6, [r0, #1]
	ldrb   r6, [r1, #2]
	strb   r6, [r0, #2]
	add    r1, r1, #3
	add    r0, r0, #3

	add    r5, r5, #1
	cmp    r5, r2
	beq    fb_draw_image$next_line
	b      fb_draw_image$copy

	fb_draw_image$next_line:
	add    r4, r4, #1
	cmp    r4, r3
	beq    fb_draw_image$end
	mov    r5, #0
	sub    r0, r0, r9
	mla    r0, r8, r7, r0
	b      fb_draw_image$copy

	fb_draw_image$end:
	pop    {r4 - r9}
	bx     lr
