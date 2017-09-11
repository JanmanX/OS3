#include "apic.h"
#include <kernel/acpi.h>
#include <libc.h>
#include <cpu/pic.h>

/* Multiple APIC Descriptor Table */
madt_t *madt = NULL;

/* LAPIC address (Can be same for multiple processors ) */
uint64_t* lapic_addr;

void lapic_enable(void)
{
	/* Spuriour Interrupt Vector Register */
	uint32_t SIVr = GET_UINT64(lapic_addr, LAPIC_REGISTER_SIVR);
	SIVr |= (1 << 8); /* Set the 8'th bit (Enable APIC) */

	GET_UINT64(lapic_addr, LAPIC_REGISTER_SIVR) = SIVr;

	kprintf("SIVR: 0x%x\n", SIVr);
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

	/* Disable PIC. I think must be initialized before */
	pic_init();
	pic_disable();

	lapic_enable();


}
