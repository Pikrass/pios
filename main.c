#include "screen.h"
#include "term.h"
#include "led.h"
#include "sd.h"
#include "mem.h"

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

	if(err = sd_init(&card)) {
		term_printf(&term, "SD card initialization failed (%x)\n", err);
		goto error;
	}

	int size = (card.csd.c_size + 1) << (card.csd.c_size_mult + card.csd.read_bl_len + 2);
	term_printf(&term, "SD card initialized: capacity 0x%x (block 0x%x)\n", size, card.csd.read_bl_len);

	unsigned int *dest = kmalloc(512, KMALLOC_CONT);

	if(err = sd_read(&card, 0, 1, dest)) {
		term_printf(&term, "SD read failed (%x)\n", err);
		goto error;
	}

	term_printf(&term, "SD read completed\n");
	term_printf(&term, "First bytes = %x %x %x %x\n", dest[0], dest[1], dest[2], dest[3]);

	led_pattern(0b00011111, 8, 0x400000, 0);

error:
	led_pattern(0b00010101, 8, 0x400000, 0);

	while(1);
}
