#pragma once

#include <stdint.h>

#define PS2_MOUSE_IRQ		(0x0C)
#define PS2_IRQ			PS2_MOUSE_IRQ	/* Kept for older callers */

/* Commands understood by the device itself, sent through the aux port */
#define PS2_MOUSE_RESET		(0xFF)
#define PS2_MOUSE_SET_DEFAULTS	(0xF6)
#define PS2_MOUSE_ENABLE	(0xF4)
#define PS2_MOUSE_DISABLE	(0xF5)
#define PS2_MOUSE_SET_RATE	(0xF3)
#define PS2_MOUSE_GET_ID	(0xF2)

/* First byte of a movement packet */
#define PS2_MOUSE_PKT_LEFT	(0x01)
#define PS2_MOUSE_PKT_RIGHT	(0x02)
#define PS2_MOUSE_PKT_MIDDLE	(0x04)
#define PS2_MOUSE_PKT_ALWAYS_1	(0x08)
#define PS2_MOUSE_PKT_X_SIGN	(0x10)
#define PS2_MOUSE_PKT_Y_SIGN	(0x20)
#define PS2_MOUSE_PKT_X_OVF	(0x40)
#define PS2_MOUSE_PKT_Y_OVF	(0x80)

/* Button bits handed out by ps2mouse_get_state() */
#define PS2_MOUSE_BUTTON_LEFT	(0x01)
#define PS2_MOUSE_BUTTON_RIGHT	(0x02)
#define PS2_MOUSE_BUTTON_MIDDLE	(0x04)

typedef struct ps2mouse_state {
	int32_t x;		/* Absolute position, already clamped */
	int32_t y;
	uint8_t buttons;	/* PS2_MOUSE_BUTTON_* bits */
	uint64_t packets;	/* Complete packets seen since boot */
} ps2mouse_state_t;

uint8_t ps2mouse_init(void);

/* Consistent snapshot of the pointer. Safe to call with interrupts on. */
void ps2mouse_get_state(ps2mouse_state_t *out);

/* Area the pointer is confined to. Defaults to the screen dimensions. */
void ps2mouse_set_bounds(uint32_t width, uint32_t height);

/* Place the pointer somewhere specific, e.g. the middle of the screen */
void ps2mouse_set_position(int32_t x, int32_t y);

uint8_t ps2mouse_is_present(void);

/* Interrupts that ran out of drain budget. Should stay at zero. */
uint64_t ps2mouse_get_drain_overruns(void);
