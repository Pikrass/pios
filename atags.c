#include "atags.h"

#include "term.h"

void parse_atags(struct terminfo *term) {
	struct atag * tag = (struct atag*)0x100;

	while(tag->type != ATAG_NONE) {
		switch(tag->type) {
			case ATAG_MEM:
				term_printf(term, "Memory from 0x%x, len 0x%x\n", tag->mem.start, tag->mem.size);
				break;
			case ATAG_CMDLINE:
				term_printf(term, "Command line: ");
				term_printf(term, &tag->cmdline.str);
				term_printf(term, "\n");
				break;
		}

		tag = (void*)tag + tag->size*4;
	}
}
