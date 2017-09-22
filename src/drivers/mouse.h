#pragma once

#define PS2_IRQ			(0x0C)

#define PS2_MOUSE_PREFIX	(0xD4)
#define PS2_MOUSE_PORT		(0x60)
#define PS2_MOUSE_RESET		(0xFF)
#define PS2_MOUSE_ENABLE	(0xF4)
void mouse_init(void);
