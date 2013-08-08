#ifndef MEM_H
#define MEM_H

#include "types.h"

// Access permission flags (first: privileged, second: user)
#define SECT_RW_NO 0<<15 | 1<<10
#define SECT_RW_RO 0<<15 | 2<<10
#define SECT_RW_RW 0<<15 | 3<<10
#define SECT_RO_NO 1<<15 | 1<<10
#define SECT_RO_RO 1<<15 | 2<<10

#define PAGE_RW_NO 0<<9 | 1<<4
#define PAGE_RW_RO 0<<9 | 2<<4
#define PAGE_RW_RW 0<<9 | 3<<4
#define PAGE_RO_NO 1<<9 | 1<<4
#define PAGE_RO_RO 1<<9 | 2<<4

// Disable execution
#define SECT_XN    1<<4
#define PAGE_XN    1<<0

// kmalloc flags
#define KMALLOC_CONT 1 // contiguous physical memory

struct kheap_chunk {
	size_t prev_size;
	size_t size; // -1 for wilderness chunk, bit 0 high if free
	struct kheap_chunk *next_free;
};

void mem_init();

void *kmalloc(size_t bytes, unsigned int flags);

void *alloc_phy_pages(size_t num);
void *virt_to_phy(void *va);

void map_section(unsigned int phy, unsigned int virt, unsigned int flags);
int map_page(unsigned int phy, unsigned int virt, unsigned int flags);

#endif
