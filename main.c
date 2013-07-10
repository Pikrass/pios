#include "screen.h"
#include "term.h"
#include "led.h"

#define WELCOME "Welcome to Pios, the little program wishing to become a " \
                "full operating system some day. But for now it barely " \
                "knows how to print a string, so... here it is!\n\n" \
                "Logo is at : 0x%x\n" \
                "Framebuffer is at : 0x%x"
 

void *get_logo();

void main() {
	void *fb, *logo;
	struct terminfo term;

	fb = fb_request(1024, 762, 24);
	if(fb == 0)
		led_pattern(0b00010101, 8, 0x400000, 0);

	logo = get_logo();
	fb_draw_image(fb, logo, 487, 211);

	term_init(fb, 1024, 768, 3);
	term_create(fb + 211*1024*3, 100, 20, &term);
	term_printf(&term, WELCOME, logo, fb);

	led_pattern(0b00011111, 8, 0x400000, 0);
}
