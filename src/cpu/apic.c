#include "apic.h"
#include <acpi/acpica.h>
#include <libc.h>
#include <cpu/pic.h>
#include <stdint.h>
#include <types.h>
#include <libc.h>
#include <kernel/interrupt.h>

/* Multiple APIC Descriptor Table */
struct acpi_table_madt* madt = NULL;

/* LAPIC */
/* LAPIC address (Can be same for multiple processors ) */
uint64_t* lapic_addr = NULL;
uint8_t lapic_num = 0x00;	/* Number of active LAPICS in the system.
				 *  Holds the next value for LAPIC ID.
				 *  Additionally, ID = 0 is bootstrap processor  */


/* IO APIC */
/* TODO: Multiple APICS?*/
uint32_t* ioapic_addr = NULL;


uint8_t spurious_interrupt_handler(pt_regs_t *regs)
{
	/* Do nothing, dont even send an EOI */
	LOG("Spurious interrupt happened!");
}


void lapic_init(void)
{
	ASSERT(lapic_addr != NULL, "lapic_addr == NULL!");

	/* Set the ID */
	uint32_t lapic_id = GET_UINT32(lapic_addr, LAPIC_REGISTER_ID);
	lapic_id &= 0xFFFFFF;		/* Preserve reserved bits 0:23 */

	lapic_id |= (lapic_num << 24);	/* Set top 24-31 bits to ID */
	GET_UINT32(lapic_addr, LAPIC_REGISTER_ID) = lapic_id; /* Write the ID */
	lapic_num++; /* Increment number of LAPICs */

	/* Spuriour Interrupt Vector Register */
	uint32_t SIVr = GET_UINT32(lapic_addr, LAPIC_REGISTER_SIVR);
	SIVr |= 0x100 | LAPIC_SPURIOUS_IRQ;
	//SIVr |= (1 << 8); /* Set the 8'th bit (Enable APIC) */
	GET_UINT32(lapic_addr, LAPIC_REGISTER_SIVR) = SIVr;
}

void lapic_eoi(void)
{
	ASSERT(lapic_addr != NULL, "lapic_addr not set.");

	GET_UINT32(lapic_addr, LAPIC_REGISTER_EOI) = 0;
}

void ioapic_write(uint32_t offset, uint32_t data)
{
	if(ioapic_addr == NULL) {
		ERROR("ioapic_addr == NULL. ");
		return;
	}

	ioapic_addr[IOAPIC_SELECT_OFFSET] = offset;

	/* Easier this way, to keep the code clean from casting due to 16 byte
	 * offset indexing problems. */
	GET_UINT32(ioapic_addr, IOAPIC_WINDOW_OFFSET) = data;
}

uint32_t ioapic_read(uint32_t offset)
{
	if(ioapic_addr == NULL) {
		ERROR("ioapic_addr == NULL. ");
		return 0x00;
	}

	ioapic_addr[IOAPIC_SELECT_OFFSET] = offset;

	return GET_UINT32(ioapic_addr, IOAPIC_WINDOW_OFFSET);
}

void ioapic_reset_irq(uint8_t irq)
{
	/* Calculate the register offsets, given the IRQ.
	 * Remember that even though offset is only 1, the register is still
	 * 32 bits. */
	const uint32_t low_index = 0x10 + irq * 2;
	const uint32_t high_index = low_index + 1;

	uint32_t high = ioapic_read(high_index);
	high &= ~(0x00FFFFFF); /* Set destination ID to 0x00 */

	uint32_t low = ioapic_read(low_index);
	low &= ~(0xFF); /* Reset last 16 bits */

	low |= (1 << 16); /* Mask interrupt (ignore signal) */

	/* Write */
	ioapic_write(high_index, high);
	ioapic_write(low_index, low);
}

void ioapic_set_irq(uint8_t irq, uint64_t lapic_id, uint8_t vector)
{
	if(vector < APIC_VECTOR_OFFSET) {
		LOG("Vector very low. Are you sure to install irq?");
	}
	/* Calculate the register offsets, given the IRQ.
	 * Remember that even though offset is only 1, the register is still
	 * 32 bits. */
	const uint32_t low_index = 0x10 + irq * 2;
	const uint32_t high_index = low_index + 1;

	/* IOAPIC I/O Redirection Table */
	/* Read top bits (32-63) */
	uint32_t high = ioapic_read(high_index);
	/* Keep the lower 3 bytes (reserved) */
	high &= ~0xFF000000;
	/* Set the destination APIC */
	high |= (lapic_id) << 24;

	/* Read bottom bits (0-31) */
	uint8_t low = ioapic_read(low_index);

	/* Unmask the IRQ bit */
	low &= ~(1<<16);

	/* Set to Physical delivery mode */
	low &= ~(1<<11);

	/* Set to Fixed delivery mode */
	low &= ~(0x700);

	/* Set bits 0-7 to Vector number */
	low &= ~(0xff);		/* Zero it out first */
	low |= vector;		/* set it */


	/* Write back to registers */
	ioapic_write(high_index, high);
	ioapic_write(low_index, low);
}

uint8_t ioapic_is_irq_free(uint8_t irq)
{
	uint32_t entry = ioapic_read(0x10 + irq * 2);

	if(entry & (1<<16)) {
		LOGF("irq = 0x%x is free\n", irq);
		return TRUE;
	}

	return FALSE;
}

uint8_t ioapic_get_free_irq(void)
{
	/* Retrieve Maximum redirection entry number */
	uint8_t num_entries = ioapic_get_num_entries();
	uint8_t i;

	for(i = 0; i < num_entries; i++) {
		if(ioapic_is_irq_free(i)) {
			LOGF("IOAPIC %d is free\n", i);
			return i;
		} else {
			LOGF("IOAPIC %d is used\n", i);
		}
	}

	return 0xFF;
}

uint8_t ioapic_get_num_entries(void)
{
	/* Retrieve Maximum redirection entry number */
	uint8_t num_entries = (ioapic_read(IOAPIC_REGISTER_VERSION) >> 16) + 1;
	return num_entries;
}

void ioapic_init(void)
{
	ASSERT(ioapic_addr != NULL, "ioapic_addr == NULL");

	uint16_t i = 0;

	/* How many IRQs? */
	for(i = 0; i < ioapic_get_num_entries(); i++) {
		ioapic_reset_irq(i);
	}

#if 0
	/* XXX: Maps 215 IRQs to vectors 40-254 on LAPIC */
	for(i = 0; i < 215; i++) {
		/* XXX: Installing to LAPIC ID = 0x00 */
		ioapic_set_irq(i, 0x00, i + 40);
	}
#endif
}


void apic_init(void)
{
	/* Get the MADT */
	if((madt = acpica_get_table(ACPI_SIG_MADT)) == NULL) {
		ERROR("Could not retrieve the MADT.");
		return;
	}

	/* Retrieve the LAPIC Address */
	lapic_addr = (uint64_t*)(uint64_t)madt->Address;
	LOGF("LAPIC_ADDR: 0x%x\n", lapic_addr);

	/* Loop over each entry */
	/* We do this loop twice, as NMI and ISO are handled after LAPIC and
	 * IOAPIC has been setup */
	/* Cast to lapic_entry, because start is always the same, and we can
	 * retrieve the entry length */
	ACPI_SUBTABLE_HEADER* madt_entry = NULL;
	for(madt_entry = (ACPI_SUBTABLE_HEADER*)((uintptr_t)madt + sizeof(ACPI_TABLE_MADT));
	    madt_entry < (ACPI_SUBTABLE_HEADER*)((uintptr_t)madt + madt->Header.Length);
	    madt_entry = (ACPI_SUBTABLE_HEADER*)((uintptr_t)madt_entry + madt_entry->Length)) {
		switch(madt_entry->Type) {
		case ACPI_MADT_TYPE_LOCAL_APIC:
			LOGF("lapic apid_id: 0x%x\n", ((ACPI_MADT_LOCAL_APIC*)
			     madt_entry)->Id);
			break;

		case ACPI_MADT_TYPE_IO_APIC:
			ioapic_addr = (uint32_t*)(uint64_t)((ACPI_MADT_IO_APIC*)
						madt_entry)->Address;

			LOGF("ioapic_addr = 0x%x\n", ioapic_addr);
			break;
		}
	}

	ASSERT(ioapic_addr != NULL, "ioapic_addr == NULL");

	/* Disable PIC. I think must be initialized before */
	pic_init();
	pic_disable();

	/* LAPIC */
	lapic_init();
	/* Install spurious interrupt handler (which should not raise EOI) */
	interrupt_install(LAPIC_SPURIOUS_IRQ, spurious_interrupt_handler);

	/* IO APIC */
	ioapic_init();


	/* Loop over each entry */
	/* We do this loop twice, as NMI and ISO are handled after LAPIC and
	 * IOAPIC has been setup */
	/* Cast to lapic_entry, because start is always the same, and we can
	 * retrieve the entry length */
	for(madt_entry = (ACPI_SUBTABLE_HEADER*)((uintptr_t)madt + sizeof(ACPI_TABLE_MADT));
	    madt_entry < (ACPI_SUBTABLE_HEADER*)((uintptr_t)madt + madt->Header.Length);
	    madt_entry = (ACPI_SUBTABLE_HEADER*)((uintptr_t)madt_entry + madt_entry->Length)) {
		switch(madt_entry->Type) {
		case ACPI_MADT_TYPE_INTERRUPT_OVERRIDE: {
			ACPI_MADT_INTERRUPT_OVERRIDE* madt_iso =
				(struct madt_entry*)madt_entry;
			kprintf("iso found: irq source : 0x%x\ttarget vector: 0x%x\n",
				madt_iso->SourceIrq,
				madt_iso->GlobalIrq);

			/* TODO: What to do here? */
			break;
		}
		case ACPI_MADT_TYPE_NMI_SOURCE:
			LOG("madt nmi found");
			break;
		}
	}
}

uint8_t ioapic_get_iso(uint8_t irq)
{
	ASSERT(madt != NULL, "MADT not setup!");

	ACPI_SUBTABLE_HEADER* madt_entry = NULL;
	for(madt_entry = (ACPI_SUBTABLE_HEADER*)((uintptr_t)madt + sizeof(ACPI_TABLE_MADT));
	    madt_entry < (ACPI_SUBTABLE_HEADER*)((uintptr_t)madt + madt->Header.Length);
	    madt_entry = (ACPI_SUBTABLE_HEADER*)((uintptr_t)madt_entry + madt_entry->Length)) {
		switch(madt_entry->Type) {
		case ACPI_MADT_TYPE_INTERRUPT_OVERRIDE: {
			ACPI_MADT_INTERRUPT_OVERRIDE* madt_iso =
				(ACPI_MADT_INTERRUPT_OVERRIDE*)madt_entry;

			if(madt_iso->SourceIrq == irq) {
				LOGF("irq: 0x%x -> vector 0x%x\n", irq,
				     madt_iso->GlobalIrq);
				return madt_iso->GlobalIrq;
			}
		}
		}


	}

	return irq;
}



