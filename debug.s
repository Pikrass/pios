.equ col_shift, 500 * 3

.data
.align 4
terminfo: .zero 20
regs_str: .ascii "r0: 0x%x    r1: 0x%x    r2: 0x%x\n"
          .ascii "r3: 0x%x    r4: 0x%x    r5: 0x%x\n"
          .ascii "r6: 0x%x    r7: 0x%x    r8: 0x%x\n"
          .asciz "r9: 0x%x\n"

.section .text
.globl dbg_init
.globl dbg_print_regs

/* r0: base address of the framebuffer
 */
dbg_init:
	push   {lr}
	ldr    r1, =col_shift
	add    r0, r0, r1
	mov    r1, #51
	mov    r2, #10
	ldr    r3, =[terminfo]
	bl     term_create
	pop    {pc}

/* Print register contents on a specific area on screen
 * All registers from r0 to r9 are preserved, and restored on return
 */
dbg_print_regs:
	push   {r0, r1, lr}
	push   {r2 - r9}

	mov    r2, r0
	mov    r3, r1
	ldr    r0, =[terminfo]
	ldr    r1, =[regs_str]
	bl     term_printf

	pop    {r2 - r9}
	pop    {r0, r1, pc}
