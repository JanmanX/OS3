#pragma once
#include <stdint.h>

/* APIC vector offset */
#define APIC_VECTOR_OFFSET	(0x20)	/* 32. IRQ 0 will be just after reserved
					   exceptions */

/* MADT Entries */
#define MADT_ENTRY_TYPE_LAPIC	(0x00)
#define MADT_ENTRY_TYPE_IO_APIC	(0x01)
#define MADT_ENTRY_TYPE_ISO	(0x02)

/* LAPIC */
/* LAPIC Address space register offsets */
#define LAPIC_SPURIOUS_IRQ	(0xFF)

#define LAPIC_REGISTER_ID	(0x020) /* LAPIC ID Register */
#define LAPIC_REGISTER_VERSION	(0x030) /* LAPIC Version Register  */
#define LAPIC_REGISTER_EOI	(0x0B0) /* End of Interrupt register */
#define LAPIC_REGISTER_SIVR	(0x0F0)	/* Spurious Interrupt Vector Register */


void lapic_init(void);
void lapic_eoi(void);


/* IO APIC */
#define IOAPIC_SELECT_OFFSET	(0x00)
#define IOAPIC_WINDOW_OFFSET	(0x10)

#define IOAPIC_REGISTER_VERSION	(0x01)

void ioapic_init(void);
void ioapic_write(uint32_t reg, uint32_t data);
uint32_t ioapic_read(uint32_t addr);
void ioapic_set_irq(uint8_t irq, uint64_t lapic_id, uint8_t vector);
void ioapic_reset_irq(uint8_t irq);
uint8_t ioapic_is_irq_free(uint8_t irq);
uint8_t ioapic_get_free_irq(void);
uint8_t ioapic_get_num_entries(void);


/* Get the Interrupt Source Override */
uint8_t apic_get_iso(uint8_t irq);


void apic_init(void);





