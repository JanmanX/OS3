#pragma once
#include <stdint.h>

/* RSDP version 1.0 and 2.0 */

#define RSDP_REV_1_SIZE		(0x14)
#define RSDP_REV_2_SIZE		(0x24)
typedef struct rsdp {
	uint8_t signature[8];
	uint8_t checksum;
	uint8_t oem[6];
	uint8_t revision;
	uint32_t rsdt_ptr;

	/* version 2+ continues here */
	uint32_t length;
	uint64_t xsdt_ptr;
	uint8_t extra_checksum;
	uint8_t reserved[3];
} __attribute__((packed)) rsdp_t;

/* System Description Table */
#define ACPI_SDT_SIZE		(0x24)
typedef struct acpi_sdt {
	uint8_t signature[4];
	uint32_t length;

#define ACPI_SDT_REV_1		(0x00) /* ACPI 1.0 */
#define ACPI_SDT_REV_2		(0x02) /* ACPI 2.0 */
	uint8_t revision;
	uint8_t checksum;
	uint8_t oem_id[6];
	uint8_t oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
} __attribute__((packed)) acpi_sdt_t;

/* RSDT and XSDT */
typedef struct xsdt {
	acpi_sdt_t header;
	uint64_t tables[0];
} __attribute__((packed)) xsdt_t;

typedef struct rsdt {
	acpi_sdt_t header;
	uint32_t tables[0];
} __attribute__((packed)) rsdt_t;


void acpi_init(void);
