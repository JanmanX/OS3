#include "gdt.h"
#include <stdint.h>
#include <libc.h>
#include <kprintf.h>

/* The GDT */
static gdt_entry_t gdt[GDT_MAX_DESCRIPTORS] __attribute__ ((aligned (8)));;

/* Pointer to GDT */
static gdt_pointer_t gdt_ptr __attribute__ ((aligned (8)));;


void gdt_load(uint64_t new_gdt_ptr)
{
	asm volatile("lgdt (%%rax)" : : "a"(new_gdt_ptr));

	BOCHS_DEBUG;
}

void gdt_init()
{
	/* Clear out the descriptors */
	memset(gdt, 0x00, GDT_MAX_DESCRIPTORS * sizeof(gdt_entry_t));

	/*		     num  base  limit     access  flags*/
	gdt_install_descriptor(0, 0x00, 0x00000000, 0x00, 0x00); /* NULL Descriptor */
	gdt_install_descriptor(1, 0x00, 0x00000000, 0x9A, 0x2); /* KCODE Descriptor */
	gdt_install_descriptor(2, 0x00, 0x00000000, 0x92, 0x2); /* KDATA Descriptor */



	/* Load the GDT */
	gdt_ptr.limit = (sizeof(gdt_entry_t) * GDT_MAX_DESCRIPTORS) - 1;
	gdt_ptr.base = (uint64_t)gdt;

	/* Load it */
	gdt_load((uint64_t)&gdt_ptr);

	/* Log */
	LOG("GDT LOADED!");
}

void gdt_install_descriptor(uint8_t num, uint32_t base, uint32_t limit,
			    uint8_t access, uint8_t flags)
{
	/* Sanity check */
	if(num >= GDT_MAX_DESCRIPTORS) {
		ERROR("DESCRIPTOR NUMBER TOO HIGH!");

		/* In case ERROR() does return .. */
		return;
	}

	/* Install */
	gdt[num].limit0 = (uint16_t)(limit & 0xFFFF);
	gdt[num].base0 = (uint16_t)(base & 0xFFFF);
	gdt[num].base1 = (uint8_t)((base >> 16) & 0xFF);
	gdt[num].access = access;
	gdt[num].limit1 = (uint8_t)((limit >> 16) & 0x0F);
	gdt[num].flags = flags;
	gdt[num].base2 = (uint8_t)((base >> 24) & 0xFF);
}
