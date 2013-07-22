#include "mem.h"

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
