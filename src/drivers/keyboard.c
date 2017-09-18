#include "keyboard.h"
#include <stdint.h>
#include <libc.h>
#include <cpu/apic.h>
#include <kernel/interrupt.h>
#include <libc.h>
#include <drivers/ps2.h>

/* Globals */
static volatile uint8_t key_down = 0x00;

/* Scan map 1 */
static uint8_t key_map_lower[] = { 0x00, 0, '1', '2', '3', '4', '5', '6', '7',
        '8', '9', '0', '-', '=', 0x0e, 0, 'q', 'w', 'e', 'r', 't', 'y', 'u',
        'i', 'o', 'p', '[', ']', 0x1c, 0, 'a', 's', 'd', 'f', 'g', 'h', 'j',
        'k', 'l', ';', 0x27, '`', 0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm',
        ',', '.', '/', 0, 0, 0, ' ', 0 };

static uint8_t key_map_upper[] = { 0x00, 0, '!', '@', '#', '$', '%', '^', '&',
        '*', '(', ')', '_', '+', 0x0e, 0, 'Q', 'W', 'E', 'R', 'T', 'Y', 'U',
        'I', 'O', 'P', '{', '}', 0x1c, 0, 'A', 'S', 'D', 'F', 'G', 'H', 'J',
        'K', 'L', ':', 0x22, '~', 0, '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M',
        '<', '>', '?', 0, 0, 0, ' ', 0 };


uint8_t keyboard_interrupt_handler(pt_regs_t *regs)
{
	uint8_t code = ps2_read(PS2_KB_COMMAND);

	/* Ignore extended */
	if(code == 0xE0 || code == 0xE1) {
		DEBUG("Extended scan code set!");
		lapic_eoi();
		return;
	}

	if(code & SCAN_RELEASE) {
		key_down = 0;
	} else {
		key_down = 1;
	}

	kprintf("%c", key_map_lower[code]);

	/* send end of interrupt */
	lapic_eoi();
}

uint8_t keyboard_write(uint8_t data)
{
	ps2_write(PS2_KB_COMMAND, data);
	return ps2_read(PS2_KB_COMMAND);
}

void keyboard_init(void)
{
	CLI;

	uint8_t reply = 0x00;

	/* Request IRQ */
	ioapic_set_irq(PS2_KB_IRQ, 0x00, APIC_VECTOR_OFFSET +
		       PS2_KB_IRQ);

	/* Setup interrupt handler */
	interrupt_install(APIC_VECTOR_OFFSET + PS2_KB_IRQ,
			  keyboard_interrupt_handler);


	/* Reset keyboard controller */
#if 0
	uint8_t a = inb(0x61);
	a |= 0x82;
	outb(0x61, a);
	a &= 0x7f;
	outb(0x61, a);
#endif


	/* Initialization */
	do {
		ps2_write(PS2_KB_COMMAND, 0xff);
		io_wait();
		reply = inb(PS2_KB_COMMAND);
		LOGF("kb reply: 0x%x\n", reply);
	/* BOCHS is weird, so check for 0xFA too */
	} while(reply != 0xAA && reply != 0xFA);

	/* Clear the output buffer */
	while(inb(PS2_STATUS) & 0x01) {
		inb(PS2_KB_COMMAND);
	}

	/* Enable interrupts */
	ps2_write(PS2_COMMAND, 0x20);
	uint8_t config = ps2_read(PS2_DATA);
	config |= 0x01; /* Enable IRQ (IRQ 1) */
	ps2_write(PS2_COMMAND, 0x60);
	ps2_write(PS2_DATA, config);

	/* Setting the typematic rate and delay */
	/*
	   do {
	   ps2_write(PS2_KB_COMMAND, PS2_KB_CMD_TYPEMATIC);
	   ps2_write(PS2_KB_COMMAND, 0x00);
	   } while(ps2_read(PS2_KB_COMMAND) != PS2_KB_CMD_RESEND);
	   */

	/* Enable scanning */
	ps2_write(PS2_KB_COMMAND, PS2_KB_CMD_ENABLE_SCAN);

	/* Re-enable interrupts */
	STI;

	LOG("Keyboard installed");
}
