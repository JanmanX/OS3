#include "screen.h"
#include <stdint.h>
#include <libc.h>
#include <multiboot2.h>
#include <multiboot_parser.h>
#include "font.h"
#include <errno.h>
#include <kprintf.h>

static uint32_t *buffer = NULL;
static uint32_t width = 0;
static uint32_t height = 0;
static uint8_t bpp = 0;


uint8_t screen_init()
{
	/* Retrieve the multiboot tag from the parser */
	struct multiboot_tag_framebuffer *mb_fb =
		(struct multiboot_tag_framebuffer*)
		multiboot_parser_get_tag(MULTIBOOT_TAG_TYPE_FRAMEBUFFER);

	/* Return if no tag found */
	if(mb_fb == NULL) {
		ERROR("Screen wrong!");
		return ENOENT;
	}

	if(mb_fb->common.framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB) {
		ERROR("framebuffer wrong type!");
		return EBADF;
	}

	width = mb_fb->common.framebuffer_width;
	height = mb_fb->common.framebuffer_height;
	bpp = mb_fb->common.framebuffer_bpp;
	buffer = (uint32_t*)mb_fb->common.framebuffer_addr;

	/* Return success */
	return EOK;
}




void screen_put_pixel(uint32_t x, uint32_t y, uint32_t argb)
{
	/* Check screen is initialized */
	if(buffer == NULL) {
		return;
	}

	/* Calculate the target offset */
	uint32_t location = x + (y * width);

	/* Check overflow */
	if(location > width * height)  {
		/* TODO: LOG */
		return;
	}

	/* Write the pixel */
	buffer[location] = argb;
}

void screen_put_char(unsigned char c, uint32_t x, uint32_t y,
		     uint32_t foreground, uint32_t background)
{
       /* Get the index of the character in the font bitmap */
       const uint8_t *bitmap = font.Bitmap + (uint64_t)(c * font.Height);

       /* Bitmask */
       int mask[8]={128, 64, 32, 16, 8, 4, 2, 1};

       /* iterators */
       uint32_t cx, cy;

       /* Iterate for each pixel-line in character */
       for(cy = 0; cy < font.Height; cy++) {

	       /* Iterate over each pixel */
               for(cx = 0; cx < font.Width; cx++) {
		       /* If pixel is on, draw it using foreground colour*/
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
	/* Sanity check */
       if(buffer == NULL) {
               return;
       }

       /* Zero out bytes from framebuffer */
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

uint8_t screen_get_bpp()
{
	return bpp;
}
