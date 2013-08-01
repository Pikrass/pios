.equ gpio_mail, 0xf200b880

.section .text
.globl mailbox_read
.globl mailbox_write

/* Read mail in mailbox 0. This function blocks until there is mail to read.
 * r0: the channel we're interested in.
 */
mailbox_read:
	ldr    r1, =gpio_mail

	mailbox_read$wait_status:
	ldr    r2, [r1, #0x18]
	ands   r2, r2, #0x40000000
	bne    mailbox_read$wait_status

	ldr    r2, [r1]
	ands   r3, r2, #0x0000000f
	cmp    r3, r0
	bne    mailbox_read$wait_status

	and    r0, r2, #0xfffffff0
	bx     lr


/* Write the value in r0 to mailbox 0.
 * r0: the value in the upper 28 bits, the channel in the lower 4 bits
 */
mailbox_write:
	ldr    r1, =gpio_mail

	mailbox_write$wait_status:
	ldr    r2, [r1, #0x18]
	ands   r2, r2, #0x80000000
	bne    mailbox_write$wait_status

	str    r0, [r1, #0x20]
	bx     lr
