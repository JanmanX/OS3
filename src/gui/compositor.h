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

	/* Both optional. paint() draws the window's content over its body,
	 * already clipped to whatever is being repainted. click() gets the
	 * screen position of a left button press that landed on the window. */
	void (*paint)(const struct gui_window *w);
	void (*click)(struct gui_window *w, int32_t x, int32_t y);
} gui_window_t;

/* Height of the title bar; content starts below it */
#define COMPOSITOR_TITLE_HEIGHT	(18)

/* Allocates the back buffer, hands it to the screen driver and paints the
 * first frame. Returns EOK, or ENOMEM when the buffer would not fit. */
uint8_t compositor_init(void);

/* Windows are painted back to front in the order they were added */
int32_t compositor_add_window(gfx_rect_t frame, const char *title,
			      uint32_t accent);

/* Hook a window up to its content. "id" is what compositor_add_window()
 * returned. */
void compositor_set_handlers(int32_t id,
			     void (*paint)(const gui_window_t *w),
			     void (*click)(gui_window_t *w, int32_t x, int32_t y));

/* Mark a region as needing to be repainted and presented */
void compositor_damage(gfx_rect_t r);
void compositor_damage_all(void);

/* Repaint and present whatever is damaged. Cheap when nothing is. */
void compositor_frame(void);

/* Composite forever, following the pointer. Does not return. */
void compositor_run(void);
