#include "pci.h"
#include <libc.h>
#include <stdint.h>

uint32_t pci_read(uint8_t bus,
			 uint8_t dev,
			 uint8_t func,
			 uint8_t reg,
			 uint8_t len)
{
	uint32_t payload = (bus << 16)
		| (dev << 11)
		| (func << 8)
		| (reg & 0xFC)
		| ((uint32_t)0x80000000);

	/* Write to the address */
	outd(PCI_ADDRESS_PORT, payload);

	/* Read the data */
	uint32_t ret = (ind(PCI_DATA_PORT)		/* Read from port*/
			>> ((reg & 3) * 8))		/* Shift, so that
							   register is at bottom
							   bytes */
			& (0xFFFFFFFF >> ((4-len) * 8)); /* Mask only the number
							   of bytes we want. */

	return ret;
}


void pci_find(pci_func_t callback, uint8_t class, uint8_t subclass)
{
	uint16_t bus;
	uint8_t device;
	uint8_t func = 0x00;
	uint32_t c = 0x00;
	uint32_t sc = 0x00;

	for(bus = 0; bus < 256; bus++) {
		for(device = 0; device < 32; device++) {
			for(func = 0; func < 8; func++) {
				if((uint16_t)pci_read(bus,
						      device,
						      func,
						      PCI_REGISTER_VENDOR_ID,
						      2)
				   == 0xFFFF) {
					continue;
				}

				c = pci_read(bus, device, func,
					     PCI_REGISTER_CLASS, 1);
				sc = pci_read(bus, device, func,
					      PCI_REGISTER_SUBCLASS, 1);


				if(class == c && subclass == sc) {
					callback(bus, device, func);
					return;
				}
			}
		}
	}

	LOG("PCI Device not found");
}

void pci_bar_write(uint32_t bar, uint32_t val)
{
	/* If port io */
	if(bar & 0x01) {
		uint16_t port = bar & ~((uint32_t)0x01);
		outd(port, val);
		return;
	}

	/* 64 bit address. Not implemented*/
	if(bar & 0x04) {
		ERROR("64bit BAR WRITE");
	}

	/* BAR is in 4GiB. Mask away 4 lower bits */
	uint32_t* addr = (uint32_t)bar & ~(0xF);
	*addr = val;
}

uint32_t pci_bar_read(uint32_t bar)
{
	/* If port io */
	if(bar & 0x01) {
		uint16_t port = bar & ~((uint32_t)0x01);
		return ind(port);
	}

	/* 64 bit address. Not implemented*/
	if(bar & 0x04) {
		ERROR("64bit BAR WRITE");
	}

	/* BAR is in 4GiB. Mask away 4 lower bits */
	return *((uint32_t*)(bar & ~(0xF)));
}


void pci_list(void)
{
	uint16_t bus;
	uint8_t device;
	uint8_t func;
	uint32_t vendor_id = 0x00;
	uint8_t class = 0x00;
	uint8_t subclass = 0x00;


	LOG("PCI Devices:");

	for(bus = 0; bus < 256; bus++) {
		for(device = 0; device < 32; device++) {
			for(func = 0; func < 8; func++) {
				if((vendor_id = (uint16_t)pci_read(bus,
								   device,
								   func,
								   PCI_REGISTER_VENDOR_ID,
								   2))
				   == 0xFFFF) {
					continue;
				}

				class = (uint8_t)pci_read(bus, device, func,
							  PCI_REGISTER_CLASS, 1);
				subclass = (uint8_t)pci_read(bus, device, func,
							     PCI_REGISTER_SUBCLASS,
							     1);
				LOGF("PCI Bus: 0x%x\tdev: 0x%x\tclass: 0x%x\tsubclass: 0x%x\n"
				     , bus, device, class, subclass);
			}
		}
	}
}
