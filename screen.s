.section .text
.globl fb_request
.extern mailbox_write
.extern mailbox_read

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

/* Request a framebuffer from the GPU
 * r0: width
 * r1: height
 * r2: depth (bits/pixel)
 */
fb_request:
	push   {r4, lr}
	adr    r4, _fb_info
	str    r0, [r4]
	str    r1, [r4, #4]
	str    r0, [r4, #8]
	str    r1, [r4, #12]
	str    r2, [r4, #20]

	orr    r0, r4, #1
	bl     mailbox_write

	mov    r0, #1
	bl     mailbox_read
	cmp    r0, #0
	popne  {r4, pc}

	fb_request$wait_pointer:
	ldr    r0, [r4, #32]
	cmp    r0, #0
	beq    fb_request$wait_pointer

	pop    {r4, pc}
