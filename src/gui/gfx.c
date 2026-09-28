#include "gfx.h"
#include <stdint.h>
#include <types.h>
#include <errno.h>
#include <drivers/screen.h>
#include <drivers/font.h>

static uint32_t *target = NULL;
static int32_t target_w = 0;
static int32_t target_h = 0;

/* Pixels between the start of one row and the next. The buffer matches the
 * framebuffer's geometry, so this can be wider than the visible area. */
static int32_t target_stride = 0;

/* Current clip rectangle, always already intersected with the buffer */
static gfx_rect_t clip = {0, 0, 0, 0};


uint8_t gfx_init(void)
{
	target = screen_get_backbuffer();

	if(target == NULL) {
		return ENODEV;
	}

	target_w = (int32_t)screen_get_width();
	target_h = (int32_t)screen_get_height();
	target_stride = (int32_t)screen_get_stride_px();

	gfx_reset_clip();

	return EOK;
}

int32_t gfx_width(void)
{
	return target_w;
}

int32_t gfx_height(void)
{
	return target_h;
}

gfx_rect_t gfx_screen_rect(void)
{
	return gfx_rect(0, 0, target_w, target_h);
}


gfx_rect_t gfx_rect(int32_t x, int32_t y, int32_t w, int32_t h)
{
	gfx_rect_t r;

	r.x = x;
	r.y = y;
	r.w = w;
	r.h = h;

	return r;
}

uint8_t gfx_rect_empty(gfx_rect_t r)
{
	return (r.w <= 0 || r.h <= 0) ? 0x01 : 0x00;
}

gfx_rect_t gfx_rect_intersect(gfx_rect_t a, gfx_rect_t b)
{
	int32_t x0 = (a.x > b.x) ? a.x : b.x;
	int32_t y0 = (a.y > b.y) ? a.y : b.y;
	int32_t x1 = ((a.x + a.w) < (b.x + b.w)) ? (a.x + a.w) : (b.x + b.w);
	int32_t y1 = ((a.y + a.h) < (b.y + b.h)) ? (a.y + a.h) : (b.y + b.h);

	if(x1 <= x0 || y1 <= y0) {
		return gfx_rect(0, 0, 0, 0);
	}

	return gfx_rect(x0, y0, x1 - x0, y1 - y0);
}

gfx_rect_t gfx_rect_union(gfx_rect_t a, gfx_rect_t b)
{
	int32_t x0, y0, x1, y1;

	if(gfx_rect_empty(a)) {
		return b;
	}
	if(gfx_rect_empty(b)) {
		return a;
	}

	x0 = (a.x < b.x) ? a.x : b.x;
	y0 = (a.y < b.y) ? a.y : b.y;
	x1 = ((a.x + a.w) > (b.x + b.w)) ? (a.x + a.w) : (b.x + b.w);
	y1 = ((a.y + a.h) > (b.y + b.h)) ? (a.y + a.h) : (b.y + b.h);

	return gfx_rect(x0, y0, x1 - x0, y1 - y0);
}

uint8_t gfx_rect_intersects(gfx_rect_t a, gfx_rect_t b)
{
	return gfx_rect_empty(gfx_rect_intersect(a, b)) ? 0x00 : 0x01;
}


void gfx_set_clip(gfx_rect_t r)
{
	clip = gfx_rect_intersect(r, gfx_rect(0, 0, target_w, target_h));
}

void gfx_reset_clip(void)
{
	clip = gfx_rect(0, 0, target_w, target_h);
}


void gfx_pixel(int32_t x, int32_t y, uint32_t argb)
{
	if(target == NULL) {
		return;
	}

	if(x < clip.x || y < clip.y
	   || x >= clip.x + clip.w || y >= clip.y + clip.h) {
		return;
	}

	target[(int64_t)y * target_stride + x] = argb;
}

void gfx_fill(gfx_rect_t r, uint32_t argb)
{
	int32_t row, col;
	gfx_rect_t a = gfx_rect_intersect(r, clip);

	if(target == NULL || gfx_rect_empty(a)) {
		return;
	}

	for(row = a.y; row < a.y + a.h; row++) {
		uint32_t *line = target + (int64_t)row * target_stride;

		for(col = a.x; col < a.x + a.w; col++) {
			line[col] = argb;
		}
	}
}

void gfx_hline(int32_t x, int32_t y, int32_t w, uint32_t argb)
{
	gfx_fill(gfx_rect(x, y, w, 1), argb);
}

void gfx_vline(int32_t x, int32_t y, int32_t h, uint32_t argb)
{
	gfx_fill(gfx_rect(x, y, 1, h), argb);
}

void gfx_frame(gfx_rect_t r, uint32_t argb)
{
	if(gfx_rect_empty(r)) {
		return;
	}

	gfx_hline(r.x, r.y, r.w, argb);
	gfx_hline(r.x, r.y + r.h - 1, r.w, argb);
	gfx_vline(r.x, r.y, r.h, argb);
	gfx_vline(r.x + r.w - 1, r.y, r.h, argb);
}

void gfx_gradient_v(gfx_rect_t r, uint32_t top, uint32_t bottom)
{
	int32_t row;
	int32_t span = (r.h > 1) ? r.h - 1 : 1;

	int32_t r0 = (top >> 16) & 0xFF;
	int32_t g0 = (top >> 8) & 0xFF;
	int32_t b0 = top & 0xFF;

	int32_t r1 = (bottom >> 16) & 0xFF;
	int32_t g1 = (bottom >> 8) & 0xFF;
	int32_t b1 = bottom & 0xFF;

	gfx_rect_t a = gfx_rect_intersect(r, clip);

	if(gfx_rect_empty(a)) {
		return;
	}

	/* Only the rows inside the clip are touched, but each row's colour is
	 * still derived from its position in the whole gradient, so painting a
	 * strip of it looks identical to painting all of it */
	for(row = a.y; row < a.y + a.h; row++) {
		int32_t t = row - r.y;

		uint32_t cr = (uint32_t)(r0 + ((r1 - r0) * t) / span);
		uint32_t cg = (uint32_t)(g0 + ((g1 - g0) * t) / span);
		uint32_t cb = (uint32_t)(b0 + ((b1 - b0) * t) / span);

		gfx_hline(a.x, row, a.w, GFX_RGB(cr, cg, cb));
	}
}


int32_t gfx_char_width(void)
{
	return (int32_t)font.Width;
}

int32_t gfx_char_height(void)
{
	return (int32_t)font.Height;
}

void gfx_char(unsigned char c, int32_t x, int32_t y, uint32_t argb)
{
	const uint8_t *bitmap = font.Bitmap + (uint64_t)(c * font.Height);
	uint32_t cx, cy;

	for(cy = 0; cy < font.Height; cy++) {
		for(cx = 0; cx < font.Width; cx++) {
			/* Unset pixels stay transparent - the text sits on top
			 * of whatever the compositor already drew */
			if(bitmap[cy] & (0x80 >> cx)) {
				gfx_pixel(x + (int32_t)cx, y + (int32_t)cy, argb);
			}
		}
	}
}

void gfx_string(const char *s, int32_t x, int32_t y, uint32_t argb)
{
	if(s == NULL) {
		return;
	}

	while(*s) {
		gfx_char((unsigned char)*s, x, y, argb);
		x += gfx_char_width();
		s++;
	}
}
