#include "pci.h"
#include <libc.h>
#include <stdint.h>
#include <acpi.h>
#include <lib/errno.h>
#include <kernel/interrupt.h>

// Translate PINs (INTA, INTB, ...) to GSI
uint8_t pin_to_gsi[4] = {0};

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
	uint32_t ret = (ind(PCI_DATA_PORT)	  /* Read from port*/
			>> ((reg & 3) * 8))	  /* Shift, so that
						     register is at bottom
						     bytes */
		& (0xFFFFFFFF >> ((4 - len) * 8)); /* Mask only the number
						      of bytes we want. */

	return ret;
}

void pci_write(const uint8_t bus,
	       const uint8_t dev,
	       const uint8_t func,
	       const uint8_t reg,
	       const uint32_t data,
	       const uint8_t len)
{
	uint32_t payload = (bus << 16) | (dev << 11) | (func << 8) | (reg & 0xFC) | ((uint32_t)0x80000000);

	/* Write to the address */
	outd(PCI_ADDRESS_PORT, payload);

	/* Write the data */
	outd(PCI_DATA_PORT, data);
}


void pci_find(pci_func_t callback, uint8_t class, uint8_t subclass)
{
	uint16_t bus;
	uint8_t device;
	uint8_t func = 0x00;
	uint32_t c = 0x00;
	uint32_t sc = 0x00;

	for (bus = 0; bus < 256; bus++)
	{
		for (device = 0; device < 32; device++)
		{
			for (func = 0; func < 8; func++)
			{
				if ((uint16_t)pci_read(bus,
						       device,
						       func,
						       PCI_REGISTER_VENDOR_ID,
						       2) == 0xFFFF)
				{
					continue;
				}

				c = pci_read(bus, device, func,
					     PCI_REGISTER_CLASS, 1);
				sc = pci_read(bus, device, func,
					      PCI_REGISTER_SUBCLASS, 1);

				if (class == c && subclass == sc)
				{
					callback(bus, device, func);
					return;
				}
			}
		}
	}

	LOG("PCI Device not found");
}

ACPI_STATUS acpi_get_devices_callback(ACPI_HANDLE object,
				      UINT32 nesting_level,
				      void *context,
				      void **return_value)
{
	uint8_t *pins = (uint8_t *)context;

	/* Retrieve device info, so we can determine for PCI bridge */
	ACPI_DEVICE_INFO *dev_info;
	ACPI_STATUS st = AcpiGetObjectInfo(object, &dev_info);
	if (ACPI_FAILURE(st))
	{
		ERROR("AcpiGetObjectInfo error!");
		return AE_ERROR;
	}

	if (dev_info->Flags & ACPI_PCI_ROOT_BRIDGE)
	{
		// Get routing for PCI Bridge
		ACPI_BUFFER buffer = {0};
		buffer.Length = ACPI_ALLOCATE_BUFFER;
		if ((st = AcpiGetIrqRoutingTable(object, &buffer)) != AE_OK)
		{
			ERROR("AcpiGetIrqRoutingTable failed!");
			return AE_ERROR;
		}

		ACPI_PCI_ROUTING_TABLE *table = buffer.Pointer;

		for (; table->Length != 0; table = (ACPI_PCI_ROUTING_TABLE *)ACPI_NEXT_RESOURCE(table))
		{
			if (table->Source[0] != 0x00)
			{
				LOGF("table->Source = %s\ntable->Pin: 0x%x\ntable->SourceIndex: 0x%x\n",
				     table->Source, table->Pin, table->SourceIndex);
			}
			else
			{
				kprintf("pin: 0x%x -> gsi: 0x%x\n", table->Pin, table->SourceIndex);
			}
		}
	}
}

void pci_init(void)
{
	return;

	void *retval;
	uint8_t pins[4] = {0};
	ACPI_STATUS st = AcpiGetDevices(NULL, acpi_get_devices_callback, (void *)pins, &retval);
	if (ACPI_FAILURE(st))
	{
		ERROR("Could not get devices");
		return;
	}
}

void pci_bar_write(uint32_t bar, uint32_t val)
{
	/* If port io */
	if (bar & 0x01)
	{
		uint16_t port = bar & ~((uint32_t)0x01);
		outd(port, val);
		return;
	}

	/* 64 bit address. Not implemented*/
	if (bar & 0x04)
	{
		ERROR("64bit BAR WRITE");
	}

	/* BAR is in 4GiB. Mask away 4 lower bits */
	uint32_t *addr = (uint32_t)bar & ~(0xF);
	*addr = val;
}

uint32_t pci_bar_read(uint32_t bar)
{
	/* If port io */
	if (bar & 0x01)
	{
		uint16_t port = bar & ~((uint32_t)0x01);
		return ind(port);
	}

	/* 64 bit address. Not implemented*/
	if (bar & 0x04)
	{
		ERROR("64bit BAR WRITE");
	}

	/* BAR is in 4GiB. Mask away 4 lower bits */
	return *((uint32_t *)(bar & ~(0xF)));
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

	for (bus = 0; bus < 256; bus++)
	{
		for (device = 0; device < 32; device++)
		{
			for (func = 0; func < 8; func++)
			{
				if ((vendor_id = (uint16_t)pci_read(bus,
								    device,
								    func,
								    PCI_REGISTER_VENDOR_ID,
								    2)) == 0xFFFF)
				{
					continue;
				}

				class = (uint8_t)pci_read(bus, device, func,
							  PCI_REGISTER_CLASS, 1);
				subclass = (uint8_t)pci_read(bus, device, func,
							     PCI_REGISTER_SUBCLASS,
							     1);
				LOGF("PCI Bus: 0x%x\tdev: 0x%x\tclass: 0x%x\tsubclass: 0x%x\n", bus, device, class, subclass);
			}
		}
	}
}

uint8_t pci_find_capability(const uint8_t bus,
			    const uint8_t dev,
			    const uint8_t func,
			    const uint8_t id)
{
	/* Check that capabilities exist */
	if (pci_read(bus, dev, func, PCI_REGISTER_STATUS, 0x02) & (1 << 4) == 0x00)
	{
		LOG("Capabilities not supported!");
		return 0x00;
	}

	/* Iterate
	 * get first item */
	uint8_t offset = (uint8_t)pci_read(bus,
					   dev,
					   func,
					   PCI_REGISTER_CAPABILITIES,
					   sizeof(uint8_t));
	while (offset != 0x00)
	{
		// Read type and next pointer in one go
		uint16_t type_next = (uint16_t)pci_read(bus,
							dev,
							func,
							offset,
							sizeof(uint16_t));

		uint8_t type = type_next & 0xFF;

		/* Check the ID field */
		if (type == id)
		{
			return offset;
		}

		/* Get the pointer to next field
		 * Bottom 2 bits are reserved, and to be masked */
		offset = (type_next >> 0x8) & 0xFC;
	}

	/* Not found */
	return 0x00;
}

uint8_t pci_install_interrupt(uint8_t bus,
			      uint8_t dev,
			      uint8_t func,
			      interrupt_handler_t handler)
{
	uint8_t capability = 0x00;

	if ((capability = pci_find_capability(bus, dev, func, PCI_CAP_ID_MSI)) == 0x00)
	{
		kprintf("MSI not found");
		return ENODEV;
	}

	// Get the Message Control
	uint16_t message_control = pci_read(bus,
					    dev,
					    func,
					    capability + 0x02,
					    sizeof(uint16_t));

	// Setup Message Address
	uint32_t message_address = (0x0FEE << 20)
		| (0x00 << 12) // APIC ID
		| (0x00 << 3)				 // Redirection Hint
		| (0x00 << 2);				 // Destination Mode
	pci_write(bus, dev, func, capability + 0x04, message_address, sizeof(uint32_t));

	// If 64 bit addressing
	if (message_control & (0x01 << 7))
	{
		LOG("MSI 64 bit capable");
		pci_write(bus, dev, func, capability + 0x8, (uint32_t)0x00, sizeof(uint32_t));
	}

	// Setup interrupt
	uint8_t vector = interrupt_get_free_vector();
	if (vector == 0x00)
	{
		ERROR("MSI no available Interrupt Vectors left");
		return EFAULT;
	}
	interrupt_install(vector, handler);

	// Setup Message Data
	uint16_t message_data = 0x00;
	message_data = vector
		| (0x00 << 8) // Fixed delivery
		| (0x00 << 14)		// Level for trigger mode
		| (0x00 << 15);		// Edge trigger mode

	if (message_control & (1 << 7))
	{
		pci_write(bus, dev, func, capability + 0xC, message_control, sizeof(uint16_t));
	}
	else
	{
		pci_write(bus, dev, func, capability + 0x8, message_control, sizeof(uint16_t));
	}

	// Enable in MSI Capability message control
	message_control |= (1 << 0);
	pci_write(bus, dev, func, capability + 0x02, message_control, sizeof(uint16_t));

	return EOK;
}
