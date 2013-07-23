#include "screen.h"
#include "term.h"
#include "led.h"
#include "sd.h"
#include "atags.h"

#define WELCOME "Welcome to Pios, the little program wishing to become a " \
                "full operating system some day. But for now it barely " \
                "knows how to print a string, so... here it is!\n\n"
 

void *get_logo();

void main() {
	void *fb, *logo;
	int err;
	struct terminfo term;
	struct sd_card card;

	fb = fb_request(1024, 762, 24);
	if(fb == 0)
		led_pattern(0b00010101, 8, 0x400000, 0);

	logo = get_logo();
	fb_draw_image(fb, logo, 487, 211);

	term_init(fb, 1024, 768, 3);
	term_create(fb + 211*1024*3, 100, 20, &term);
	term_printf(&term, WELCOME);

	atags_find_mem(&term);

	if(err = sd_init(&card)) {
		term_printf(&term, "SD card initialization failed (%x)\n", err);
		goto error;
	}

	int size = (card.csd.c_size + 1) << (card.csd.c_size_mult + card.csd.read_bl_len + 2);
	term_printf(&term, "SD card initialized: capacity 0x%x (block 0x%x)\n", size, card.csd.read_bl_len);

	unsigned int *dest = (unsigned int *)0x100000;

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
