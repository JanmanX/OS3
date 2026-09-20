#pragma once

#include <stdint.h>

#define SERIAL_COM1		(0x3F8)

/* Register offsets from the port base */
#define SERIAL_REG_DATA		(0x00)
#define SERIAL_REG_IER		(0x01)
#define SERIAL_REG_DLL		(0x00)	/* Only with DLAB set */
#define SERIAL_REG_DLM		(0x01)	/* Only with DLAB set */
#define SERIAL_REG_FCR		(0x02)
#define SERIAL_REG_LCR		(0x03)
#define SERIAL_REG_MCR		(0x04)
#define SERIAL_REG_LSR		(0x05)

/* Line status bits */
#define SERIAL_LSR_THRE		(0x20)	/* Transmit holding register empty */

/* How long to spin waiting for the transmitter, so that a machine without
 * a UART cannot wedge the kernel */
#define SERIAL_TX_TIMEOUT	(100000)

uint8_t serial_init(void);
void serial_write_char(uint8_t c);
void serial_write(const char *str);
