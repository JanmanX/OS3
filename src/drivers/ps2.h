#pragma once

#include <stdint.h>

#define PS2_DATA	(0x60)
#define PS2_COMMAND	(0x64)
#define PS2_STATUS	(0x64)

/* Status register bits */
#define PS2_STATUS_OUTPUT_FULL	(0x01)	/* Data waiting to be read */
#define PS2_STATUS_INPUT_FULL	(0x02)	/* Controller still busy */
#define PS2_STATUS_FROM_AUX	(0x20)	/* Byte came from the second port */

/* Controller commands (written to PS2_COMMAND) */
#define PS2_CMD_READ_CONFIG	(0x20)
#define PS2_CMD_WRITE_CONFIG	(0x60)
#define PS2_CMD_DISABLE_AUX	(0xA7)
#define PS2_CMD_ENABLE_AUX	(0xA8)
#define PS2_CMD_DISABLE_KB	(0xAD)
#define PS2_CMD_ENABLE_KB	(0xAE)
#define PS2_CMD_WRITE_AUX	(0xD4)	/* Next data byte goes to the mouse */

/* Configuration byte bits */
#define PS2_CONFIG_KB_IRQ	(0x01)
#define PS2_CONFIG_AUX_IRQ	(0x02)
#define PS2_CONFIG_KB_CLOCK_OFF	(0x10)
#define PS2_CONFIG_AUX_CLOCK_OFF (0x20)
#define PS2_CONFIG_TRANSLATE	(0x40)

#define PS2_TIMEOUT		(0xFFFF)
#define PS2_ACK			(0xFA)
#define PS2_RESEND		(0xFE)
#define PS2_SELF_TEST_OK	(0xAA)

uint8_t ps2_read(uint8_t port);
void ps2_write(uint8_t port, uint8_t data);

/* Timeout-aware variants. Both return EOK, or ETIMEDOUT-ish EBUSY when the
 * controller never became ready. ps2_read_poll always reads the data port,
 * which is where device replies arrive. */
uint8_t ps2_read_poll(uint8_t *out);
uint8_t ps2_write_poll(uint8_t port, uint8_t data);

/* Throw away whatever the controller still has buffered */
void ps2_flush(void);

/* Configuration byte accessors */
uint8_t ps2_read_config(uint8_t *config);
uint8_t ps2_write_config(uint8_t config);
