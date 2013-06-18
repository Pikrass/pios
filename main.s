.section .text
.globl _start
.extern led_on
_start:
	mov    sp, #0x10000

	bl     led_mode_write
	bl     led_on

loop:
	b      loop
