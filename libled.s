.section .rodata
	.equ gpio_ctrl, 0x20200000

.section .text
.globl led_mode_write
.globl led_on
.globl led_off
.globl led_pattern

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

# Affiche une séquence de positions de led
# r0: motif de bits, 0 pour éteint et 1 pour allumé
# r1: nombre de bits utilisés (de poids faible à poids fort)
# r2: temps à attendre
# r3: si la séquence doit se répéter indéfiniment
led_pattern:
	push   {lr}
	bl     led_mode_write

#TODO

	pop   {pc}
