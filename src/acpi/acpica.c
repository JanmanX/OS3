#include "acpica.h"
#include <acpi.h>

void acpica_init(void)
{
	ACPI_STATUS status;

	/* Initialize the ACPICA subsystem */
	status = AcpiInitializeSubsystem ();
	if (ACPI_FAILURE (status))
	{
		ERROR("Could not initialize ACPICA");
	}
	/* Initialize the ACPICA Table Manager and get all ACPI tables */
	status = AcpiInitializeTables (NULL, 16, FALSE);
	if (ACPI_FAILURE (status))
	{
		ERROR("Could not initialize ACPICA");
	}

	/* Create the ACPI namespace from ACPI tables */
	status = AcpiLoadTables ();
	if (ACPI_FAILURE (status))
	{
		ERROR("Could not initialize ACPICA");
	}
	/* Note: Local handlers should be installed here */
	/* Initialize the ACPI hardware */
	status = AcpiEnableSubsystem (ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE (status))
	{
		ERROR("Could not initialize ACPICA");
	}
	/* Complete the ACPI namespace object initialization */
	status = AcpiInitializeObjects (ACPI_FULL_INITIALIZATION);
	if (ACPI_FAILURE (status))
	{
		ERROR("Could not initialize ACPICA");
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


