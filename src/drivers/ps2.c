#include "ps2.h"
#include <stdint.h>
#include <errno.h>
#include <libc.h>


/* Spin until the controller has room for another byte from us */
static uint8_t ps2_wait_writable(void)
{
	uint64_t timer = PS2_TIMEOUT;

	while(timer--) {
		if((inb(PS2_STATUS) & PS2_STATUS_INPUT_FULL) == 0) {
			return EOK;
		}
		io_wait();
	}

	return EBUSY;
}

/* Spin until the controller has a byte for us */
static uint8_t ps2_wait_readable(void)
{
	uint64_t timer = PS2_TIMEOUT;

	while(timer--) {
		if(inb(PS2_STATUS) & PS2_STATUS_OUTPUT_FULL) {
			return EOK;
		}
		io_wait();
	}

	return EBUSY;
}


/* Reads the data port specifically - the controller hands every device reply
 * back through it, whichever port the command went out on. */
uint8_t ps2_read_poll(uint8_t *out)
{
	if(ps2_wait_readable() != EOK) {
		return EBUSY;
	}

	*out = inb(PS2_DATA);

	return EOK;
}

uint8_t ps2_write_poll(uint8_t port, uint8_t data)
{
	if(port != PS2_COMMAND && port != PS2_DATA) {
		LOG("PS2 invalid port.");
		return EINVAL;
	}

	if(ps2_wait_writable() != EOK) {
		return EBUSY;
	}

	/* Exactly one write. Writing the byte repeatedly - as this used to do -
	 * queues a command several times over and desynchronises the device */
	outb(port, data);

	return EOK;
}


uint8_t ps2_read(uint8_t port)
{
	if(port != PS2_COMMAND && port != PS2_DATA) {
		LOG("PS2 invalid port.");
		return 0x00;
	}

	if(ps2_wait_readable() != EOK) {
		return 0x00;
	}

	/* Read the port that was asked for. Going through ps2_read_poll here
	 * would quietly turn a status read into a data read. */
	return inb(port);
}

void ps2_write(uint8_t port, uint8_t data)
{
	ps2_write_poll(port, data);
}


void ps2_flush(void)
{
	uint64_t guard = PS2_TIMEOUT;

	while((inb(PS2_STATUS) & PS2_STATUS_OUTPUT_FULL) && guard--) {
		inb(PS2_DATA);
		io_wait();
	}
}


uint8_t ps2_read_config(uint8_t *config)
{
	if(ps2_write_poll(PS2_COMMAND, PS2_CMD_READ_CONFIG) != EOK) {
		return EBUSY;
	}

	return ps2_read_poll(config);
}

uint8_t ps2_write_config(uint8_t config)
{
	if(ps2_write_poll(PS2_COMMAND, PS2_CMD_WRITE_CONFIG) != EOK) {
		return EBUSY;
	}

	return ps2_write_poll(PS2_DATA, config);
}
