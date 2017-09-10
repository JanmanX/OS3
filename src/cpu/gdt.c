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
}

void gdt_init()
{
	/* Clear out the descriptors */
	memset((uint8_t*)gdt, 0x00, GDT_MAX_DESCRIPTORS * sizeof(gdt_entry_t));

	/*		       num       base  limit     access  flags*/
	gdt_install_descriptor(GDT_NULL_INDEX, 0x00, 0x00000000, 0x00, 0x00); /* NULL Descriptor */
	gdt_install_descriptor(GDT_KERNEL_CODE_INDEX, 0x00, 0x00000000, 0x9A, 0x2);
	gdt_install_descriptor(GDT_KERNEL_DATA_INDEX, 0x00, 0x00000000, 0x92, 0x2);
	/* TODO: USER CODE/DATA SEGMENTS HERE */

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

void gdt_install_tss(uint8_t num, uint64_t base, uint64_t limit)
{
	if(num - 1  >= GDT_MAX_DESCRIPTORS) {  /*TSS Entry is size of 2 */
		LOG("GDT Entry num out of bounds");
		return;
	}

	gdt_system_entry_t *tss_entry = (gdt_system_entry_t*)&gdt[num];

	/* TYPE = 0x9 (Available 64-bit TSS) | Present (8) */
	uint8_t type = 0x89;

	/* Type */
	tss_entry->type0 = type;

	/* Limit */
	tss_entry->limit0 = limit & 0xFFFF;
	tss_entry->limit1 = (limit >> 0x10) & 0xF;

	/* Base (Address) */
	tss_entry->base0 = base & 0xFFFF;
	tss_entry->base1 = (base >> 0x10) & 0xFF;
	tss_entry->base2 = (base >> 0x18) & 0xFF;
	tss_entry->base3 = (base >> 0x20);

	/* Other */
	tss_entry->reserved = 0x00;
}
