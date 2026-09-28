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

/*
 * Double buffering
 *
 * Until a back buffer is handed over, every draw goes straight at the
 * framebuffer. Once one exists, draws land in it and only reach the screen
 * when something asks for them to be presented.
 *
 * While auto-present is on - which it is by default, so that the boot log
 * still appears as it is written - each pixel is written to both buffers.
 * A compositor turns it off and drives the presents itself.
 */
uint8_t screen_set_backbuffer(uint32_t *back);
uint32_t *screen_get_backbuffer(void);

void screen_set_auto_present(uint8_t on);

void screen_present(void);
void screen_present_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h);

/* Pixels from the start of one row to the start of the next. Not always the
 * visible width: the framebuffer may pad its scanlines. */
uint32_t screen_get_stride_px(void);

/* Bytes a buffer matching the framebuffer's geometry has to be */
uint64_t screen_get_buffer_size(void);
