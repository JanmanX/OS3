#include "acpica.h"
#include <lib/libc.h>
#include <acpi.h>


#define ACPI_MAX_INIT_TABLES	(16)
static ACPI_TABLE_DESC TableArray[ACPI_MAX_INIT_TABLES];

void* acpica_get_rsdp(void)
{
	/* The value to return */
	rsdp_t* ret = NULL;

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


void acpica_early_init(void)
{
	ACPI_STATUS Status;
	/* Initialize the ACPICA Table Manager and get all ACPI tables */
	Status = AcpiInitializeTables (TableArray, ACPI_MAX_INIT_TABLES, TRUE);
	return (Status);
}

void acpica_init(void)
{
	ACPI_STATUS status;

	/* Initialize the ACPICA subsystem */
	status = AcpiInitializeSubsystem ();

	if (ACPI_FAILURE (status))
	{
		ERRORF("Could not initialize ACPICA. Status: 0x%x\n", status);
	}
	/* Initialize the ACPICA Table Manager and get all ACPI tables */
	status = AcpiInitializeTables (NULL, 16, FALSE);

	if (ACPI_FAILURE (status))
	{
		ERRORF("Could not initialize ACPICA. Status: 0x%x\n", status);
	}

	/* Create the ACPI namespace from ACPI tables */
	status = AcpiLoadTables ();

	if (ACPI_FAILURE (status))
	{
		ERRORF("Could not initialize ACPICA. Status: 0x%x\n", status);
	}
	/* Note: Local handlers should be installed here */
	/* Initialize the ACPI hardware */
	status = AcpiEnableSubsystem (ACPI_FULL_INITIALIZATION);

	if (ACPI_FAILURE (status))
	{
		ERRORF("Could not initialize ACPICA. Status: 0x%x\n", status);
	}
	/* Complete the ACPI namespace object initialization */
	status = AcpiInitializeObjects (ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE (status))
	{
		ERRORF("Could not initialize ACPICA. Status: 0x%x\n", status);
	}
}


ACPI_TABLE_HEADER* acpica_get_table(uint8_t* signature)
{
	ACPI_TABLE_HEADER* h;

	if(AcpiGetTable(signature, 1, &h) != AE_OK) {
		LOG("Table not found.");
		return NULL;
	}

	return h;
}


