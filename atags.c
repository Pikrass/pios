#include "atags.h"

#include "term.h"

unsigned int atags_get_mem() {
	struct atag * tag = (struct atag*)0x100;

	while(tag->type != ATAG_NONE) {
		if(tag->type == ATAG_MEM)
			return tag->mem.size;

		tag = (void*)tag + tag->size*4;
	}
}
