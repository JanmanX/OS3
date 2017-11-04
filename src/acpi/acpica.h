#pragma once

#include <acpi.h>

/* Macros for weird structures in the ACPICA */
#if 0
typedef struct acpi_generic_address
{
	/* Lower to upper */
    UINT8                   SpaceId;                /* Address space where struct or register exists */
    UINT8                   BitWidth;               /* Size in bits of given register */
    UINT8                   BitOffset;              /* Bit offset within the register */
    UINT8                   AccessWidth;            /* Minimum Access size (ACPI 3.0) */
    UINT64                  Address;                /* 64-bit address of struct or register */
} ACPI_GENERIC_ADDRESS;
#endif


#define ACPICA_ADDRESS_UINT32(a) (   (uint64_t)a.SpaceId\
				  || (uint64_t)a.BitWidth	<< 8\
				  || (uint64_t)a.BitOffset	<< 16\
				  || (uint64_t)a.AccessWidth	<< 24)



void acpica_init(void);
ACPI_TABLE_HEADER* acpica_get_table(uint8_t* signature);


