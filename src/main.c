#include <libc.h>
#include <multiboot_parser.h>
#include <multiboot2.h>
#include <drivers/screen.h>
#include <mem/mem.h>
#include <init/gdt.h>

int main(unsigned long mb_info_struct_addr)
{
	/* Initialize Multiboot info struct parser */
	multiboot_parser_init(mb_info_struct_addr);

	/* Initialize screen */
	screen_init();

	/* Install identity-pages */
	paging_init();

	/* Initialize GDT */
	gdt_init();


	/* Done initializing */
	kprintf("DONE INITIALIZING.\n");

	asm __volatile__("hlt");
	for(;;) {
	}

	return 0;
}
