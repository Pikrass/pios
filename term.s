.equ char_wb, 10 * 3
.equ char_hb, 16 * 3

.data
.align 4
_term_fb_start: .word
_term_fb_w:     .word
_term_fb_h:     .word
_term_depth:    .word

.section .text
.globl term_init
.globl term_create
.globl term_print_char
.globl term_printf

/* Initialise the terminal module.
 * r0: start address of the framebuffer
 * r1: framebuffer width in pixels
 * r2: framebuffer height in pixels
 * r3: depth (bytes per pixel)
 */
term_init:
	push   {r4, lr}
	ldr    r4, =[_term_fb_start]
	stmia  r4, {r0 - r3}
	pop    {r4, pc}

/* Allocate a screen area to draw text
 * No check is performed: this function is meant to be wrapped in the
 * future
 *
 * r0: start address in the framebuffer
 * r1: requested width (in characters)
 * r2: requested height (in characters)
 * r3: address of a 5 words buffer (terminfo)
 */
term_create:
	str    r0, [r3]      // base
	str    r1, [r3, #4]  // width
	str    r2, [r3, #8]  // height
	mov    r0, #0
	str    r0, [r3, #12] // x
	str    r0, [r3, #16] // y
	bx     lr

/* Print a character in the next position of the terminal
 * r0: terminfo address
 * r1: ascii char
 */
term_print_char:
	push   {r4-r8, lr}
	mov    r4, r0
	mov    r2, r1

	ldr    r0, [r4]
	ldr    r5, [r4, #12]         // x
	ldr    r6, [r4, #16]         // y
	ldr    r1, =[_term_fb_start]
	ldr    r1, [r1, #4]          // _term_fb_w

	// r0 = base + y * char_hb * term_w + x * char_wb
	mov    r7, #char_wb
	mul    r7, r5, r7
	mov    r8, #char_hb
	mul    r8, r6, r8
	mla    r0, r8, r1, r0
	add    r0, r0, r7

	bl     font_draw_char

	mov    r0, r4
	bl     term_cursor_forward

	pop    {r4-r8, pc}


/* Move the cursor forward
 * r0: address of terminfo
 */
term_cursor_forward:
	ldr    r1, [r0, #4]
	ldr    r2, [r0, #12]
	add    r2, r2, #1
	cmp    r1, r2
	strne  r2, [r0, #12]
	bxne   lr

	mov    r2, #0
	str    r2, [r0, #12]
	// FIXME: check for last line
	ldr    r2, [r0, #16]
	add    r2, r2, #1
	str    r2, [r0, #16]  // y
	bx     lr

/* Print a string to the terminal
 * r0: terminfo address
 * r1: address of a null-terminated string
 */
term_printf:
	push   {r4 - r6, lr}
	mov    r4, r0
	mov    r5, r1
	ldrb   r6, [r5]

	term_printf$char:
	cmp    r6, #0x20
	bne    term_printf$char_print
	bl     term_cursor_forward
	b      term_printf$next

	term_printf$char_print:
	mov    r0, r4
	mov    r1, r6
	bl     term_print_char

	term_printf$next:
	add    r5, r5, #1
	ldrb   r6, [r5]
	cmp    r6, #0
	bne    term_printf$char

	pop    {r4 - r6, pc}
