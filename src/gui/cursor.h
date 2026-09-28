#pragma once

#include <stdint.h>
#include "gfx.h"

/* The pointer bitmap is an arrow whose tip is its origin, so the hotspot is
 * simply the position it is drawn at */
#define CURSOR_WIDTH	(12)
#define CURSOR_HEIGHT	(18)

/* Where the cursor would land if drawn at x,y - what the compositor damages */
gfx_rect_t cursor_bounds(int32_t x, int32_t y);

/* Draw the pointer into the back buffer. "pressed" picks the highlight fill
 * so that a click is visible without any other feedback. */
void cursor_draw(int32_t x, int32_t y, uint8_t pressed);
