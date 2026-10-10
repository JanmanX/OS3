#include "serial.h"
#include <stdint.h>
#include <libc.h>
#include <errno.h>

/* Set once the UART has been configured. Writes before that are dropped
 * rather than poked at an unconfigured port */
static uint8_t initialized = 0;

/* Write a single byte, without any newline translation */
static void serial_put_raw(uint8_t c)
{
	uint32_t timeout = SERIAL_TX_TIMEOUT;

	/* Wait for the transmit holding register to drain */
	while(!(inb(SERIAL_COM1 + SERIAL_REG_LSR) & SERIAL_LSR_THRE)) {
		if(--timeout == 0) {
			return;
		}
	}

	outb(SERIAL_COM1 + SERIAL_REG_DATA, c);
}

uint8_t serial_init(void)
{
	/* Disable interrupts while the UART is reconfigured */
	outb(SERIAL_COM1 + SERIAL_REG_IER, 0x00);

	/* Set DLAB so the divisor latch is reachable */
	outb(SERIAL_COM1 + SERIAL_REG_LCR, 0x80);

	/* Divisor 1, giving 115200 baud */
	outb(SERIAL_COM1 + SERIAL_REG_DLL, 0x01);
	outb(SERIAL_COM1 + SERIAL_REG_DLM, 0x00);

	/* 8 data bits, no parity, one stop bit. Also clears DLAB */
	outb(SERIAL_COM1 + SERIAL_REG_LCR, 0x03);

	/* Enable the FIFOs, clear them, 14 byte trigger level */
	outb(SERIAL_COM1 + SERIAL_REG_FCR, 0xC7);

	/* DTR, RTS and OUT2 */
	outb(SERIAL_COM1 + SERIAL_REG_MCR, 0x0B);

	initialized = 1;

	return EOK;
}

void serial_write_char(uint8_t c)
{
	/* Drop output until the port is up */
	if(!initialized) {
		return;
	}

	/* Terminals want a carriage return with their line feed */
	if(c == '\n') {
		serial_put_raw('\r');
	}

	serial_put_raw(c);
}

void serial_write(const char *str)
{
	uint8_t c;

	while((c = *str++) != 0) {
		serial_write_char(c);
	}
}
