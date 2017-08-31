#pragma once
#include <stdint.h>

/* Table Signatures */
#define APIC_SIGNATURE_MADT "APIC"
#define APIC_SIGNATURE_BGRT "BGRT"
#define APIC_SIGNATURE_BERT "BERT"
#define APIC_SIGNATURE_CPEP "CPEP"
#define APIC_SIGNATURE_DSDT "DSDT"
#define APIC_SIGNATURE_ECDI "ECDT"
#define APIC_SIGNATURE_EINJ "EINJ"
#define APIC_SIGNATURE_ERST "ERST"
#define APIC_SIGNATURE_FADT "FACP"
#define APIC_SIGNATURE_FACS "FACS"
#define APIC_SIGNATURE_HEST "HEST"
#define APIC_SIGNATURE_MSCT "MSCT"
#define APIC_SIGNATURE_MPST "MPST"
#define APIC_SIGNATURE_OEMx "OEMx"
#define APIC_SIGNATURE_PMTT "PMTT"
#define APIC_SIGNATURE_PSDT "PSDT"
#define APIC_SIGNATURE_RASF "RASF"
#define APIC_SIGNATURE_RSDT "RSDT"
#define APIC_SIGNATURE_SBST "SBST"
#define APIC_SIGNATURE_SLIT "SLIT"
#define APIC_SIGNATURE_SRAT "SRAT"
#define APIC_SIGNATURE_SSDT "SSDT"
#define APIC_SIGNATURE_XSDT "XSDT"


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
	uint64_t tables[0];
} __attribute__((packed)) xsdt_t;

/* RSDT */
typedef struct rsdt {
	acpi_sdt_t header;
	uint32_t tables[0];
} __attribute__((packed)) rsdt_t;

/* Other system descriptor tables */

/* MADT (Multiple APIC Description Table */
typedef struct madt {
	acpi_sdt_t header;

	uint32_t local_controller_addr;
	uint32_t flags;

	/* */
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
void acpi_get_table(uint8_t[4] signature);



