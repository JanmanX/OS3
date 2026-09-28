#include "cursor.h"
#include "gfx.h"
#include <stdint.h>
#include <types.h>

/* Colours of the two solid parts of the arrow */
#define CURSOR_OUTLINE		GFX_RGB(0x10, 0x12, 0x18)
#define CURSOR_FILL		GFX_RGB(0xF2, 0xF4, 0xF8)
#define CURSOR_FILL_PRESSED	GFX_RGB(0x5B, 0xC8, 0xFA)

/*
 * 'X' outline, '#' fill, ' ' transparent. Every row is CURSOR_WIDTH long and
 * there are CURSOR_HEIGHT of them.
 */
static const char *cursor_pixmap[CURSOR_HEIGHT] = {
	"X           ",
	"XX          ",
	"X#X         ",
	"X##X        ",
	"X###X       ",
	"X####X      ",
	"X#####X     ",
	"X######X    ",
	"X#######X   ",
	"X########X  ",
	"X#####XXXXX ",
	"X##X##X     ",
	"X#X X##X    ",
	"XX  X##X    ",
	"X    X##X   ",
	"     X##X   ",
	"     X##X   ",
	"      XX    "
};


gfx_rect_t cursor_bounds(int32_t x, int32_t y)
{
	return gfx_rect(x, y, CURSOR_WIDTH, CURSOR_HEIGHT);
}

void cursor_draw(int32_t x, int32_t y, uint8_t pressed)
{
	int32_t row, col;
	uint32_t fill = pressed ? CURSOR_FILL_PRESSED : CURSOR_FILL;

	for(row = 0; row < CURSOR_HEIGHT; row++) {
		const char *line = cursor_pixmap[row];

		for(col = 0; col < CURSOR_WIDTH; col++) {
			char p = line[col];

			if(p == 'X') {
				gfx_pixel(x + col, y + row, CURSOR_OUTLINE);
			} else if(p == '#') {
				gfx_pixel(x + col, y + row, fill);
			}
		}
	}
}
