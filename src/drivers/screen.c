#include "screen.h"
#include <stdint.h>
#include <libc.h>
#include <multiboot2.h>
#include <multiboot_parser.h>
#include "font.h"
#include <errno.h>
#include <kprintf.h>

/* The real framebuffer */
static uint32_t *front = NULL;

/* Where drawing actually lands. Equal to "front" until a back buffer is
 * installed. */
static uint32_t *buffer = NULL;

/* The installed back buffer, or NULL */
static uint32_t *back = NULL;

static uint32_t width = 0;
static uint32_t height = 0;
static uint8_t bpp = 0;

/* Pixels from the start of one row to the start of the next. The framebuffer
 * is allowed to pad its scanlines, so this is not always the visible width -
 * every row offset has to go through it rather than through "width". */
static uint32_t stride = 0;

/* Mirror every draw into the framebuffer as it happens */
static uint8_t auto_present = 0x01;


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
	front = (uint32_t*)mb_fb->common.framebuffer_addr;

	if(bpp != 32) {
		ERROR("framebuffer is not 32 bits per pixel!");
		return EBADF;
	}

	/* Trust the pitch the bootloader reports, and only fall back to the
	 * width if it gave us something unusable */
	stride = mb_fb->common.framebuffer_pitch / sizeof(uint32_t);

	if(stride < width) {
		stride = width;
	}

	LOGF("Screen: %dx%d, pitch %d bytes (%d pixels)\n",
	     (uint64_t)width, (uint64_t)height,
	     (uint64_t)mb_fb->common.framebuffer_pitch, (uint64_t)stride);

	/* Printed so that a test harness can read the framebuffer straight
	 * out of guest memory and compare it against what we composed */
	LOGF("Screen: framebuffer at 0x%x size 0x%x\n",
	     (uint64_t)front, screen_get_buffer_size());

	/* No back buffer yet - draw straight at the screen */
	buffer = front;

	/* Return success */
	return EOK;
}


uint8_t screen_set_backbuffer(uint32_t *new_back)
{
	if(front == NULL) {
		return ENODEV;
	}

	if(new_back == NULL) {
		/* Hand the screen back to direct drawing */
		back = NULL;
		buffer = front;
		return EOK;
	}

	back = new_back;
	buffer = back;

	/* Start from whatever is on screen, so that turning double buffering
	 * on mid-boot does not blank the log */
	memcpy(back, front, screen_get_buffer_size());

	return EOK;
}

uint32_t *screen_get_backbuffer(void)
{
	return back;
}

void screen_set_auto_present(uint8_t on)
{
	auto_present = on ? 0x01 : 0x00;
}


void screen_present(void)
{
	if(back == NULL || front == NULL) {
		return;
	}

	memcpy(front, back, screen_get_buffer_size());
}

void screen_present_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	uint32_t row;

	if(back == NULL || front == NULL) {
		return;
	}

	/* Clip to the screen */
	if(x >= width || y >= height) {
		return;
	}
	/* Subtract rather than add: x + w overflows for a large w and would
	 * skip the clamp entirely. x and y are known to be inside by now. */
	if(w > width - x) {
		w = width - x;
	}
	if(h > height - y) {
		h = height - y;
	}

	for(row = 0; row < h; row++) {
		uint64_t offset = (uint64_t)(y + row) * stride + x;

		memcpy(front + offset, back + offset, w * sizeof(uint32_t));
	}
}


uint32_t screen_get_stride_px(void)
{
	return stride;
}

uint64_t screen_get_buffer_size(void)
{
	return (uint64_t)stride * height * sizeof(uint32_t);
}


void screen_put_pixel(uint32_t x, uint32_t y, uint32_t argb)
{
	/* Check screen is initialized */
	if(buffer == NULL) {
		return;
	}

	/* Anything off-screen is dropped. Checking the axes separately is
	 * what keeps a large x from wrapping onto the next row. */
	if(x >= width || y >= height) {
		return;
	}

	/* Calculate the target offset */
	uint64_t location = (uint64_t)x + ((uint64_t)y * stride);

	/* Write the pixel */
	buffer[location] = argb;

	/* Keep the screen in step while nobody is driving the presents */
	if(auto_present && back != NULL) {
		front[location] = argb;
	}
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
       uint64_t i;
       for(i = 0; i < (uint64_t)stride * height; i++) {
               buffer[i] = 0x00000000;
       }

       if(auto_present && back != NULL) {
	       screen_present();
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
