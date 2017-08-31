#include "acpi.h"
#include <multiboot2.h>
#include <libc.h>

/* Root System Description Table Pointer */
rsdp_t *rsdp;

/* Root System Description Table.
 * Can point to either RSDT or XSDT */
rsdt_t *rsdt;

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
			return NULL;
		}
	}

	/* RSDP is stored in the tag structure */
	ret = &(mb_acpi->rsdp);

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
		LOG("Cannot get Root System Description Table. RSDP == NULLL");
		return NULL;
	}

	/* Get RSDT / XSDT */
	if(rsdp->revision == RSDP_REV_1) {
		ret = (rsdt_t*)(rsdp->rsdt_ptr);
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
