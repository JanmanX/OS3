#pragma once

#include <stdint.h>

#define PS2_DATA	(0x60)
#define PS2_COMMAND	(0x64)
#define PS2_STATUS	(0x64)

#define PS2_TIMEOUT		(0xFFFF)
#define PS2_ACK			(0xFA)
#define PS2_RESEND		(0xFE)
#define PS2_SELF_TEST_OK	(0xAA)

uint8_t ps2_read(uint8_t port);
void ps2_write(uint8_t port, uint8_t data);
