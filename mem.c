#include "mem.h"

#define TOP_TABLE ((unsigned int*)0xf0000000)
#define F00_TABLE ((unsigned int*)0xf0004c00)

void kheap_init();
void *kmalloc_chunk(struct kheap_chunk *chunk, struct kheap_chunk **prev_list, size_t bytes);
void *kmalloc_wilderness(struct kheap_chunk *chunk, struct kheap_chunk **prev_list, size_t bytes);

extern void *__edata;
struct kheap_chunk *free_chunk;
void *kheap_brk;

void mem_init() {
	kheap_init();
}

void kheap_init() {
	free_chunk = (struct kheap_chunk*)(((unsigned int)&__edata & 0xfffff000) + 0x1000);
	free_chunk->prev_size = 0;
	free_chunk->size = -1;
	free_chunk->next_free = NULL;
	kheap_brk = (struct hheap_chunk*)(((unsigned int)&__edata & 0xfffff000) + 0x2000);
}

void *kmalloc(size_t bytes) {
	struct kheap_chunk *chunk = free_chunk, *best_fit = (void*)0xffffffff;
	struct kheap_chunk **prev_list = &free_chunk, **best_prev;
	size_t best_size = -1;

	if(bytes < 8)
		bytes = 8;
	else if(bytes & 0x7)
		bytes = (bytes & ~0x7) + 0x8;

	while(chunk != NULL) {
		if(chunk->size > bytes+8
				&& (chunk->size < best_size
					|| (chunk->size == best_size && chunk < best_fit))) {
			best_fit = chunk;
			best_size = chunk->size;
			best_prev = prev_list;
		}

		prev_list = &chunk->next_free;
		chunk = chunk->next_free;
	}

	if(best_fit == 0)
		return NULL;

	if(best_fit->size == -1)
		return kmalloc_wilderness(best_fit, best_prev, bytes);
	else
		return kmalloc_chunk(best_fit, best_prev, bytes);
}

void *kmalloc_chunk(struct kheap_chunk *chunk, struct kheap_chunk **prev_list, size_t bytes) {
	if(chunk->size-1 == bytes + 8) {
		// Perfect fit
		chunk->size--;
		*prev_list = chunk->next_free;
	}
	else {
		struct kheap_chunk *new  = (void*)chunk + bytes + 8,
		                   *next = (void*)chunk + chunk->size-1;
		new->size = chunk->size - bytes - 8;
		chunk->size = bytes + 8;
		new->prev_size = chunk->size;
		next->prev_size = new->size - 1;

		*prev_list = new;
		new->next_free = chunk->next_free;
	}

	return &chunk->next_free;
}

void *kmalloc_wilderness(struct kheap_chunk *chunk, struct kheap_chunk **prev_list, size_t bytes) {
	size_t remain = (unsigned int)kheap_brk - (unsigned int)chunk;

	while(remain - 8 < bytes) {
		//TODO: allocate a page
		return NULL;
	}

	chunk->size = bytes + 8;

	struct kheap_chunk *next = (void*)chunk + chunk->size;
	next->prev_size = chunk->size;
	next->size = -1;

	*prev_list = next;
	next->next_free = chunk->next_free;

	return &chunk->next_free;
}

void map_section(unsigned int phy, unsigned int virt, unsigned int flags) {
	virt &= 0xfff00000;
	phy &= 0xfff00000;
	TOP_TABLE[virt>>20] = phy | flags | 2;
	__asm("mcr p15, 0, %[addr], c8, c7, 1" : : [addr] "r" (virt));
}

unsigned short map_frame = 5;
void *map_page(unsigned int phy, unsigned int flags) {
	phy &= 0xfffff000;

	if(map_frame < 256) {
		void *virt = (void*)(0xf0000000 | map_frame << 12);
		F00_TABLE[map_frame++] = phy | flags | 2;
		__asm("mcr p15, 0, %[addr], c8, c7, 1" : : [addr] "r" (virt));
		return virt;
	}
	else {
		//FIXME: look for holes, then allocate a new block, rather than failing
		return NULL;
	}
}
void unmap_page(void *mapping) {
	unsigned int page = ((unsigned int)mapping & 0x000ff000) >> 12;
	F00_TABLE[page] = 0;

	while(map_frame > 5 && F00_TABLE[map_frame-1] == 0)
		map_frame--;
}

void *virt_to_phy(void *va) {
	unsigned int tablei;
	unsigned int entry, entry_type;

	tablei = ((unsigned int)va & 0xfff00000) >> 20;
	entry = TOP_TABLE[tablei];
	entry_type = entry & 0x3;

	switch(entry_type) {
		case 0:
		case 3:
			return (void*)-1;
		case 2:
			return (void*)((entry & 0xfff00000) | ((unsigned int)va & 0x000fffff));
	}

	unsigned int (*tables)[256];
	unsigned int table_loc = entry & 0xfffffc00;
	tables = map_page(table_loc, PAGE_RO_NO | PAGE_XN);
	if(tables == NULL)
		return (void*)-2;

	tablei = ((unsigned int)va & 0x000ff000) >> 12;
	entry = tables[(table_loc & 0xc00)>>10][tablei];
	entry_type = (entry & 0x3);

	unmap_page(tables);

	switch(entry_type) {
		case 0:
			return (void*)-1;
		case 1:
			return (void*)((entry & 0xffff0000) | ((unsigned int)va & 0x0000ffff));
		case 2:
		case 3:
			return (void*)((entry & 0xfffff000) | ((unsigned int)va & 0x00000fff));
	}
}
