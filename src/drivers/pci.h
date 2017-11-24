#pragma once
#include <stdint.h>


#define PCI_ADDRESS_PORT		(0xCF8)
#define PCI_DATA_PORT			(0xCFC)

/* PCI Device Structure field offsets */
#define PCI_REGISTER_VENDOR_ID          0x00
#define PCI_REGISTER_DEVICE_ID          0x02
#define PCI_REGISTER_COMMAND            0x04
#define PCI_REGISTER_STATUS             0x06
#define PCI_REGISTER_SUBCLASS           0x0A
#define PCI_REGISTER_CLASS              0x0B
#define PCI_REGISTER_HEADER_TYPE	0x0E
#define PCI_REGISTER_BAR0               0x10
#define PCI_REGISTER_BAR1               0x14
#define PCI_REGISTER_BAR2               0x18
#define PCI_REGISTER_BAR3               0x1C
#define PCI_REGISTER_BAR4               0x20
#define PCI_REGISTER_BAR5               0x24
/* ... more here ... */
#define PCI_REGISTER_CAPABILITIES	0x34
#define PCI_REGISTER_INTERRUPT_LINE     0x3C
#define PCI_REGISTER_INTERRUPT_PIN      0x3D

/* CLASS CODES */
#define PCI_CLASS_ATA			(0x01)

/* SUBCLASS CODES */
#define PCI_SUBCLASS_IDE		(0x01)
#define PCI_SUBCLASS_RAID		(0x04)
#define PCI_SUBCLASS_ATA		(0x05)
#define PCI_SUBCLASS_SATA		(0x06)
#define PCI_SUBCLASS_SOLID_STATE	(0x08)

/* Protocols */
#define PCI_PROTOCOL_AHCI		(0x01)

/* Capability IDs */
#define PCI_CAP_ID_MSI_X		(0x11)
#define PCI_CAP_ID_MSI			(0x05)

/* Structures */
typedef struct msi_x_table_entry {
	uint32_t addr_low;
	uint32_t addr_high;
	uint32_t data;
	uint32_t vector_control;
} __attribute__((packed)) msi_x_table_entry_t;

/* Callback type */
typedef void (*pci_func_t)(uint8_t bus, uint8_t dev, uint8_t func);

/* Read from PCI */
uint32_t pci_read(const uint8_t bus,
		  const uint8_t dev,
		  const uint8_t func,
		  const uint8_t reg,
		  const uint8_t len);

/* Write to PCI */
void pci_write(const uint8_t bus,
	       const uint8_t dev,
	       const uint8_t func,
	       const uint8_t reg,
	       const uint32_t data,
	       const uint8_t len);

void pci_find(pci_func_t callback, uint8_t class, uint8_t subclass);

void pci_init(void);
void pci_bar_write(uint32_t bar, uint32_t val);
uint32_t pci_bar_read(uint32_t bar);

void pci_init_devices(void);

/* Iterates over the PCI capabilities list, and returns the offset to the
 * desired capability.
 * 0 if not found. */
uint8_t pci_find_capability(uint8_t bus, uint8_t dev, uint8_t func, uint8_t id);

/* Installs an interrupt for a device */
uint8_t pci_install_interrupt(uint8_t bus,
			      uint8_t dev,
			      uint8_t func,
			    interrupt_handler_t handler);



