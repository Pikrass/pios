.section .rodata
	.equ gpio_ctrl, 0x20200000

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

@ Affiche une séquence de positions de led
@ r0: motif de bits, 0 pour éteint et 1 pour allumé
@ r1: nombre de bits utilisés (de poids faible à poids fort)
@ r2: temps d'attente entre chaque changement
@ r3: si la séquence doit se répéter indéfiniment (non-implémenté)
led_pattern:
	push   {r4-r11, lr}
	mov    r4, r0
	mov    r5, r1
	mov    r6, r2
	mov    r7, r3
	
	@ Bloque r5 à 32
	cmp    r5, #32
	movhi  r5, #32

	@ Récupère l'état actuel de la LED
	@bl    led_mode_read
	@bl    led_status
	@mov   r8, r0
	bl     led_mode_write

	led_pattern$loop:
	mov    r10, #1  @ masque roulant
	mov    r11, r5  @ itérateur

	led_pattern$nextbit:
	ands   r9, r4, r10
	beq    led_pattern$off

	led_pattern$on:
	@cmp   r8, #0
	@bleq  led_on
	bl     led_on @debug
	b      led_pattern$endbit

	led_pattern$off:
	@cmp   r8, #0
	bl     led_off @debug
	blne   led_off

	led_pattern$endbit:
	@mov   r8, r9
	subs   r11, r11, #1
	beq    led_pattern$end
	mov    r0, r6
	bl     _wait
	lsl    r10, #1
	b      led_pattern$nextbit

	led_pattern$end:
	pop    {r4-r11, pc}

@ Attend un temps donné dans r0
_wait:
	cmp    r0, #0
	bxeq   lr
	sub    r0, r0, #1
	b       _wait
