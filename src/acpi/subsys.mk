# Set the module name
MODULE := acpi

FILES := $(wildcard acpi/acpica/source/components/*/*.c)\
	acpi/acpica/osl.c\
	acpi/acpica/acpica.c

SRC += FILES
