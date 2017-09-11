#include "pic.h"
#include <libc.h>

void pic_init(void)
{
	uint8_t mask1;
	uint8_t mask2;

	/* Get masks */
	mask1 = inb(PIC1_DATA);
	mask2 = inb(PIC2_DATA);

	/* Init in cascade mode */
	outb(PIC1_COMMAND, ICW1_INIT+ICW1_ICW4);
	outb(PIC2_COMMAND, ICW1_INIT+ICW1_ICW4);

	/* Set PIC1 to 32-39 and PIC2 to 40-47 */
	outb(PIC1_DATA, 0x20);
	outb(PIC2_DATA, 0x28);

	/* Connect master and slave */
	outb(PIC1_DATA, 0x04); /* PIC2 connected at pin 2 (0x04) */
	outb(PIC2_DATA, 0x02);

	/* ICW4, send system mode*/
	outb(PIC1_DATA, ICW4_8086);
	outb(PIC2_DATA, ICW4_8086);

	/* Restore masks */
	outb(PIC1_DATA, mask1);
	outb(PIC2_DATA, mask2);
}


void pic_disable(void)
{
	LOG("Disabling PIC");
	/*
	   mov al, 0xff
	   out 0xa1, al
	   out 0x21, al
	   */
	outb(0xa1, 0xff);
	outb(0x21, 0xff);

	LOG("PIC disabled");

}
