#include "atags.h"

#include "term.h"

void atags_find_mem(struct terminfo *term) {
	struct atag * tag = (struct atag*)0x100;

	while(tag->type != ATAG_NONE) {
		if(tag->type == ATAG_MEM)
			term_printf(term, "Memory from 0x%x, len 0x%x\n", tag->mem.start, tag->mem.size);

		tag = (void*)tag + tag->size*4;
	}
}
