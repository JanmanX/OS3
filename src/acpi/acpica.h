#pragma once

#include <acpi.h>

/* RSDP version 1.0 and 2.0 */
#define RSDP_REV_1_SIZE		(0x14)
#define RSDP_REV_2_SIZE		(0x24)
typedef struct rsdp {
	uint8_t signature[8];
	uint8_t checksum;
	uint8_t oem[6];

#define RSDP_REV_1		(0x00)
#define RSDP_REV_2		(0x02)
	uint8_t revision;
	uint32_t rsdt_ptr;

	/* version 2+ continues here */
	uint32_t length;
	uint64_t xsdt_ptr;
	uint8_t extra_checksum;
	uint8_t reserved[3];
} __attribute__((packed)) rsdp_t;



void* acpica_get_rsdp(void);
void acpica_early_init(void);
void acpica_init(void);
ACPI_TABLE_HEADER* acpica_get_table(uint8_t* signature);


