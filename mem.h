#ifndef MEM_H
#define MEM_H

#include "types.h"

struct kheap_chunk {
	size_t prev_size;
	size_t size; // -1 for wilderness chunk, bit 0 high if free
	struct kheap_chunk *next_free;
};

void mem_init();
void *kmalloc(size_t bytes);

#endif
