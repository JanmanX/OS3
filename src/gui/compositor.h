#pragma once

#include <stdint.h>
#include "gfx.h"

/*
 * A very small compositor.
 *
 * Everything is drawn into an off-screen buffer and only the rectangles that
 * actually changed are copied to the framebuffer, so moving the pointer costs
 * two small blits rather than a full-screen redraw and never shows a half
 * drawn frame.
 */

#define COMPOSITOR_MAX_WINDOWS	(8)
#define COMPOSITOR_MAX_DAMAGE	(16)

/* Height of the bar along the top of the screen */
#define COMPOSITOR_PANEL_HEIGHT	(22)

typedef struct gui_window {
	gfx_rect_t frame;
	const char *title;
	uint32_t accent;
	uint8_t visible;
} gui_window_t;

/* Allocates the back buffer, hands it to the screen driver and paints the
 * first frame. Returns EOK, or ENOMEM when the buffer would not fit. */
uint8_t compositor_init(void);

/* Windows are painted back to front in the order they were added */
int32_t compositor_add_window(gfx_rect_t frame, const char *title,
			      uint32_t accent);

/* Mark a region as needing to be repainted and presented */
void compositor_damage(gfx_rect_t r);
void compositor_damage_all(void);

/* Repaint and present whatever is damaged. Cheap when nothing is. */
void compositor_frame(void);

/* Composite forever, following the pointer. Does not return. */
void compositor_run(void);
