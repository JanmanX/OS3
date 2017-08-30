#include "acpi.h"
#include <multiboot2.h>
#include <libc.h>

rsdp_t *rsdp;


rsdp_t *acpi_get_rsdp()
{
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
	rsdp = &(mb_acpi->rsdp);

	/* Verify checksum */
	uint8_t checksum = checksum_zero((uint8_t*)rsdp,
					 rsdp->revision == 0 ?
						RSDP_REV_1_SIZE :
						RSDP_REV_2_SIZE);

	if(checksum == 0) {
		/* Valid RSDP found*/
		return rsdp;
	} else {
		LOG("Checksum mismatch for RSDP");
	}
	/* No RSDP found */
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

	/* debug. remove this  */
	char signature[9] = {0};
	char oem[7] = {0};
	memcpy(signature, rsdp->signature, 8);
	LOGF("RSDP: %s\n", signature);

}
