#pragma once

#define PCI_ADDRESS_PORT		(0xCF8)
#define PCI_DATA_PORT			(0xCFC)

/* PCI Device Structure field offsets */
#define PCI_REGISTER_VENDOR_ID          0x00
#define PCI_REGISTER_DEVICE_ID          0x02
#define PCI_REGISTER_COMMAND            0x04
#define PCI_REGISTER_STATUS             0x06
#define PCI_REGISTER_SUBCLASS           0x0A
#define PCI_REGISTER_CLASS              0x0B
#define PIC_REGISTER_HEADER_TYPE	0x0E
#define PCI_REGISTER_BAR0               0x10
#define PCI_REGISTER_BAR1               0x14
#define PCI_REGISTER_BAR2               0x18
#define PCI_REGISTER_BAR3               0x1C
#define PCI_REGISTER_BAR4               0x20
#define PCI_REGISTER_BAR5               0x24
/* ... more here ... */
#define PCI_REGISTER_INTERRUPT_LINE     0x3C
#define PCI_REGISTER_INTERRUPT_PIN      0x3D




uint32_t pci_read(const uint8_t bus,
		  const uint8_t dev,
		  const uint8_t func,
		  const uint8_t reg,
		  const uint8_t len);

