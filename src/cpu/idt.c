#include "idt.h"
#include <stdint.h>
#include <libc.h>


/* The IDT */
idt_entry_t idt[IDT_NUM_ENTRIES] __attribute__((aligned(8)));

/* IDT Pointer */
idt_pointer_t idt_ptr __attribute__((aligned(8)));


void idt_load(uint64_t idt_ptr)
{
	asm volatile("lidt (%%rax)"
		     :				/* Output operands */
		     : "a"(idt_ptr));		/* Input operands */
}

void idt_init()
{
	/* Clear the IDT */
	memset((uint8_t*)idt, 0x00, sizeof(idt_entry_t) * IDT_NUM_ENTRIES);


	/* Setup IDT Pointer */
	idt_ptr.limit = (uint16_t)((IDT_NUM_ENTRIES * sizeof(idt_entry_t)) - 1);
	idt_ptr.base = (uint64_t)idt;


	/* Reload it */
	idt_load((uint64_t)&idt_ptr);
	BOCHS_DEBUG;
	LOG("IDT LOADED");
}


void idt_install_gate(uint32_t num, uint64_t offset, uint16_t selector,
		      uint8_t ist, uint8_t type_attr)
{
	/* Sanity check */
	if(num >= IDT_NUM_ENTRIES) {
		ERROR("TRYING TO INSTALL IDT GATE OUTSIDE BOUDARIES!");
		return;
	}

	idt[num].offset0 = (uint16_t)(offset & 0xFFFF);
	idt[num].selector = selector;
	idt[num].ist = ist; /* This should always be 0 */
	idt[num].type_attr = type_attr;
	idt[num].offset1 = (uint16_t)((offset >> 16) & 0xFFFF);
	idt[num].offset2 = (uint32_t)((offset >> 32) & 0xFFFFFFFF);
	idt[num].zero = 0;
}


