#include "ps2.h"
#include <stdint.h>
#include <libc.h>


uint8_t ps2_read(uint8_t port)
{
	if(port != PS2_COMMAND && port != PS2_DATA) {
		LOG("PS2 invalid port.");
		return 0x00;
	}

	uint64_t timer = PS2_TIMEOUT;

	while(timer--) {
		/* Output buffer is empty */
		if(inb(PS2_STATUS) & 0x01) {
			return inb(port);
		}
		io_wait();
	}

	/* TODO: What to return? */
	return 0x00;
}

void ps2_write(uint8_t port, uint8_t data)
{
	uint64_t timer = PS2_TIMEOUT;

	while(timer--) {
		/* If input buffer is empty */
		if((inb(PS2_STATUS) & 0x02) == 0) {
			outb(port, data);
		}

		io_wait();
	}
}
