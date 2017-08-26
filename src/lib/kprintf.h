#pragma once

#include <stdint.h>
#include <stdarg.h>
#include <drivers/screen.h>

void write_char(uint8_t c);
void kprint(char *str);
void kprintf(const char *fmt, ...);
void clear_screen();

