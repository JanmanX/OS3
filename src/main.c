#include <libc.h>
#include <multiboot_parser.h>
#include <multiboot2.h>
#include <drivers/screen.h>
#include <mem/mem.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <kernel/interrupt.h>
#include <kernel/acpi.h>
#include <spinlock.h>
#include <drivers/ps2mouse.h>
#include <drivers/ps2keyboard.h>
#include <drivers/pci.h>
#include <drivers/rtc.h>
#include <test/test.h>
#include <cpu/hpet.h>

int main(uint64_t mb_info_struct_addr)
{
	/* Initialize Multiboot info struct parser */
	multiboot_parser_init((uint8_t*)mb_info_struct_addr);

	/* Initialize screen */
	screen_init();

	/* Initialize memory systems */
	mem_init();

	/* ACPI Init */
	acpi_init();

	/* Initialize GDT */
	gdt_init();

	clear_screen();

	/* Initialize Interrupts */
	interrupt_init();

	/* Keyboard :) */
	ps2keyboard_init();

	/* Time subsystem */
	time_init();


	pci_list();



	/* Done initializing */
	LOG("Done initializing.\n");

	/* Stop the machine */
	asm __volatile__("hlt");
	for(;;) {
	}

	return 0;
}
