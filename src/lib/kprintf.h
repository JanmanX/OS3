#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <drivers/screen.h>

void write_char(uint8_t c);
void kprint(char *str);
void kprintf(const char *fmt, ...);
void clear_screen();

/*
 * Whether the log is also painted onto the screen. A compositor owns the
 * framebuffer once it starts, so it turns this off and lets the serial
 * console carry the log on its own.
 */
void kprintf_set_screen_enabled(uint8_t on);

