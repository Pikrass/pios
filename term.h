#ifndef TERM_H
#define TERM_H

struct terminfo {
	void* base;
	int   width, height;
	int   x, y;
};

void term_init(void *fb, int width, int height, int depth);
void term_create(void *fb, int cols, int lines, struct terminfo *term);
void term_printf(struct terminfo *term, char *format, ...);
void term_print_hex(struct terminfo *term, unsigned int number);
void term_print_char(struct terminfo *term, char character);

#endif
