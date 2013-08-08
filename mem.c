#include "mem.h"

#include "atags.h"

#define TOP_TABLE ((unsigned int*)0xf0000000)
#define F00_TABLE ((unsigned int*)0xf0004c00)

void kheap_init();
void page_alloc_init();
void page_alloc_init_walksection();

void section_used(unsigned int addr);
void page_used(unsigned int addr);

int map_page(unsigned int phy, unsigned int virt, unsigned int flags);

void *map_page_tmp(unsigned int phy, unsigned int flags);
void unmap_page_tmp(void *mapping);

int chunk_is_contiguous(const struct kheap_chunk *chunk, size_t bytes);
void *kmalloc_chunk(struct kheap_chunk *chunk, struct kheap_chunk **prev_list, size_t bytes);
void *kmalloc_wilderness(struct kheap_chunk *chunk, struct kheap_chunk **prev_list, size_t bytes, unsigned int flags);


extern void *__edata;
struct kheap_chunk *free_chunk = NULL;
unsigned int kheap_brk = 0;

unsigned int max_mem = 0;
unsigned char *page_bitmap = NULL;
unsigned int first_free_page = 0;


void mem_init() {
	kheap_init();
	page_alloc_init();
}

void kheap_init() {
	free_chunk = (struct kheap_chunk*)(((unsigned int)&__edata & 0xfffff000) + 0x1000);
	free_chunk->prev_size = 0;
	free_chunk->size = -1;
	free_chunk->next_free = NULL;
	kheap_brk = ((unsigned int)&__edata & 0xfffff000) + 0x3000;
}

void page_alloc_init() {
	max_mem = atags_get_mem();
	unsigned int mem = max_mem >> 15; // bytes / (4096*8)
	page_bitmap = kmalloc(mem, 0);

	for(int i=0 ; i < mem ; ++i)
		page_bitmap[i] = 0;

	for(int i=0 ; i < 4096 ; ++i) {
		if(TOP_TABLE[i] & 2)
			section_used(TOP_TABLE[i]);
		else if(TOP_TABLE[i] & 1)
			page_alloc_init_walksection(TOP_TABLE[i]);
	}

	// Skip the range 0x0000 - 0x8000 for the "first free page"
	for(int i=1 ; i<mem ; ++i) {
		if(page_bitmap[i] != 0xff) {
			first_free_page = i*8;

			int mask = 1;
			while(page_bitmap[i] & mask) {
				first_free_page++;
				mask <<= 1;
			}

			break;
		}
	}
}

void page_alloc_init_walksection(unsigned int section) {
	section &= 0xfffffc00;
	unsigned int *page = map_page_tmp(section, PAGE_RO_NO | PAGE_XN);

	if(page == NULL)
		return;

	unsigned int *table = (unsigned int*)((unsigned int)page | (section & 0xc00));

	for(int i=0 ; i<256 ; ++i) {
		if(table[i] & 2)
			page_used(table[i]);
	}

	unmap_page_tmp(page);
}

void section_used(unsigned int addr) {
	if(addr >= max_mem)
		return;

	addr >>= 20;
	for(int i=0 ; i<32 ; ++i)
		page_bitmap[32*addr + i] = 0xff;
}

void page_used(unsigned int addr) {
	if(addr >= max_mem)
		return;

	addr >>= 12;
	char bit = 1 << (addr & 7);
	addr >>= 3;
	page_bitmap[addr] |= bit;
}

void page_unused(unsigned int addr) {
	if(addr >= max_mem)
		return;

	addr >>= 12;
	char bit = 1 << (addr & 7);
	addr >>= 3;
	page_bitmap[addr] &= ~bit;
}

void *alloc_phy_pages(size_t num) {
	// This is a very simple (and dumb) way to manage memory pages. It will be
	// improved when we need more than a few megs. :)

	void *ret = (void*)(first_free_page << 12);

	for(int i=0 ; i<num ; ++i) {
		page_used(first_free_page << 12);
		first_free_page++;
	}

	return ret;
}

void free_phy_pages(void *start, int num) {
	start = (void*)((unsigned int)start & 0xfffff000);
	for(int i=0 ; i<num ; ++i) {
		page_unused((unsigned int)start + i*0x1000);
	}

	while((page_bitmap[(first_free_page-1)/8] & 1<<((first_free_page-1)%8)) == 0)
		first_free_page--;
}

int kheap_grow(unsigned int nb_pages) {
	void *pages = alloc_phy_pages(nb_pages);

	if(pages == NULL)
		return 1;

	for(int i=0 ; i<nb_pages ; ++i) {
		map_page((unsigned int)(pages + i*0x1000), kheap_brk, PAGE_RW_NO | PAGE_XN);
		kheap_brk += 0x1000;
	}

	return 0;
}

void *kmalloc(size_t bytes, unsigned int flags) {
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
			if((flags & KMALLOC_CONT) == 0 || chunk_is_contiguous(chunk, bytes)) {
				best_fit = chunk;
				best_size = chunk->size;
				best_prev = prev_list;
			}
		}

		prev_list = &chunk->next_free;
		chunk = chunk->next_free;
	}

	if(best_fit == 0)
		return NULL;

	if(best_fit->size == -1)
		return kmalloc_wilderness(best_fit, best_prev, bytes, flags);
	else
		return kmalloc_chunk(best_fit, best_prev, bytes);
}

int chunk_is_contiguous(const struct kheap_chunk *chunk, size_t bytes) {
	// The wilderness chunk can always produce contiguous chunks
	if(chunk->size == -1)
		return 1;

	unsigned int first = ((unsigned int)chunk + 8) & 0xfffff000,
				 last = ((unsigned int)chunk + bytes + 8) & 0xfffff000;

	if(first == last)
		return 1;

	unsigned int next_page = (unsigned int)virt_to_phy((void*)first);

	for(unsigned int vpage=first+0x1000 ; vpage <= last ; vpage += 0x1000) {
		next_page += 0x1000;

		if((unsigned int)virt_to_phy((void*)vpage) != next_page)
			return 0;
	}

	return 1;
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

void *kmalloc_wilderness(struct kheap_chunk *chunk, struct kheap_chunk **prev_list, size_t bytes, unsigned int flags) {
	size_t remain = kheap_brk - (unsigned int)chunk;

	if(remain - 8 < bytes) {
		if(flags & KMALLOC_CONT) {
			// Move the wilderness chunk to the boundary
			struct kheap_chunk *prev = chunk;
			chunk = (struct kheap_chunk*)(kheap_brk - 8);

			prev->size = (unsigned int)chunk - (unsigned int)prev + 1;
			if(prev->size >= 17) {
				*prev_list = prev;
				prev_list = &prev->next_free;
			}

			// Allocate enough pages
			if(kheap_grow(bytes / 0x1000 + 1))
				return NULL;

			chunk->next_free = NULL;
		}
		else {
			while(remain - 8 < bytes) {
				if(kheap_grow(1))
					return NULL;
				remain += 0x1000;
			}
		}
	}

	chunk->size = bytes + 8;

	struct kheap_chunk *next = (void*)chunk + chunk->size;
	if((unsigned int)next >= kheap_brk) {
		if(kheap_grow(1))
			return NULL;
	}

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

unsigned int *next_coarse = NULL;
unsigned int *alloc_coarse_table() {
	unsigned int *ret;

	if(next_coarse == NULL) {
		ret = alloc_phy_pages(1);
		next_coarse = ret + 0x400;
	}
	else {
		ret = next_coarse;
		next_coarse += 0x400;
		if(((unsigned int)next_coarse & 0xfff) == 0)
			next_coarse = NULL;
	}

	return ret;
}

int map_page(unsigned int phy, unsigned int virt, unsigned int flags) {
	virt &= 0xfffff000;
	phy &= 0xfffff000;

	unsigned int section = TOP_TABLE[virt>>20];
	unsigned int *coarse_table_phy;
	unsigned int *coarse_table;

	switch(section & 3) {
		case 0:
			coarse_table_phy = alloc_coarse_table();
			coarse_table = map_page_tmp((unsigned int)coarse_table_phy, PAGE_RW_NO | PAGE_XN);
			coarse_table = (unsigned int*)((unsigned int)coarse_table | ((unsigned int)coarse_table_phy & 0xc00));
			TOP_TABLE[virt>>20] = (unsigned int)coarse_table_phy | 1;
			break;
		case 1:
			coarse_table = map_page_tmp(section, PAGE_RW_NO | PAGE_XN);
			coarse_table = (unsigned int*)((unsigned int)coarse_table | (section & 0xc00));
			break;
		case 2:
			return 1;
	}

	coarse_table[(virt&0xff000)>>12] = phy | flags | 1;
	__asm("mcr p15, 0, %[addr], c8, c7, 1" : : [addr] "r" (virt));
	return 0;
}

unsigned short map_frame = 5;
void *map_page_tmp(unsigned int phy, unsigned int flags) {
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
void unmap_page_tmp(void *mapping) {
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
	tables = map_page_tmp(table_loc, PAGE_RO_NO | PAGE_XN);
	if(tables == NULL)
		return (void*)-2;

	tablei = ((unsigned int)va & 0x000ff000) >> 12;
	entry = tables[(table_loc & 0xc00)>>10][tablei];
	entry_type = (entry & 0x3);

	unmap_page_tmp(tables);

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
