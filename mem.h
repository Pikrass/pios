#ifndef MEM_H
#define MEM_H

#include "types.h"

// Access permission flags (first: privileged, second: user)
#define SECT_RW_NO 0<<15 | 1<<10
#define SECT_RW_RO 0<<15 | 2<<10
#define SECT_RW_RW 0<<15 | 3<<10
#define SECT_RO_NO 1<<15 | 1<<10
#define SECT_RO_RO 1<<15 | 2<<10
// Disable execution
#define SECT_XN    1<<4

struct kheap_chunk {
	size_t prev_size;
	size_t size; // -1 for wilderness chunk, bit 0 high if free
	struct kheap_chunk *next_free;
};

void mem_init();
void *kmalloc(size_t bytes);
void map_section(unsigned int phy, unsigned int virt, unsigned int flags);

#endif
