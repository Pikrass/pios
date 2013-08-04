#include "mem.h"
#include "screen.h"
#include "term.h"
#include "led.h"
#include "dma.h"
#include "sd.h"
#include "mbr.h"
#include "fat32.h"

#define WELCOME "Welcome to Pios, the little program wishing to become a " \
                "full operating system some day. But for now it barely " \
                "knows how to print a string, so... here it is!\n\n"
 

void *get_logo();

void main() __attribute__((noreturn));

void main() {
	void *fb, *logo;
	int err;
	struct terminfo term;
	struct sd_card card;

	mem_init();

	fb = fb_request(1024, 762, 24);
	if(fb == 0)
		goto error;

	// Map the framebuffer in our address space at 0xf0100000 - 0xf0500000
	for(int i=0 ; i<4 ; ++i)
		map_section((unsigned int)fb + i*0x100000, 0xf0100000 + i*0x100000, SECT_XN | SECT_RW_NO);

	fb = (void*)(((unsigned int)fb & ~0xfff00000) | 0xf0100000);

	logo = get_logo();
	fb_draw_image(fb, logo, 487, 211);

	term_init(fb, 1024, 768, 3);
	term_create(fb + 211*1024*3, 100, 20, &term);
	term_printf(&term, WELCOME);

	dma_reset(DMA_CHAN_EMMC);

	if(err = sd_init(&card)) {
		term_printf(&term, "SD card initialization failed (%x)\n", err);
		goto error;
	}

	int size = (card.csd.c_size + 1) << (card.csd.c_size_mult + card.csd.read_bl_len + 2);
	term_printf(&term, "SD card initialized: capacity 0x%x (block 0x%x)\n", size, card.csd.read_bl_len);

	struct mbr *mbr = kmalloc(512, KMALLOC_CONT);

	if(err = sd_read(&card, 0, 1, mbr)) {
		term_printf(&term, "SD read failed (%x)\n", err);
		goto error;
	}

	term_printf(&term, "SD read completed\n");
	struct partition *parts = kmalloc(4 * sizeof(struct partition), 0);
	parse_partition_table(mbr, parts);

	struct fat **fat_parts = kmalloc(4 * sizeof(struct fat*), 0);

	for(int i=0 ; i<4 ; ++i) {
		fat_parts[i] = NULL;

		if(parts[i].size == 0)
			continue;
		if(parts[i].type != PARTTYPE_FAT32) {
			term_printf(&term, "Partition %x is of type 0x%x\n", i);
			continue;
		}

		struct fat *fat = kmalloc(sizeof(struct fat), 0);
		fat32_init(&card, parts[i].start, fat);
		fat_parts[i] = fat;

		term_printf(&term, "Partition %x is fat32: sec_len 0x%x clu_len 0x%x fat_len 0x%x root 0x%x\n",
			i, fat->sector_len, fat->cluster_len, fat->fat_len, fat->root);
	}

	led_pattern(0b00011111, 8, 0x400000, 0);

error:
	led_pattern(0b00010101, 8, 0x400000, 0);

	while(1);
}
