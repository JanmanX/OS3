#include "kprintf.h"
#include <stdarg.h>
#include <stdint.h>
#include <drivers/screen.h>
#include <drivers/font.h>
#include <libc.h>


/* Position of the */
static uint32_t x = 0;
static uint32_t y = 0;

void write_char(uint8_t c)
{
	/* Clear screen, if bottom of screen reached*/
	if(y >= screen_get_height()) {
		clear_screen();
	}

	/* Handle newlines */
	if(c == '\n') {
		y += font.Height;
		x = 0;
	} else {
		/* Write character at position x,y*/
		screen_put_char(c, x, y, 0xFFFFFF88, 0xFF000000);

		/* Move to next character*/
		x += font.Width;

		/* If at the end of the line, move down */
		if(x >= screen_get_width()) {
			x = 0;
			y += font.Height;
		}
	}
}

void kprint(char *str)
{
	uint8_t c;
	while((c = *str++) != 0) {
		write_char(c);
	}
}

void kprintf(const char *fmt, ...)
{
	/* Retrieve variable arguments */
	va_list args;
	va_start(args, fmt);

	/* Buffers */
	uint8_t c;
	char buf[24];

	/* Iterate over format string */
	while((c = *fmt++) != 0)
	{
		if( c != '%') {
			write_char(c);
			continue;
		}

		/* Get the character after '%' */
		c = *fmt++;
		char *s;


		switch(c) {
		case 'd':
		case 'u':
		case 'x':
			itoa(buf, c, va_arg(args, uint64_t));

			s = buf;
			goto string;

		case 'c':
			buf[0] = (char)va_arg(args, int);
			buf[1] = (char)0;
			s = buf;
			goto string;

		case 's':
			s = va_arg(args, char*);

			if(!s) {
				s = "(NULL)";
			}
			/* Fall through */
			/* goto string; */
		string:
			kprint(s);
			break;
		default:
			break;
		}
	}


	va_end(args);
}

void clear_screen()
{
	screen_clear();
	x = y = 0;
}

