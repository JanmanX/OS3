#include "explorer.h"
#include "compositor.h"
#include "gfx.h"
#include <stdint.h>
#include <types.h>
#include <errno.h>
#include <libc.h>
#include <fs/fat.h>

#define WINDOW_MARGIN		(20)
#define WINDOW_MAX_WIDTH	(680)
#define WINDOW_MAX_HEIGHT	(440)

#define TOOLBAR_HEIGHT		(26)
#define BUTTON_WIDTH		(48)
#define BUTTON_HEIGHT		(18)
#define PAD			(6)
#define ROW_HEIGHT		(16)
#define STATUS_HEIGHT		(18)

/* The preview keeps this many lines, of at most PREVIEW_BYTES in all */
#define PREVIEW_LINES		(5)
#define PREVIEW_BYTES		(512)

#define MAX_ROWS		(64)
#define PATH_MAX_LEN		(512)

/* Bytes asked of the file system at a time */
#define READ_CHUNK		(4096)

#define COLOR_BODY		GFX_RGB(0x21, 0x27, 0x33)
#define COLOR_TOOLBAR		GFX_RGB(0x2A, 0x31, 0x40)
#define COLOR_BUTTON		GFX_RGB(0x3A, 0x46, 0x5E)
#define COLOR_BUTTON_OFF	GFX_RGB(0x26, 0x2C, 0x38)
#define COLOR_EDGE		GFX_RGB(0x4A, 0x55, 0x6C)
#define COLOR_TEXT		GFX_RGB(0xE4, 0xE8, 0xF0)
#define COLOR_DIM		GFX_RGB(0x6C, 0x78, 0x90)
#define COLOR_FOLDER		GFX_RGB(0xF2, 0xC2, 0x5C)
#define COLOR_FILE		GFX_RGB(0x8F, 0xB4, 0xE8)
#define COLOR_SELECTED		GFX_RGB(0x2F, 0x4F, 0x7F)
#define COLOR_ROW_ALT		GFX_RGB(0x25, 0x2B, 0x38)

typedef enum { BUTTON_UP, BUTTON_PREV, BUTTON_NEXT, BUTTON_COUNT } button_t;

static const char *button_label[BUTTON_COUNT] = { "Up", "Prev", "Next" };

static int32_t window_id = -1;

static uint8_t mounted = 0;
static char path[PATH_MAX_LEN] = "/";

/* The page on show */
static fat_entry_t rows[MAX_ROWS];
static uint32_t row_count = 0;
static uint32_t page = 0;
static uint32_t total = 0;		/* Entries in the folder, "." and ".." aside */
static uint32_t rows_per_page = 1;
static int32_t selected = -1;		/* Row of the file last opened, or -1 */

/* What the last file click read */
static char status[160] = "";
static char preview[PREVIEW_BYTES + 1];
static uint32_t preview_len = 0;


/* ---------------------------------------------------------------- layout */

static gfx_rect_t button_rect(const gui_window_t *w, button_t b)
{
	return gfx_rect(w->frame.x + PAD + (int32_t)b * (BUTTON_WIDTH + PAD),
			w->frame.y + COMPOSITOR_TITLE_HEIGHT
			+ (TOOLBAR_HEIGHT - BUTTON_HEIGHT) / 2,
			BUTTON_WIDTH, BUTTON_HEIGHT);
}

static int32_t preview_height(void)
{
	return PREVIEW_LINES * gfx_char_height() + 2 * PAD;
}

static gfx_rect_t list_rect(const gui_window_t *w)
{
	int32_t top = w->frame.y + COMPOSITOR_TITLE_HEIGHT + TOOLBAR_HEIGHT;
	int32_t bottom = w->frame.y + w->frame.h
			 - preview_height() - STATUS_HEIGHT - 1;

	return gfx_rect(w->frame.x + 1, top, w->frame.w - 2, bottom - top);
}

static gfx_rect_t row_rect(const gui_window_t *w, uint32_t row)
{
	gfx_rect_t l = list_rect(w);

	return gfx_rect(l.x, l.y + (int32_t)row * ROW_HEIGHT, l.w, ROW_HEIGHT);
}

static gfx_rect_t status_rect(const gui_window_t *w)
{
	return gfx_rect(w->frame.x + 1,
			w->frame.y + w->frame.h - preview_height()
			- STATUS_HEIGHT - 1,
			w->frame.w - 2, STATUS_HEIGHT);
}

static gfx_rect_t preview_rect(const gui_window_t *w)
{
	return gfx_rect(w->frame.x + 1,
			w->frame.y + w->frame.h - preview_height() - 1,
			w->frame.w - 2, preview_height());
}


/* ------------------------------------------------------------- directory */

static uint8_t is_dot(const fat_entry_t *e)
{
	return strcmp(e->name, ".") == 0 || strcmp(e->name, "..") == 0;
}

/* Fill "rows" with the entries of the current page, and count the folder.
 * The page is clamped, so a folder that shrank cannot strand the view. */
static void load_page(void)
{
	fat_dir_t dir;
	fat_entry_t e;
	uint32_t index = 0;
	uint32_t first;

	row_count = 0;
	total = 0;
	selected = -1;

	if(fat_opendir(path, &dir) != EOK) {
		return;
	}

	/* Count first so the page can be clamped before it is read */
	while(fat_readdir(&dir, &e) == EOK) {
		if(!is_dot(&e)) {
			total++;
		}
	}

	if(page * rows_per_page >= total && page > 0) {
		page = (total - 1) / rows_per_page;
	}

	first = page * rows_per_page;
	fat_opendir(path, &dir);

	while(fat_readdir(&dir, &e) == EOK) {
		if(is_dot(&e)) {
			continue;
		}

		if(index >= first && row_count < rows_per_page
		   && row_count < MAX_ROWS) {
			rows[row_count++] = e;
		}

		index++;
	}
}

static uint32_t page_count(void)
{
	return total == 0 ? 1 : (total + rows_per_page - 1) / rows_per_page;
}

static void log_page(void)
{
	uint32_t i;

	LOGF("explorer: dir %s page %d/%d entries %d\n", path,
	     (uint64_t)(page + 1), (uint64_t)page_count(), (uint64_t)total);

	for(i = 0; i < row_count; i++) {
		LOGF("explorer:   %s%s\n", rows[i].name,
		     rows[i].is_dir ? "/" : "");
	}
}

static gfx_rect_t window_frame;

static void damage_window(void)
{
	compositor_damage(window_frame);
}

static void go_to(const char *new_path)
{
	strncpy(path, new_path, PATH_MAX_LEN - 1);
	path[PATH_MAX_LEN - 1] = '\0';

	page = 0;
	status[0] = '\0';
	preview_len = 0;

	load_page();
	log_page();
	damage_window();
}

static void go_up(void)
{
	uint32_t n = (uint32_t)strlen(path);

	if(n <= 1) {
		return;
	}

	/* Trailing separator, then the last component */
	while(n > 1 && path[n - 1] == '/') {
		n--;
	}

	while(n > 1 && path[n - 1] != '/') {
		n--;
	}

	if(n > 1) {
		n--;
	}

	path[n] = '\0';

	go_to(path);
}

static void go_into(const char *name)
{
	char next[PATH_MAX_LEN];
	uint32_t n = (uint32_t)strlen(path);

	if(n + strlen(name) + 2 > PATH_MAX_LEN) {
		return;
	}

	strcpy(next, path);

	if(n > 1) {
		strcat(next, "/");
	}

	strcat(next, name);

	go_to(next);
}


/* ------------------------------------------------------------------ file */

static char *append_str(char *p, const char *s)
{
	while(*s) {
		*p++ = *s++;
	}

	return p;
}

static void open_file(uint32_t row)
{
	char full[PATH_MAX_LEN];
	fat_file_t f;
	uint8_t buf[READ_CHUNK];
	uint32_t hash = 2166136261U;	/* FNV-1a over the whole file */
	uint64_t read = 0;
	uint32_t i;
	uint8_t err;
	char *p;
	char num[24];

	if(strlen(path) + strlen(rows[row].name) + 2 > PATH_MAX_LEN) {
		return;
	}

	strcpy(full, path);

	if(strlen(path) > 1) {
		strcat(full, "/");
	}

	strcat(full, rows[row].name);

	selected = (int32_t)row;
	preview_len = 0;

	err = fat_open(full, &f);

	if(err != EOK) {
		p = append_str(status, "cannot open: error ");
		itoa(num, 'd', err);
		p = append_str(p, num);
		*p = '\0';
		LOGF("explorer: open %s failed: %d\n", full, (uint64_t)err);
		damage_window();
		return;
	}

	for(;;) {
		int64_t n = fat_read(&f, buf, sizeof(buf));

		if(n <= 0) {
			if(n < 0) {
				LOGF("explorer: read %s failed: %d\n", full,
				     (uint64_t)(-n));
			}

			break;
		}

		for(i = 0; i < (uint32_t)n; i++) {
			if(read + i < PREVIEW_BYTES) {
				preview[preview_len++] = (char)buf[i];
			}

			hash = (hash ^ buf[i]) * 16777619U;
		}

		read += (uint64_t)n;
	}

	LOGF("explorer: file %s size %d read %d fnv 0x%x\n", full,
	     (uint64_t)f.size, (uint64_t)read, (uint64_t)hash);

	p = status;
	p = append_str(p, rows[row].name);
	p = append_str(p, "  ");
	itoa(num, 'd', (int64_t)read);
	p = append_str(p, num);
	p = append_str(p, read == f.size ? " bytes" : " bytes (short read)");
	*p = '\0';

	damage_window();
}


/* ----------------------------------------------------------------- paint */

static uint8_t button_enabled(button_t b)
{
	if(!mounted) {
		return 0;
	}

	switch(b) {
	case BUTTON_UP:
		return strlen(path) > 1;
	case BUTTON_PREV:
		return page > 0;
	case BUTTON_NEXT:
		return page + 1 < page_count();
	default:
		return 0;
	}
}

static void paint_button(const gui_window_t *w, button_t b)
{
	gfx_rect_t r = button_rect(w, b);
	uint8_t on = button_enabled(b);
	int32_t tw = (int32_t)strlen(button_label[b]) * gfx_char_width();

	gfx_fill(r, on ? COLOR_BUTTON : COLOR_BUTTON_OFF);
	gfx_frame(r, COLOR_EDGE);
	gfx_string(button_label[b], r.x + (r.w - tw) / 2,
		   r.y + (r.h - gfx_char_height()) / 2,
		   on ? COLOR_TEXT : COLOR_DIM);
}

/* Cut a name down to "max" characters, ending in ".." when it does not fit */
static void fit(const char *name, char *out, uint32_t max)
{
	uint32_t n = (uint32_t)strlen(name);

	if(n <= max) {
		strcpy(out, name);
		return;
	}

	memcpy(out, name, max - 2);
	out[max - 2] = '.';
	out[max - 1] = '.';
	out[max] = '\0';
}

static void paint_rows(const gui_window_t *w)
{
	uint32_t i;
	int32_t cw = gfx_char_width();
	gfx_rect_t l = list_rect(w);
	uint32_t cols = (uint32_t)(l.w / cw);

	for(i = 0; i < rows_per_page; i++) {
		gfx_rect_t r = row_rect(w, i);
		int32_t ty = r.y + (r.h - gfx_char_height()) / 2;
		char name[FAT_NAME_MAX];
		char size[24];

		gfx_fill(r, (int32_t)i == selected ? COLOR_SELECTED
			    : (i & 1) ? COLOR_ROW_ALT : COLOR_BODY);

		if(i >= row_count) {
			continue;
		}

		if(rows[i].is_dir) {
			strcpy(size, "<DIR>");
		} else {
			itoa(size, 'd', rows[i].size);
		}

		/* Marker, name, and the size flush with the right edge */
		gfx_string(rows[i].is_dir ? "[+]" : " - ", r.x + PAD, ty,
			   rows[i].is_dir ? COLOR_FOLDER : COLOR_FILE);

		fit(rows[i].name, name, cols > 16 + 4 ? cols - 16 - 4 : 1);
		gfx_string(name, r.x + PAD + 4 * cw, ty, COLOR_TEXT);

		gfx_string(size, r.x + r.w - PAD - (int32_t)strlen(size) * cw,
			   ty, rows[i].is_dir ? COLOR_DIM : COLOR_TEXT);
	}
}

static void paint_preview(const gui_window_t *w)
{
	gfx_rect_t r = preview_rect(w);
	int32_t cw = gfx_char_width();
	int32_t ch = gfx_char_height();
	int32_t cols = (r.w - 2 * PAD) / cw;
	int32_t line = 0;
	int32_t col = 0;
	uint32_t i;

	gfx_fill(r, GFX_RGB(0x17, 0x1B, 0x24));
	gfx_hline(r.x, r.y, r.w, COLOR_EDGE);

	for(i = 0; i < preview_len && line < PREVIEW_LINES; i++) {
		unsigned char c = (unsigned char)preview[i];

		if(c == '\n') {
			line++;
			col = 0;
			continue;
		}

		if(c == '\r') {
			continue;
		}

		if(c == '\t') {
			c = ' ';
		} else if(c < 0x20 || c > 0x7E) {
			c = '.';
		}

		if(col >= cols) {
			line++;
			col = 0;

			if(line >= PREVIEW_LINES) {
				break;
			}
		}

		gfx_char(c, r.x + PAD + col * cw, r.y + PAD + line * ch,
			 GFX_RGB(0xB8, 0xD8, 0xB0));
		col++;
	}
}

static void paint(const gui_window_t *w)
{
	gfx_rect_t tb = gfx_rect(w->frame.x + 1, w->frame.y
				 + COMPOSITOR_TITLE_HEIGHT,
				 w->frame.w - 2, TOOLBAR_HEIGHT);
	gfx_rect_t st = status_rect(w);
	int32_t cw = gfx_char_width();
	int32_t ty = tb.y + (tb.h - gfx_char_height()) / 2;
	int32_t px = button_rect(w, BUTTON_COUNT - 1).x + BUTTON_WIDTH + PAD * 2;
	char label[PATH_MAX_LEN];
	char pg[32];
	char *p;
	uint32_t room;
	uint32_t b;

	gfx_fill(tb, COLOR_TOOLBAR);

	for(b = 0; b < BUTTON_COUNT; b++) {
		paint_button(w, (button_t)b);
	}

	if(!mounted) {
		gfx_fill(list_rect(w), COLOR_BODY);
		gfx_string("No FAT volume found on any disk.",
			   w->frame.x + PAD,
			   list_rect(w).y + PAD, COLOR_TEXT);
		gfx_fill(st, COLOR_TOOLBAR);
		gfx_fill(preview_rect(w), COLOR_BODY);
		return;
	}

	/* Path, then "page n/m" at the right-hand end when there are several */
	pg[0] = '\0';

	if(page_count() > 1) {
		char num[16];

		p = append_str(pg, "page ");
		itoa(num, 'd', page + 1);
		p = append_str(p, num);
		*p++ = '/';
		itoa(num, 'd', page_count());
		p = append_str(p, num);
		*p = '\0';
	}

	room = (uint32_t)((tb.x + tb.w - px) / cw);
	room = room > strlen(pg) + 3 ? room - (uint32_t)strlen(pg) - 3 : 1;
	fit(path, label, room);
	gfx_string(label, px, ty, COLOR_TEXT);
	gfx_string(pg, tb.x + tb.w - PAD - (int32_t)strlen(pg) * cw, ty,
		   COLOR_DIM);

	paint_rows(w);

	gfx_fill(st, COLOR_TOOLBAR);
	gfx_string(status, st.x + PAD,
		   st.y + (st.h - gfx_char_height()) / 2, COLOR_DIM);

	paint_preview(w);
}


/* ----------------------------------------------------------------- click */

static uint8_t inside(gfx_rect_t r, int32_t x, int32_t y)
{
	return gfx_rect_intersects(r, gfx_rect(x, y, 1, 1));
}

static void click(gui_window_t *w, int32_t x, int32_t y)
{
	uint32_t i;
	gfx_rect_t l;

	if(!mounted) {
		return;
	}

	for(i = 0; i < BUTTON_COUNT; i++) {
		if(!inside(button_rect(w, (button_t)i), x, y)) {
			continue;
		}

		if(!button_enabled((button_t)i)) {
			return;
		}

		if(i == BUTTON_UP) {
			go_up();
		} else {
			page += (i == BUTTON_NEXT) ? 1 : (uint32_t)-1;
			status[0] = '\0';
			preview_len = 0;
			load_page();
			log_page();
			damage_window();
		}

		return;
	}

	l = list_rect(w);

	if(!inside(l, x, y)) {
		return;
	}

	i = (uint32_t)((y - l.y) / ROW_HEIGHT);

	if(i >= row_count) {
		return;
	}

	if(rows[i].is_dir) {
		fat_entry_t entry = rows[i];

		go_into(entry.name);
	} else {
		open_file(i);
	}
}


/* ------------------------------------------------------------------ init */

void explorer_init(void)
{
	gfx_rect_t frame;
	gfx_rect_t list;
	gui_window_t probe;
	int32_t w = gfx_width() - 2 * WINDOW_MARGIN;
	int32_t h = gfx_height() - COMPOSITOR_PANEL_HEIGHT - 2 * WINDOW_MARGIN;
	uint32_t b;

	if(w > WINDOW_MAX_WIDTH) {
		w = WINDOW_MAX_WIDTH;
	}

	if(h > WINDOW_MAX_HEIGHT) {
		h = WINDOW_MAX_HEIGHT;
	}

	frame = gfx_rect(WINDOW_MARGIN, COMPOSITOR_PANEL_HEIGHT + WINDOW_MARGIN,
			 w, h);

	window_id = compositor_add_window(frame, "File Explorer",
					  GFX_RGB(0x2F, 0x6F, 0xB8));

	if(window_id < 0) {
		LOG("Explorer: no room for a window");
		return;
	}

	window_frame = frame;
	probe.frame = frame;
	list = list_rect(&probe);

	rows_per_page = (uint32_t)(list.h / ROW_HEIGHT);

	if(rows_per_page < 1) {
		rows_per_page = 1;
	}

	if(rows_per_page > MAX_ROWS) {
		rows_per_page = MAX_ROWS;
	}

	mounted = (fat_opendir("/", &(fat_dir_t){ 0 }) == EOK);

	if(mounted) {
		load_page();
	}

	compositor_set_handlers(window_id, paint, click);

	/* Where everything is, in screen coordinates, so that whatever drives
	 * the machine does not have to know the layout */
	for(b = 0; b < BUTTON_COUNT; b++) {
		gfx_rect_t r = button_rect(&probe, (button_t)b);

		LOGF("explorer: button %s %d,%d,%d,%d\n", button_label[b],
		     (int64_t)r.x, (int64_t)r.y, (int64_t)r.w, (int64_t)r.h);
	}

	LOGF("explorer: rows %d,%d,%d,%d,%d\n",
	     (int64_t)list.x, (int64_t)list.y, (int64_t)list.w,
	     (int64_t)ROW_HEIGHT, (uint64_t)rows_per_page);

	if(mounted) {
		log_page();
	} else {
		LOG("explorer: no volume");
	}

	LOG("Explorer: running");
}
