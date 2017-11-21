#include "sata.h"
#include <stdint.h>
#include <libc.h>
#include <drivers/pci.h>


ahci_register_set_t* ahci = NULL;


bool sata_gain_ownership(uint64_t* hba)
{

	return TRUE;
}


void sata_found(uint8_t bus, uint8_t dev, uint8_t func)
{
	LOG("SATA found!");

	ahci = pci_read(bus, dev, func, PCI_REGISTER_BAR5, 4);
	LOGF("ABAR: 0x%x\n", bar5);


}



void sata_init(void)
{
	pci_find(sata_found, PCI_CLASS_ATA, PCI_SUBCLASS_SATA);
}
