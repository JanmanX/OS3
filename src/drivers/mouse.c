#include "mouse.h"
#include <drivers/ps2.h>
#include <stdint.h>
#include <libc.h>
#include <kernel/interrupt.h>
#include <cpu/apic.h>

uint8_t mouse_interrupt_handler(pt_regs_t *regs)
{
	LOG("Mouse event!");

	/* Clear the output buffer */
	while(inb(PS2_STATUS) & 0x01) {
		inb(PS2_MOUSE_PORT);
	}


	lapic_eoi();
}


uint8_t mouse_write(uint8_t data)
{
	ps2_write(PS2_COMMAND, PS2_MOUSE_PREFIX);

	ps2_write(PS2_MOUSE_PORT, data);

	return ps2_read(PS2_MOUSE_PORT);
}


void mouse_init(void)
{
	LOG("Mouse init");
	uint8_t reply;
	CLI;

	/* Install interrupt gate */
	interrupt_install(PS2_IRQ + APIC_VECTOR_OFFSET,
			  mouse_interrupt_handler);

	/* Setup IRQ in APIC */
	ioapic_set_irq(PS2_IRQ, 0x00, PS2_IRQ + APIC_VECTOR_OFFSET);


	/* Init mouse */
	reply = mouse_write(PS2_MOUSE_RESET);
	LOGF("mouse reset -  0x%x\n", reply);

	/* Enable command */
	reply = mouse_write(PS2_MOUSE_ENABLE);
	LOGF("mouse enable - 0x%x\n", reply);


	/* Enable interrupts */
	ps2_write(PS2_COMMAND, 0x20);
	uint8_t config = ps2_read(PS2_DATA);
	config |= 0x02; /* Enable IRQ (IRQ 12) ((Second channel))*/
	ps2_write(PS2_COMMAND, 0x60);
	ps2_write(PS2_DATA, config);



	STI;
}
