#pragma once

/* MADT Entries */
#define MADT_ENTRY_TYPE_LAPIC	(0x00)
#define MADT_ENTRY_TYPE_IO_APIC	(0x01)
#define MADT_ENTRY_TYPE_ISO	(0x02)

/* LAPIC Address space register offsets */
#define LAPIC_REGISTER_SIVR	(0x0F0)	/* Spurious Interrupt Vector Register */

void apic_init(void);


