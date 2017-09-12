#pragma once
#include <stdint.h>


/* MADT Entries */
#define MADT_ENTRY_TYPE_LAPIC	(0x00)
#define MADT_ENTRY_TYPE_IO_APIC	(0x01)
#define MADT_ENTRY_TYPE_ISO	(0x02)

/* LAPIC */
/* LAPIC Address space register offsets */
#define LAPIC_REGISTER_SIVR	(0x0F0)	/* Spurious Interrupt Vector Register */

void lapic_init(void);

/* IO APIC */
#define IOAPIC_SELECT_OFFSET	(0x00)
#define IOAPIC_WINDOW_OFFSET	(0x10)

void ioapic_init(void);
void ioapic_write(uint32_t reg, uint32_t data);
uint32_t ioapic_read(uint32_t addr);
void ioapic_set_irq(uint8_t irq, uint64_t apic_id, uint8_t vector);

void apic_init(void);





