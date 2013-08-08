#include "exception.h"

#include "mem.h"
#include "term.h"
#include "led.h"

extern unsigned int vectors_start, vectors_end;
extern struct terminfo term_dbg;

void exception_init() {
	void *page = alloc_phy_pages(1);
	map_page((unsigned int)page, 0xffff0000, PAGE_RW_RO);

	unsigned int *table = (unsigned int*)0xffff0000;
	unsigned int *vect = &vectors_start;

	// Copy the vectors table and code
	while(vect < &vectors_end)
		*table++ = *vect++;

	// Select high vectors
	unsigned int ctrl_reg;
	__asm("mrc p15, 0, %[reg], c1, c0, 0" : [reg] "=r" (ctrl_reg));
	ctrl_reg |= 0x2000;
	__asm("mcr p15, 0, %[reg], c1, c0, 0" : : [reg] "r" (ctrl_reg));

	// Set up the banked stack pointers
	banked_sp_init();
}

void inst_abort_handler(void *inst, unsigned int status) {
	term_printf(&term_dbg, "Instruction abort:\nInst 0x%x\nStatus 0x%x\n", inst, status);
	led_pattern(0b00010101, 8, 0x400000, 0);
	while(1);
}

void data_abort_handler(void *inst, void *data, unsigned int status) {
	term_printf(&term_dbg, "Data abort:\nInst 0x%x\nAddr 0x%x\nStatus 0x%x\n", inst, data, status);
	led_pattern(0b00010101, 8, 0x400000, 0);
	while(1);
}
