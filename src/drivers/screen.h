#pragma once

#include <stdint.h>
#include <multiboot2.h>

static uint32_t *buffer;
static uint32_t width;
static uint32_t height;
static uint32_t position;
static uint8_t bpp;

uint8_t screen_init(struct multiboot_tag_framebuffer* mb_fb);
static void put_pixel(uint32_t x, uint32_t y, uint32_t argb);

