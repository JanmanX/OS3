#pragma once

#include <stdint.h>


/* There are 256 interrupts [0;255], so lets use them all. More are ignored. */
#define IDT_NUM_ENTRIES		256

/* IDT Entry */
typedef struct idt_entry {
	uint16_t offset0;
	uint16_t selector;
	uint8_t ist; /* bit 0 and 1 used. Rest is reserved */

#define IDTE_PRESENT		(1 << 7)
#define IDTE_DPL_RING0		(0 << 5)
#define IDTE_DPL_RING1		(1 << 5)
#define IDTE_DPL_RING2		(2 << 5)
#define IDTE_DPL_RING3		(3 << 5)

/* Difference between TRAP and INTERRUPT gates are that interrupt gates clear
 * the IF bit in EFLAGS, disabling interrupts. */
#define IDTE_TYPE_TRAP_32	(0xF)
#define IDTE_TYPE_INTERRUPT_32  (0xE)
	uint8_t type_attr;
	uint16_t offset1;
	uint32_t offset2;
	uint32_t zero;
} __attribute__((packed)) idt_entry_t;


/* IDT Pointer */
typedef struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) idt_pointer_t;


void idt_load();
void idt_init();
void idt_install_gate(uint32_t num, uint64_t offset, uint16_t selector,
		      uint8_t ist, uint8_t type_attr);


