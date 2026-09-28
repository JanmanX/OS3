#include "compositor.h"
#include "gfx.h"
#include "cursor.h"
#include <stdint.h>
#include <types.h>
#include <errno.h>
#include <libc.h>
#include <mem/mem.h>
#include <drivers/screen.h>
#include <drivers/ps2mouse.h>

/* Desktop colours */
#define DESKTOP_TOP		GFX_RGB(0x1B, 0x2B, 0x44)
#define DESKTOP_BOTTOM		GFX_RGB(0x0B, 0x10, 0x1A)
#define PANEL_BACKGROUND	GFX_RGB(0x14, 0x18, 0x22)
#define PANEL_EDGE		GFX_RGB(0x2E, 0x36, 0x48)
#define PANEL_TEXT		GFX_RGB(0xD8, 0xDE, 0xEA)
#define PANEL_DIM		GFX_RGB(0x7C, 0x88, 0xA0)

#define WINDOW_BODY		GFX_RGB(0x21, 0x27, 0x33)
#define WINDOW_EDGE		GFX_RGB(0x3C, 0x45, 0x58)
#define WINDOW_TITLE_TEXT	GFX_RGB(0xF0, 0xF3, 0xF8)
#define WINDOW_TITLE_HEIGHT	(18)

/* The pointer readout lives at the right-hand end of the panel */
#define READOUT_WIDTH		(280)

static gui_window_t windows[COMPOSITOR_MAX_WINDOWS];
static int32_t window_count = 0;

static gfx_rect_t damage[COMPOSITOR_MAX_DAMAGE];
static int32_t damage_count = 0;

/* Pointer as the compositor last drew it */
static int32_t cursor_x = 0;
static int32_t cursor_y = 0;
static uint8_t cursor_buttons = 0x00;

/* What the readout shows, rebuilt whenever the pointer moves */
static char readout[96];


/* ------------------------------------------------------------------ text */

static char *append_str(char *p, const char *s)
{
	while(*s) {
		*p++ = *s++;
	}

	return p;
}

static char *append_int(char *p, int64_t n)
{
	char buf[24];

	itoa(buf, 'd', n);

	return append_str(p, buf);
}

static void readout_build(int32_t x, int32_t y, uint8_t buttons,
			  uint64_t packets)
{
	char *p = readout;

	/* Say so rather than showing a pointer that will never move */
	if(ps2mouse_is_present() == 0x00) {
		p = append_str(p, "no pointer");
		*p = '\0';
		return;
	}

	p = append_str(p, "x:");
	p = append_int(p, x);
	p = append_str(p, " y:");
	p = append_int(p, y);

	p = append_str(p, "  ");
	p = append_str(p, (buttons & PS2_MOUSE_BUTTON_LEFT)   ? "L" : "-");
	p = append_str(p, (buttons & PS2_MOUSE_BUTTON_MIDDLE) ? "M" : "-");
	p = append_str(p, (buttons & PS2_MOUSE_BUTTON_RIGHT)  ? "R" : "-");

	p = append_str(p, "  pkt:");
	p = append_int(p, (int64_t)packets);

	*p = '\0';
}


/* ----------------------------------------------------------------- paint */

static gfx_rect_t panel_rect(void)
{
	return gfx_rect(0, 0, gfx_width(), COMPOSITOR_PANEL_HEIGHT);
}

static gfx_rect_t readout_rect(void)
{
	return gfx_rect(gfx_width() - READOUT_WIDTH, 0,
			READOUT_WIDTH, COMPOSITOR_PANEL_HEIGHT);
}

static void paint_panel(void)
{
	gfx_rect_t r = panel_rect();
	int32_t text_y = (COMPOSITOR_PANEL_HEIGHT - gfx_char_height()) / 2;

	gfx_fill(r, PANEL_BACKGROUND);
	gfx_hline(r.x, r.y + r.h - 1, r.w, PANEL_EDGE);

	gfx_string("OS3", 8, text_y, PANEL_TEXT);
	gfx_string("compositor", 8 + 4 * gfx_char_width(), text_y, PANEL_DIM);

	gfx_string(readout, readout_rect().x, text_y, PANEL_TEXT);
}

static void paint_window(const gui_window_t *w)
{
	gfx_rect_t title;
	gfx_rect_t body;

	if(w->visible == 0x00 || gfx_rect_empty(w->frame)) {
		return;
	}

	title = gfx_rect(w->frame.x, w->frame.y,
			 w->frame.w, WINDOW_TITLE_HEIGHT);
	body = gfx_rect(w->frame.x, w->frame.y + WINDOW_TITLE_HEIGHT,
			w->frame.w, w->frame.h - WINDOW_TITLE_HEIGHT);

	gfx_fill(body, WINDOW_BODY);
	gfx_fill(title, w->accent);
	gfx_frame(w->frame, WINDOW_EDGE);

	gfx_string(w->title,
		   title.x + 6,
		   title.y + (WINDOW_TITLE_HEIGHT - gfx_char_height()) / 2,
		   WINDOW_TITLE_TEXT);
}

/* Redraw the whole scene, clipped to one rectangle */
static void paint_region(gfx_rect_t area)
{
	int32_t i;
	gfx_rect_t a = gfx_rect_intersect(area, gfx_screen_rect());

	if(gfx_rect_empty(a)) {
		return;
	}

	gfx_set_clip(a);

	gfx_gradient_v(gfx_screen_rect(), DESKTOP_TOP, DESKTOP_BOTTOM);

	for(i = 0; i < window_count; i++) {
		if(gfx_rect_intersects(windows[i].frame, a)) {
			paint_window(&windows[i]);
		}
	}

	if(gfx_rect_intersects(panel_rect(), a)) {
		paint_panel();
	}

	gfx_reset_clip();
}


/* ---------------------------------------------------------------- damage */

void compositor_damage(gfx_rect_t r)
{
	int32_t i;
	gfx_rect_t a = gfx_rect_intersect(r, gfx_screen_rect());

	if(gfx_rect_empty(a)) {
		return;
	}

	/* A pointer that moved a few pixels leaves an old and a new rectangle
	 * that mostly overlap. Folding them together repaints and presents the
	 * shared pixels once instead of twice. */
	for(i = 0; i < damage_count; i++) {
		if(gfx_rect_intersects(damage[i], a)) {
			damage[i] = gfx_rect_union(damage[i], a);
			return;
		}
	}

	/* Out of slots: one full-screen repaint is cheaper than tracking
	 * more rectangles than this */
	if(damage_count >= COMPOSITOR_MAX_DAMAGE) {
		compositor_damage_all();
		return;
	}

	damage[damage_count++] = a;
}

void compositor_damage_all(void)
{
	damage[0] = gfx_screen_rect();
	damage_count = 1;
}


/* ----------------------------------------------------------------- frame */

void compositor_frame(void)
{
	int32_t i;

	if(damage_count == 0) {
		return;
	}

	/* Erase: repaint the desktop under everything that changed */
	for(i = 0; i < damage_count; i++) {
		paint_region(damage[i]);
	}

	/* The pointer is always on top and never part of a window */
	cursor_draw(cursor_x, cursor_y, cursor_buttons);

	/* Present: only the rectangles that changed reach the framebuffer */
	for(i = 0; i < damage_count; i++) {
		screen_present_rect((uint32_t)damage[i].x,
				    (uint32_t)damage[i].y,
				    (uint32_t)damage[i].w,
				    (uint32_t)damage[i].h);
	}

	damage_count = 0;
}


/* ------------------------------------------------------------------ init */

int32_t compositor_add_window(gfx_rect_t frame, const char *title,
			      uint32_t accent)
{
	int32_t id;

	if(window_count >= COMPOSITOR_MAX_WINDOWS) {
		return -1;
	}

	id = window_count++;

	windows[id].frame = frame;
	windows[id].title = title;
	windows[id].accent = accent;
	windows[id].visible = 0x01;

	compositor_damage(frame);

	return id;
}

uint8_t compositor_init(void)
{
	uint64_t pixels = (uint64_t)screen_get_width() * screen_get_height();
	uint64_t bytes = screen_get_buffer_size();
	uint32_t *back = NULL;
	ps2mouse_state_t mouse;

	if(pixels == 0) {
		LOG("Compositor: no framebuffer to composite onto");
		return ENODEV;
	}

	LOGF("Compositor: back buffer %d bytes for %dx%d (stride %d)\n",
	     (uint64_t)bytes,
	     (uint64_t)screen_get_width(),
	     (uint64_t)screen_get_height(),
	     (uint64_t)screen_get_stride_px());

	back = (uint32_t*)malloc(bytes);

	if(back == NULL) {
		LOG("Compositor: could not allocate the back buffer");
		return ENOMEM;
	}

	memset(back, 0x00, bytes);

	if(screen_set_backbuffer(back) != EOK) {
		LOG("Compositor: screen refused the back buffer");
		free(back);
		return ENODEV;
	}

	if(gfx_init() != EOK) {
		LOG("Compositor: gfx would not bind to the back buffer");
		screen_set_backbuffer(NULL);
		free(back);
		return ENODEV;
	}

	/* From here the framebuffer only changes when we present, and the
	 * log lives on the serial console alone */
	screen_set_auto_present(0x00);
	kprintf_set_screen_enabled(0x00);

	/* Keep the pointer inside the screen */
	ps2mouse_set_bounds(screen_get_width(), screen_get_height());
	ps2mouse_get_state(&mouse);

	cursor_x = mouse.x;
	cursor_y = mouse.y;
	cursor_buttons = mouse.buttons;

	readout_build(cursor_x, cursor_y, cursor_buttons, mouse.packets);

	/* Something to composite over */
	compositor_add_window(gfx_rect(60, 70, 300, 170), "welcome",
			      GFX_RGB(0x2F, 0x6F, 0xB8));
	compositor_add_window(gfx_rect(300, 200, 330, 190), "pointer",
			      GFX_RGB(0x8A, 0x46, 0x9B));

	compositor_damage_all();
	compositor_frame();

	LOGF("Compositor: back buffer at 0x%x size 0x%x\n",
	     (uint64_t)back, screen_get_buffer_size());

	LOG("Compositor: running");

	/* Report where the pointer starts, so that anything watching the log
	 * has a baseline to compare later movement against */
	LOGF("gui: pointer %d,%d buttons 0x%x\n",
	     (int64_t)cursor_x,
	     (int64_t)cursor_y,
	     (uint64_t)cursor_buttons);

	return EOK;
}


void compositor_run(void)
{
	ps2mouse_state_t mouse;
	uint64_t last_packets = 0;
	uint64_t last_overruns = 0;

	ps2mouse_get_state(&mouse);
	last_packets = mouse.packets;
	last_overruns = ps2mouse_get_drain_overruns();

	for(;;) {
		ps2mouse_get_state(&mouse);

		if(mouse.packets != last_packets) {
			uint64_t overruns = ps2mouse_get_drain_overruns();

			last_packets = mouse.packets;

			/* Worth saying out loud: it means packets are arriving
			 * faster than one interrupt can collect them */
			if(overruns != last_overruns) {
				last_overruns = overruns;
				LOGF("gui: mouse drain overruns: %d\n",
				     (uint64_t)overruns);
			}

			if(mouse.x != cursor_x || mouse.y != cursor_y
			   || mouse.buttons != cursor_buttons) {

				/* Where the pointer was has to be repainted,
				 * and where it is going has to be presented */
				compositor_damage(cursor_bounds(cursor_x,
								cursor_y));

				cursor_x = mouse.x;
				cursor_y = mouse.y;
				cursor_buttons = mouse.buttons;

				compositor_damage(cursor_bounds(cursor_x,
								cursor_y));

				readout_build(cursor_x, cursor_y,
					      cursor_buttons, mouse.packets);
				compositor_damage(readout_rect());

				LOGF("gui: pointer %d,%d buttons 0x%x\n",
				     (int64_t)cursor_x,
				     (int64_t)cursor_y,
				     (uint64_t)cursor_buttons);
			}
		}

		compositor_frame();

		/*
		 * Sleep until something happens. The check for new packets is
		 * made with interrupts off because a packet landing between an
		 * enabled-interrupt check and the HLT would otherwise not be
		 * noticed until the packet after it, leaving the pointer a
		 * frame behind wherever the mouse came to rest.
		 *
		 * In practice the SCI and the other devices interrupt often
		 * enough to hide that, so this is about not depending on them:
		 * on a quiet machine the plain HLT is what would strand the
		 * cursor.
		 */
		CLI;

		ps2mouse_get_state(&mouse);

		if(mouse.packets != last_packets) {
			STI;
			continue;
		}

		/* STI keeps interrupts blocked for one more instruction, so
		 * this pair cannot miss the wakeup it is waiting for */
		asm volatile("sti; hlt");
	}
}
