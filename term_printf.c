#include "term.h"

#include <stdarg.h>

void term_printf(struct terminfo *term, char *format, ...) {
	va_list va;
	va_start(va, format);

	while(*format != '\0') {
		switch(*format) {
			case '%':
				++format;
				switch(*format) {
					case 'x':
						term_print_hex(term, va_arg(va, int));
						break;
					case '\0':
						goto end;
				}
				break;
			case ' ':
				term->x++;
				if(term->x == term->width) {
					term->x = 0;
					term->y++;
					if(term->y == term->height)
						term->y--;
				}
				break;
			case '\n':
				term->x = 0;
				term->y++;
				if(term->y == term->height)
					term->y--;
				break;
			default:
				term_print_char(term, *format);
		}
		++format;
	}

end:
	va_end(va);
}

void term_print_hex(struct terminfo *term, unsigned int number) {
	char str[9];
	int i = 8;

	str[8] = '\0';

	do {
		unsigned short digit = number & 0xf;
		str[--i] = digit + (digit >= 0xa ? 0x57 : 0x30);
		number >>= 4;
	} while(number != 0);

	term_printf(term, str + i);
}
