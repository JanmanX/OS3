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
#include <drivers/ata.h>
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

	/* Initialize Interrupts */
	interrupt_init();

	/* Keyboard :) */
	ps2keyboard_init();

	ata_init();
	pci_list();


	acpi_get_table(ACPI_SIGNATURE_HPET);
	hpet_init();

	/* Done initializing */
	LOG("Done initializing.\n");

	/*
	rtc_init();
	LOGF("Current time: %d:%d:%d, the %d of %d, %d\n",
	     rtc_get_hours(), rtc_get_minutes(), rtc_get_seconds(),
	     rtc_get_day(), rtc_get_month(), 2000 + rtc_get_year());
	*/


	/* Stop the machine */
	asm __volatile__("hlt");
	for(;;) {
	}

	return 0;
}
