#include "ahci.h"
#include <stdint.h>
#include <libc.h>
#include <drivers/pci.h>
#include <kernel/time.h>
#include <errno.h>
#include <kernel/time.h>


ahci_register_set_t* ahci = NULL;


void ahci_enable_controller(ahci_register_set_t* controller)
{
	controller->generic.ghc |= (1 << 31); // AHCI Enable
}

bool ahci_gain_ownership(ahci_register_set_t* controller)
{
	/* Only 1.2.0 and above supported */
	if(controller->generic.vs >= 0x00010200) {

		/* Only supported if CAP2.BOH is supported */
		if(controller->generic.cap2 & (1 << 0)) {
			/* Set the OOS bit */
			controller->generic.bohc |= (1 << 1);

			sleep(25);

			/* If after 25 msec the BB is set, we need to wait 2
			 * seconds. */
			if((controller->generic.bohc &
			    ((1 << 4)	 // Bios Busy
			     | (1 << 1)  // OOS
			     | (1 << 0))) // BOS
			   != (1 << 1)) { // OOS
				// Failed to gain ownership
				return FALSE;
			}
		}
	}
	return TRUE;
}

uint32_t ahci_get_port_register_sets_allocated(ahci_register_set_t* controller)
{
	return (controller->generic.cap & 0x1F) + 1;
}

uint32_t ahci_get_command_slots_count(ahci_register_set_t * controller)
{
	uint32_t caps = controller->generic.cap;

	return ((caps >> 8) & 0x1F) + 1;
}

uint8_t ahci_port_reset(ahci_register_set_t *controller, uint32_t drive)
{
	ASSERT(drive >= ahci_get_port_register_sets_allocated(controller), "Invalid AHCI port to reset!");

	// COMRESET
	controller->port_register[drive].sctl |= (1 << 0);

	// Sleep some time, to ensure multiple COMRESETS are sent over the interface
	sleep(10);

	// Reset the COMRESET bit
	controller->port_register[drive].sctl &= ~(1 << 0);

	// Wait some more for initialization to complete
	// XXX
	DEBUG("Waiting for port reset to complete.");

	// Loop until the controller is up
	uint32_t timeout = 1000;
	while((controller->port_register[drive].ssts & 0x7) != 0x3) {
		if(timeout-- == 0) {
			return FALSE;
		}
		sleep(1);
	}

	return TRUE;
}

void ahci_port_init(ahci_register_set_t* controller, uint32_t drive)
{
	// Reset the drive, and check if attached
	if(ahci_port_reset(controller, drive) == FALSE) {
			LOGF("Could not reset AHCI port: 0x%x. Not attached", drive);
			return;
	}

	if(controller->port_register[drive].sig != AHCI_PORT_SIG_SATA) {
		LOGF("Drive is not a SATA device.");
		return;
	}


}

void ahci_ports_init(ahci_register_set_t* controller) 
{
	uint32_t drive = 0x00;

	uint32_t ports_implemented = controller->generic.pi;

	uint32_t i;
	for(i = 0x00; i < 32; i++) {
		if(ports_implemented & (1 << i)) {
			LOGF("Port 0x%x implemented!\n", i);
			
			ahci_port_init(controller, i);	
		}
	}

}

uint8_t ahci_interrupt_handler(pt_regs_t* regs)
{
	LOG("AHCI Interrupt!");
	lapic_eoi();
}

void ahci_found(uint8_t bus, uint8_t dev, uint8_t func)
{
	LOG("SATA found!");

	// Get the AHCI MMIO
	ahci = pci_read(bus, dev, func, PCI_REGISTER_BAR5, 4);
	ASSERT(ahci != NULL, "AHCI == NULL");

	/* Gain ownership */
	if(ahci_gain_ownership(ahci) == FALSE) {
		ERROR("Failed to gain ownership of SATA");
		HALT;
	}

	/* Enable the controller */
	ahci_enable_controller(ahci);

	/* Initialize ports */
	ahci_ports_init(ahci);


	if(pci_install_interrupt(bus, dev, func, ahci_interrupt_handler) != EOK) {
		ERROR("Interrupt not installed for AHCI");
		return;
	} else {
		LOG("Interrupt installed for AHCI");
	}

	
}



void ahci_init(void)
{
	LOG("AHCI Init");
	pci_find(ahci_found, PCI_CLASS_ATA, PCI_SUBCLASS_SATA);
}
