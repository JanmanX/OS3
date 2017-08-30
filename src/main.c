#include <libc.h>
#include <multiboot_parser.h>
#include <multiboot2.h>
#include <drivers/screen.h>
#include <mem/mem.h>
#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <kernel/interrupt.h>

int main(uint64_t mb_info_struct_addr)
{
	/* Initialize Multiboot info struct parser */
	multiboot_parser_init((uint8_t*)mb_info_struct_addr);

	/* Initialize screen */
	screen_init();

	/* Install identity-pages */
	paging_init();

	/* Initialize GDT */
	gdt_init();

	/* Initialize Interrupts */
	interrupt_init();

	/* ACPI Init */
	acpi_init();

	/* Done initializing */
	LOG("Done initializing.\n");

	/* Stop the machine */
	asm __volatile__("hlt");
	for(;;) {
	}

	return 0;
}
