#include "screen.h"
#include <stdint.h>
#include <multiboot2.h>
#include "font.h"
#include <types.h>


uint8_t screen_init(struct multiboot_tag_framebuffer* mb_fb)
{
	if(mb_fb->common.framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB) {
		/* TODO: Log and fail*/
		return 1;
	}

	width = mb_fb->common.framebuffer_width;
	height = mb_fb->common.framebuffer_height;
	bpp = mb_fb->common.framebuffer_bpp;
	buffer = (uint32_t*)(uint32_t)mb_fb->common.framebuffer_addr;

}


void screen_put_pixel(uint32_t x, uint32_t y, uint32_t argb)
{
	uint32_t location = x + (y * width);

	/* Check overflow */
	if(location > width * height)  {
		/* TODO: LOG */
		return;
	}

	buffer[location] = argb;
}

void screen_put_char(unsigned char c, uint32_t x, uint32_t y,
		     uint32_t foreground, uint32_t background)
{
	/* Get the index of the character in the font bitmap */
	uint8_t *bitmap = font.Bitmap + (uint64_t)(c * font.Height);

	/* Bitmask */
	int mask[8]={128, 64, 32, 16, 8, 4, 2, 1};

	/* iterators */
	uint32_t cx, cy;

	for(cy = 0; cy < font.Height; cy++) {
		for(cx = 0; cx < font.Width; cx++) {
			if(bitmap[cy] & mask[cx]) {
				screen_put_pixel(x+cx, y+cy, foreground);
			} else {
				screen_put_pixel(x+cx, y+cy, background);
			}
		}
	}
}

void screen_clear()
{
	if(buffer == NULL) {
		return;
	}

	uint32_t i;
	for(i = 0; i < width * height; i++) {
		buffer[i] = 0x00000000;
	}
}

uint32_t screen_get_width()
{
	return width;
}

uint32_t screen_get_height()
{
	return height;
}
