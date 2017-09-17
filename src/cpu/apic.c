#include "apic.h"
#include <kernel/acpi.h>
#include <libc.h>
#include <cpu/pic.h>
#include <stdint.h>
#include <kernel/interrupt.h>

/* Multiple APIC Descriptor Table */
madt_t *madt = NULL;

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
	SIVr = 0x100 | LAPIC_SPURIOUS_IRQ;
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

void ioapic_set_irq(uint8_t irq, uint64_t apic_id, uint8_t vector)
{
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
	high |= (apic_id) << 24;

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


void ioapic_init(void)
{
	ioapic_set_irq(0x01, 0xFF, 0x40);
}

void apic_init(void)
{
	/* Get the MADT */
	if((madt = acpi_get_table(ACPI_SIGNATURE_MADT)) == NULL) {
		ERROR("Could not retrieve the MADT.");
		return;
	}

	/* Retrieve the LAPIC Address */
	lapic_addr = (uint64_t*)(uint64_t)madt->lapic_addr;
	LOGF("LAPIC_ADDR: 0x%x\n", lapic_addr);

	/* Loop over each entry */
	/* Cast to lapic_entry, because start is always the same, and we can
	 * retrieve the entry length */
	struct madt_entry_lapic* madt_entry = NULL;
	for(madt_entry = (madt_entry_lapic_t*)((uintptr_t)madt + sizeof(madt_t));
	    madt_entry < (madt_entry_lapic_t*)((uintptr_t)madt + madt->header.length);
	    madt_entry = (madt_entry_lapic_t*)((uintptr_t)madt_entry + madt_entry->length)) {
		switch(madt_entry->type) {
		case MADT_ENTRY_TYPE_LAPIC:
			LOGF("lapic apid_id: 0x%x\n", madt_entry->apid_id);
			break;

		case MADT_ENTRY_TYPE_IOAPIC:
			ioapic_addr = (uint32_t*)(uint64_t)(
							    (madt_entry_ioapic_t*)madt_entry)->io_apic_addr;

			LOGF("ioapic_addr = 0x%x\n", ioapic_addr);
			break;

		case MADT_ENTRY_TYPE_ISO: {
			struct madt_entry_iso* madt_iso =
				(struct madt_entry*)madt_entry;
			kprintf("iso found: irq source : 0x%x\tbus source :0x%x\n",
			       	madt_iso->irq_source,
				madt_iso->bus_source);

			break;
		}
		case MADT_ENTRY_TYPE_NMI:
			LOG("madt nmi found");

			break;
		}

	}

	if(ioapic_addr == NULL) {
		ERROR("NO IOAPIC FOUND!");
		return;
	}

	/* Disable PIC. I think must be initialized before */
	pic_init();
	pic_disable();

	/* LAPIC */
	lapic_init();
	/* Install spurious interrupt handler (which should not raise EOI) */
	interrupt_install(LAPIC_SPURIOUS_IRQ, spurious_interrupt_handler);

	/* IO APIC */
	ioapic_init();

}
