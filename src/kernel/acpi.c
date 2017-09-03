#include "acpi.h"
#include <multiboot2.h>
#include <multiboot_parser.h>
#include <libc.h>

/* Root System Description Table Pointer */
rsdp_t *rsdp = NULL;

/* Root System Description Table.
 * Can point to either RSDT or XSDT */
rsdt_t *rsdt = NULL;

rsdp_t *acpi_get_rsdp()
{
	/* The value to return */
	rsdp_t *ret = NULL;

	/* Get the Multiboot ACPI tag */
	/* Since the multiboot tag structure for both old and new ACPI
	 * revisions is the same, just use the same pointer. */
	struct multiboot_tag_new_acpi* mb_acpi;

	mb_acpi = (struct multiboot_tag_new_acpi*)multiboot_parser_get_tag(
						MULTIBOOT_TAG_TYPE_ACPI_NEW);

	/* If no new ACPI, try the old one */
	if(mb_acpi == NULL) {
		LOG("Multiboot New ACPI tag not found. Trying the old one");
		mb_acpi = (struct
			   multiboot_tag_new_acpi*)multiboot_parser_get_tag(
						MULTIBOOT_TAG_TYPE_ACPI_OLD);

		/* Not found either. Bail out */
		if(mb_acpi == NULL) {
			/* TODO: Search manualy in BIOS memory */
			LOG("No MB ACPI found!");
			return NULL;
		}
	}

	/* RSDP is stored in the tag structure */
	ret = (rsdp_t*)&(mb_acpi->rsdp);

	/* Verify checksum */
	uint8_t checksum = checksum_zero((uint8_t*)ret,
					 ret->revision == 0 ?
						RSDP_REV_1_SIZE :
						RSDP_REV_2_SIZE);

	if(checksum == 0) {
		/* Valid RSDP found*/
		LOG("Checksum passed");
		return ret;
	} else {
		LOG("Checksum mismatch for RSDP");
	}
	/* No RSDP found */
	return NULL;
}

rsdt_t *acpi_get_rsdt()
{
	/* The value to return */
	rsdt_t *ret = NULL;

	/* Check for valid RSDP */
	if(rsdp == NULL) {
		LOG("Cannot get Root System Description Table. RSDP == NULL");
		return NULL;
	}

	/* Get RSDT / XSDT */
	if(rsdp->revision == RSDP_REV_1) {
		ret = (rsdt_t*)(uint64_t)(rsdp->rsdt_ptr);
	} else { /* ACPI revision 2.0+*/
		ret = (rsdt_t*)(rsdp->xsdt_ptr);
	}

	/* Verify checksum */
	if(checksum_zero((uint8_t*)ret, ret->header.length) == 0) {
		LOG("[RX]SDT checksum passed");
		return ret;
	} else { /* Checksum failed */
		LOG("[RX]SDT checksum failed");
	}

	/* Nothing found */
	return NULL;
}

void acpi_init(void)
{
	/* Retrieve the RSDP */
	rsdp = acpi_get_rsdp();
	if(rsdp == NULL) {
		ERROR("RSDP NOT FOUND!");
		return;
	}

	/* Get the [RX]SDT */
	rsdt = acpi_get_rsdt();
	if(rsdt == NULL) {
		ERROR("RSDT NOT FOUND!");
		return;
	}
}

void* acpi_get_table(uint8_t *signature)
{
	/* Check that the [RX]SDT table is set */
	if(rsdt == NULL) {
		DEBUG("RSDT not set!");
		return NULL;
	}

	/* Scan the tables, until signature is met */
	acpi_sdt_t *table_ptr = NULL;

	/* The table pointer size is either 4 or 8 bytes, depending on the */
	uint64_t table_pointer_size = rsdp->revision == ACPI_SDT_REV_1
					? sizeof(uint32_t)
					: sizeof(uint64_t);

	/* Calculate the number of table pointers after the header */
	uint64_t num_tables = (rsdt->header.length - sizeof(acpi_sdt_t)) /
				table_pointer_size;

	uint32_t i = 0;
	for(i = 0; i < num_tables; i++) {
		/* Default RSDT table */
		table_ptr = (acpi_sdt_t*)(uint64_t)(rsdt->tables[i]);

		/* If XSDT, the pointer size is 8 bytes */
		if(table_pointer_size == sizeof(uint64_t)) {
			xsdt_t *xsdt = (xsdt_t*)rsdt;
			table_ptr = (acpi_sdt_t*)(xsdt->tables[i]);
		}

		/* Compare signature */
		if(memcmp(table_ptr->signature, signature, ACPI_SIGNATURE_LEN)
		   == 0) {

			/* Checksum */
			if(checksum_zero(table_ptr, table_ptr->length) == 0) {
				return table_ptr;
			} else {
				LOG("Checksum did not match!");
			}
		}
	}

	LOG("APIC Table not found.");
	return NULL;
}
