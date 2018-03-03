#include "ahci.h"
#include <stdint.h>
#include <libc.h>
#include <drivers/pci.h>
#include <mem/mem.h>
#include <kernel/time.h>
#include <errno.h>
#include <kernel/time.h>


ahci_register_set_t* ahci = NULL;
ahci_cmd_hdr_t cmd_hdrs[32] __attribute__((aligned(4096)));
ahci_cmd_tbl_t cmd_tbl_t[32] __attribute__((alligned(4096)));
ahci_fis_register_h2d_t* fis __attribute__((alligned(4096)));


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

void ahci_disable_command_engine(ahci_register_set_t* controller, uint32_t port)
{
	// Clear PxCMD.ST
	controller->port_register[port].cmd &= ~(1 << 0);

	// Wait until both PxCMD.FR and PxCMD.CR are clear
	while(controller->port_register[port].cmd & (1 << 15))
		;

	controller->port_register[port].cmd &= ~(1 << 4);
	while(controller->port_register[port].cmd & (1 << 14))
		;
}


void ahci_enable_command_engine(ahci_register_set_t* controller, uint32_t port)
{
	// wait until CR (bit 15) is cleared
	while(controller->port_register[port].cmd & (1 << 15))
		;

	// Enable the command engine
	controller->port_register[port].cmd |= (1 << 4); // PxCMD.FRE
	controller->port_register[port].cmd |= (1 << 0); // PxCMD.ST
}



void ahci_read(ahci_register_set_t* controller,
				uint32_t port,
				uint64_t lba,
				uint64_t count,
				uint8_t buffer)
{
	ahci_cmd_hdr_t* hdr = (ahci_cmd_hdr_t*)controller->port_register[port].clbu;
	ahci_cmd_tbl_t tbl = {0};

	hdr->cfl = sizeof(ahci_h2d_register_fis_t) / sizeof(uint32_t); // command FIS size in DWORDs
	hdr->w = 0; // read
	hdr->prdtl = 1; // 1 PRDT
	hdr->ctbau = (uint64_t)&tbl;



	HBA_CMD_TBL *cmdtbl = (HBA_CMD_TBL*)(cmdheader->ctba);
	memset(cmdtbl, 0, sizeof(HBA_CMD_TBL) +
 		(cmdheader->prdtl-1)*sizeof(HBA_PRDT_ENTRY));

	// 8K bytes (16 sectors) per PRDT
	for (int i=0; i<cmdheader->prdtl-1; i++)
	{
		cmdtbl->prdt_entry[i].dba = (uint32_t) buf;
		cmdtbl->prdt_entry[i].dbc = 8*1024-1;	// 8K bytes (this value should always be set to 1 less than the actual value)
		cmdtbl->prdt_entry[i].i = 1;
		buf += 4*1024;	// 4K words
		count -= 16;	// 16 sectors
	}
	// Last entry
	cmdtbl->prdt_entry[i].dba = (uint32_t) buf;
	cmdtbl->prdt_entry[i].dbc = (count<<9)-1;	// 512 bytes per sector
	cmdtbl->prdt_entry[i].i = 1;

	// Setup command
	FIS_REG_H2D *cmdfis = (FIS_REG_H2D*)(&cmdtbl->cfis);

	cmdfis->fis_type = FIS_TYPE_REG_H2D;
	cmdfis->c = 1;	// Command
	cmdfis->command = ATA_CMD_READ_DMA_EX;

	cmdfis->lba0 = (uint8_t)startl;
	cmdfis->lba1 = (uint8_t)(startl>>8);
	cmdfis->lba2 = (uint8_t)(startl>>16);
	cmdfis->device = 1<<6;	// LBA mode

	cmdfis->lba3 = (uint8_t)(startl>>24);
	cmdfis->lba4 = (uint8_t)starth;
	cmdfis->lba5 = (uint8_t)(starth>>8);

	cmdfis->countl = count & 0xFF;
	cmdfis->counth = (count >> 8) & 0xFF;

	// The below loop waits until the port is no longer busy before issuing a new command
	while ((port->tfd & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 1000000)
	{
		spin++;
	}
	if (spin == 1000000)
	{
		trace_ahci("Port is hung\n");
		return FALSE;
	}

	port->ci = 1<<slot;	// Issue command

	// Wait for completion
	while (1)
	{
		// In some longer duration reads, it may be helpful to spin on the DPS bit
		// in the PxIS port field as well (1 << 5)
		if ((port->ci & (1<<slot)) == 0)
			break;
		if (port->is & HBA_PxIS_TFES)	// Task file error
		{
			trace_ahci("Read disk error\n");
			return FALSE;
		}
	}

	// Check again
	if (port->is & HBA_PxIS_TFES)
	{
		trace_ahci("Read disk error\n");
		return FALSE;
	}

	return 1;
}


void ahci_port_init(ahci_register_set_t* controller, uint32_t drive)
{
	// Reset the drive, and check if attached
	if(ahci_port_reset(controller, drive) == FALSE) {
		LOGF("Could not reset AHCI port: 0x%x. Not attached", drive);
		return;
	}

	if(controller->port_register[drive].sig != AHCI_PORT_SIG_SATA) {
		LOG("Drive is not a SATA device.");
		return;
	}

	// Disable the command engine
	ahci_disable_command_engine(controller, drive);

	// Now, set the addresses of Command Header list and Receive FIS
	controller->port_register[drive].clbu = (uint64_t)cmd_hdrs;
	controller->port_register[drive].fbu = (uint64_t)fis;

	// Enable the command engine
	ahci_enable_command_engine(controller, drive);

	// Clear the PxIS register
	controller->port_register[drive].is = 0xFFFFFFFF;
	controller->port_register[drive].ie = 0xFFFFFFFF;

	uint8_t buffer[513] = {0};
	//ahci_read(controller, drive, 0, 1, buffer);

	kprintf("read data: %s\n", buffer);
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
			return;
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

	/* Install Interrupts */
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
