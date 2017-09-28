#include "pci.h"
#include <stdint.h>



uint32_t pci_read(const uint8_t bus,
		  const uint8_t dev,
		  const uint8_t func,
		  const uint8_t reg,
		  const uint8_t len)
{
	uint32_t ret = 0x00;

	const uint8_t val = 0x80000000
				| (bus << 16)
				| ((dev & 0x1F) << 11)
				| ((func & 0x7) << 8)
				| ((reg & 0xFC));

	outd(PCI_ADDRESS_PORT, val);

	/* This ensures we can read on any offset, which otherwise has to be on
	 * DWORD boundary. */
	ret = (ind(PCI_DATA_PORT) >> ((reg & 3) * 8))
		& (0xFFFFFFFF >> ((4 - len) * 8));

	return ret;

}



