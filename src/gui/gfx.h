#pragma once

#include <stdint.h>

/*
 * Drawing primitives for the back buffer.
 *
 * Everything here writes into the buffer the screen driver hands out, never
 * into the framebuffer itself, and everything is clipped twice: once against
 * the current clip rectangle and once against the buffer. That is what lets
 * the compositor repaint an arbitrary sub-rectangle of the desktop by setting
 * a clip and then drawing the whole scene over it.
 */

typedef struct gfx_rect {
	int32_t x;
	int32_t y;
	int32_t w;
	int32_t h;
} gfx_rect_t;

/* 0xAARRGGBB, to match what the framebuffer expects */
#define GFX_RGB(r, g, b)	((uint32_t)(((r) << 16) | ((g) << 8) | (b)))

uint8_t gfx_init(void);

int32_t gfx_width(void);
int32_t gfx_height(void);
gfx_rect_t gfx_screen_rect(void);

/* Rectangle helpers */
gfx_rect_t gfx_rect(int32_t x, int32_t y, int32_t w, int32_t h);
gfx_rect_t gfx_rect_intersect(gfx_rect_t a, gfx_rect_t b);
gfx_rect_t gfx_rect_union(gfx_rect_t a, gfx_rect_t b);
uint8_t gfx_rect_empty(gfx_rect_t r);
uint8_t gfx_rect_intersects(gfx_rect_t a, gfx_rect_t b);

/* Clipping. Draws outside the clip rectangle are dropped. */
void gfx_set_clip(gfx_rect_t r);
void gfx_reset_clip(void);

/* Primitives */
void gfx_pixel(int32_t x, int32_t y, uint32_t argb);
void gfx_fill(gfx_rect_t r, uint32_t argb);
void gfx_frame(gfx_rect_t r, uint32_t argb);
void gfx_hline(int32_t x, int32_t y, int32_t w, uint32_t argb);
void gfx_vline(int32_t x, int32_t y, int32_t h, uint32_t argb);
void gfx_gradient_v(gfx_rect_t r, uint32_t top, uint32_t bottom);

/* Text, drawn from the same bitmap font the console uses. The background is
 * left alone, so text composites over whatever is already there. */
void gfx_char(unsigned char c, int32_t x, int32_t y, uint32_t argb);
void gfx_string(const char *s, int32_t x, int32_t y, uint32_t argb);
int32_t gfx_char_width(void);
int32_t gfx_char_height(void);
