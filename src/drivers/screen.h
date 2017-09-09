#pragma once

#include <stdint.h>
#include <multiboot2.h>

/* Colors */
#define COLOR_ALPHA	0xFF000000
#define COLOR_RED	0x00FF0000
#define COLOR_GREEN	0x0000FF00
#define COLOR_BLUE	0x000000FF

uint8_t screen_init();
void screen_put_pixel(uint32_t x, uint32_t y, uint32_t argb);
void screen_put_char(unsigned char c, uint32_t x, uint32_t y,
		     uint32_t foreground, uint32_t background);


void screen_clear();
uint32_t screen_get_width();
uint32_t screen_get_height();
uint8_t screen_get_bpp();
