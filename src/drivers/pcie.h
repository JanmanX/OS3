#pragma once

#include <stdint.h>

/* Structures */
typedef struct pcie_dev {
    uint16_t pci_segment;
    uint8_t bus;
    uint8_t dev;
    uint8_t func;
} pcie_dev_t;

/* Callback type */
typedef void (*pcie_func_t)(pcie_dev_t dev);



/* Read from PCI */
uint64_t pcie_read(
          const pcie_dev_t pcie_dev,
    	  const uint8_t reg,
		  const uint8_t len);

/* Write to PCI */
void pcie_write(
           const pcie_dev_t pcie_dev,
   	       const uint8_t reg,
	       const uint64_t data,
	       const uint8_t len);


struct acpi_mcfg_allocation* pcie_get_allocation(
        const pcie_dev_t pcie_dev);


uint8_t pcie_init(void);

void pcie_find(pcie_func_t callback, uint8_t class, uint8_t subclass);
