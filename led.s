.equ gpio_ctrl, 0xf2200000

.section .text
.globl led_mode_write
.globl led_on
.globl led_off
.globl led_status
.globl led_pattern

led_mode_read:
	ldr    r0, =gpio_ctrl
	mov    r1, #0
	str    r1, [r0, #0x04]
	bx     lr

led_mode_write:
	ldr    r0, =gpio_ctrl
	mov    r1, #1
	lsl    r1, #18
	str    r1, [r0, #0x04]
	bx     lr

led_on:
	ldr    r0, =gpio_ctrl
	mov    r1, #1
	lsl    r1, #16
	str    r1, [r0, #0x28]
	bx     lr

led_off:
	ldr    r0, =gpio_ctrl
	mov    r1, #1
	lsl    r1, #16
	str    r1, [r0, #0x1c]
	bx     lr

led_status:
	ldr    r1, =gpio_ctrl
	ldr    r0, [r1, #0x34]
	cmp    r0, #0
	moveq  r0, #1
	movne  r0, #0
	bx     lr

/* Make the led blink in a specified pattern
 * r0: bit pattern, 0 = off and 1 = on
 * r1: number of bits in the pattern
 * r2: delay between each bit
 * r3: number of times the pattern must repeat, or 0 if it should repeat forever
 */
led_pattern:
	push   {r4-r11, lr}
	mov    r4, r0
	mov    r5, r1
	mov    r6, r2
	mov    r7, r3
	
	// Limit r5 to 32
	cmp    r5, #32
	movhi  r5, #32

	// Get the current led status
	bl     led_mode_read
	bl     led_status
	mov    r8, r0
	bl     led_mode_write

	led_pattern$loop:
	mov    r10, #1  // bitmask
	mov    r11, r5  // iterator

	led_pattern$nextbit:
	ands   r9, r4, r10
	beq    led_pattern$off

	led_pattern$on:
	cmp    r8, #0
	bleq   led_on
	b      led_pattern$endbit

	led_pattern$off:
	cmp    r8, #0
	blne   led_off

	led_pattern$endbit:
	mov    r8, r9
	subs   r11, r11, #1
	beq    led_pattern$endloop
	mov    r0, r6
	bl     _wait
	lsl    r10, #1
	b      led_pattern$nextbit

	led_pattern$endloop:
	cmp    r7, #1
	beq    led_pattern$end
	cmp    r7, #0
	subne  r7, r7, #1
	mov    r0, r6
	bl     _wait
	b      led_pattern$loop

	led_pattern$end:
	pop    {r4-r11, pc}

/* Waste some time
 * r0: number of iterations
 */
_wait:
	cmp    r0, #0
	bxeq   lr
	sub    r0, r0, #1
	b       _wait
