#pragma once
#include <stdint.h>

/* Table Signatures */
#define ACPI_SIGNATURE_LEN	(0x04)

#define ACPI_SIGNATURE_MADT ((uint8_t*)"APIC")
#define ACPI_SIGNATURE_BGRT ((uint8_t*)"BGRT")
#define ACPI_SIGNATURE_BERT ((uint8_t*)"BERT")
#define ACPI_SIGNATURE_CPEP ((uint8_t*)"CPEP")
#define ACPI_SIGNATURE_DSDT ((uint8_t*)"DSDT")
#define ACPI_SIGNATURE_ECDI ((uint8_t*)"ECDT")
#define ACPI_SIGNATURE_EINJ ((uint8_t*)"EINJ")
#define ACPI_SIGNATURE_ERST ((uint8_t*)"ERST")
#define ACPI_SIGNATURE_FADT ((uint8_t*)"FACP")
#define ACPI_SIGNATURE_FACS ((uint8_t*)"FACS")
#define ACPI_SIGNATURE_HEST ((uint8_t*)"HEST")
#define ACPI_SIGNATURE_MSCT ((uint8_t*)"MSCT")
#define ACPI_SIGNATURE_MPST ((uint8_t*)"MPST")
#define ACPI_SIGNATURE_OEMx ((uint8_t*)"OEMx")
#define ACPI_SIGNATURE_PMTT ((uint8_t*)"PMTT")
#define ACPI_SIGNATURE_PSDT ((uint8_t*)"PSDT")
#define ACPI_SIGNATURE_RASF ((uint8_t*)"RASF")
#define ACPI_SIGNATURE_RSDT ((uint8_t*)"RSDT")
#define ACPI_SIGNATURE_SBST ((uint8_t*)"SBST")
#define ACPI_SIGNATURE_SLIT ((uint8_t*)"SLIT")
#define ACPI_SIGNATURE_SRAT ((uint8_t*)"SRAT")
#define ACPI_SIGNATURE_SSDT ((uint8_t*)"SSDT")
#define ACPI_SIGNATURE_XSDT ((uint8_t*)"XSDT")


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

/* XSDT */
typedef struct xsdt {
	acpi_sdt_t header;
	uint64_t tables[ ];
} __attribute__((packed)) xsdt_t;

/* RSDT */
typedef struct rsdt {
	acpi_sdt_t header;
	uint32_t tables[ ];
} __attribute__((packed)) rsdt_t;

/* Other system descriptor tables */

/* MADT (Multiple APIC Description Table */
typedef struct madt {
	acpi_sdt_t header;

	uint32_t lapic_addr;
	uint32_t flags;


} __attribute__((packed)) madt_t;

typedef struct madt_entry_lapic {
#define MADT_ENTRY_TYPE_LAPIC	(0x00)
	uint8_t type;
	uint8_t length;

	uint8_t apic_processor_id;
	uint8_t apid_id;
	uint32_t flags;
} __attribute__((packed)) madt_entry_lapic_t;

typedef struct madt_entry_ioapic {
#define MADT_ENTRY_TYPE_IOAPIC	(0x01)
	uint8_t type;
	uint8_t length;

	uint8_t io_apic_id;
	uint8_t reserved;
	uint32_t io_apic_addr;
	uint32_t global_system_interrupt_base;
} __attribute__((packed)) madt_entry_ioapic_t;

typedef struct madt_entry_iso {
#define MADT_ENTRY_TYPE_ISO	(0x02)
	uint8_t type;
	uint8_t length;

	uint8_t bus_source;
	uint8_t irq_source;
	uint32_t global_system_interrupt;
	uint16_t flags;
} __attribute__((packed)) madt_entry_iso_t;

typedef struct madt_entry_nmi {
#define MADT_ENTRY_TYPE_NMI	(0x03)
	uint8_t type;
	uint8_t length;

	uint8_t processor;
	uint16_t flags;
	uint8_t lint;
} __attribute__((packed)) madt_entry_nmi_t;

void acpi_init(void);
void acpi_parse_table(acpi_sdt_t *table);
void* acpi_get_table(uint8_t* signature);



