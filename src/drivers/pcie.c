#include "pcie.h"

#include <drivers/pci.h>
#include <acpi/acpica.h>
#include <lib/libc.h>
#include <errno.h>

ACPI_TABLE_MCFG* mcfg = NULL;


ACPI_MCFG_ALLOCATION* pcie_get_allocation(const pcie_dev_t pcie_dev) 
{
    // Get the first allocation table 
    ACPI_MCFG_ALLOCATION* allocations = (ACPI_MCFG_ALLOCATION*)(mcfg + 1);

    // Get the number of allocation tables
    size_t n_allocations = (mcfg->Header.Length - sizeof(ACPI_TABLE_MCFG))
                            / sizeof(ACPI_MCFG_ALLOCATION);
    size_t i;
    for(i = 0; i < n_allocations; i++) {
        // Find the fitting allocation
        if(allocations[i].PciSegment == pcie_dev.pci_segment) {
            if(allocations[i].StartBusNumber <= pcie_dev.bus
            && allocations[i].EndBusNumber >= pcie_dev.bus) {
                // Return the current allocation
                return (allocations + 1);
            }
        }
    }

    LOG("Allocation not found!");
    return NULL;

} 


uint64_t pcie_read(
          const pcie_dev_t pcie_dev,
    	  const uint8_t reg,
		  const uint8_t len)
{
    ACPI_MCFG_ALLOCATION* allocation = pcie_get_allocation(pcie_dev);
    if(allocation == NULL) {
        return 0x00;
    }


    uint64_t* addr = (uint64_t*)(allocation->Address 
                + ((pcie_dev.bus - allocation->StartBusNumber) << 20
                | pcie_dev.dev << 15
                | pcie_dev.func << 12)
                + reg);

    switch(len) {
        case sizeof(uint8_t):
            return *addr & 0xFF;
        case sizeof(uint16_t):
            return *addr & 0xFFFF;
        case sizeof(uint32_t):
            return *addr & 0xFFFFFFFF;
        default:
            return *addr;
    }

}

void pcie_write(
           const pcie_dev_t pcie_dev,
   	       const uint8_t reg,
	       const uint64_t data,
	       const uint8_t len)
{
    ACPI_MCFG_ALLOCATION* allocation = pcie_get_allocation(pcie_dev);
    if(allocation == NULL) {
        return;
    }

    uint64_t* addr = (uint64_t*)(allocation->Address 
                + ((pcie_dev.bus - allocation->StartBusNumber) << 20
                | pcie_dev.dev << 15
                | pcie_dev.func << 12));

    switch(len) {
        case sizeof(uint8_t):
           *(uint8_t*)(addr) = (uint8_t)data;
        case sizeof(uint16_t):
            *(uint16_t*)(addr) = (uint16_t)data;
        case sizeof(uint32_t):
            *(uint32_t*)(addr) = (uint32_t)data;
        default:
            *addr = data;
    }
}

void pcie_find(pcie_func_t callback, uint8_t class, uint8_t subclass)
{
    ASSERT(mcfg != NULL, "PCIe not initialized, or MCFG table not found!");

    ACPI_MCFG_ALLOCATION* allocations = (ACPI_MCFG_ALLOCATION*)(mcfg + 1);

    size_t n_allocations = (mcfg->Header.Length - sizeof(ACPI_TABLE_MCFG))
                            / sizeof(ACPI_MCFG_ALLOCATION); 

    pcie_dev_t pcie_dev;
    
    uint32_t c = 0x00;
    uint32_t sc = 0x00;
    size_t i; 
    for(i = 0; i < n_allocations; i++) {
        pcie_dev.pci_segment = allocations[i].PciSegment;

        for(pcie_dev.bus = allocations[i].StartBusNumber;
            pcie_dev.bus < allocations[i].EndBusNumber;
            pcie_dev.bus++) {
               for(pcie_dev.dev = 0; pcie_dev.dev < 32; pcie_dev.dev++) {
                    for(pcie_dev.func = 0; pcie_dev.func < 8; pcie_dev.func++) {
                        if(pcie_read(pcie_dev, PCI_REGISTER_VENDOR_ID, sizeof(uint16_t))
                            == 0xFFFF) {
                                continue;
                        }

                        c = pcie_read(pcie_dev, PCI_REGISTER_CLASS, 1);
                        sc = pcie_read(pcie_dev, PCI_REGISTER_SUBCLASS, 1);

                        // Debug
                        if(c != 0x00 || sc != 0x00) {
                            kprintf("c: 0x%x sc: 0x%x\n", c, sc);
                        }

                        if(c == class && sc == subclass) {
                            callback(pcie_dev);
                            return;
                        }
                    }
                }
 
            }

    }

    LOG("PCIe device not found!");
}

uint8_t pcie_init(void)
{
    LOG("PCIe INIT");

    // Retrieve the ACPI MCFG table
    if((mcfg = acpica_get_table(ACPI_SIG_MCFG)) == NULL) {
        ERROR("Could not find ACPI MCFG Table for PCIe!");
        return ENOENT;
    }

    // Everything is OK
    return EOK;
}
