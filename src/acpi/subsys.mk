# Set the module name
MODULE := acpi

FILES := $(wildcard $(MODULE)/acpica/source/components/dispatcher/*.c)\
	$(wildcard $(MODULE)/acpica/source/components/events/*.c)\
	$(wildcard $(MODULE)/acpica/source/components/executer/*.c)\
	$(wildcard $(MODULE)/acpica/source/components/hardware/*.c)\
	$(wildcard $(MODULE)/acpica/source/components/namespace/*.c)\
	$(wildcard $(MODULE)/acpica/source/components/parser/*.c)\
	$(wildcard $(MODULE)/acpica/source/components/resources/*.c)\
	$(wildcard $(MODULE)/acpica/source/components/tables/*.c)\
	$(wildcard $(MODULE)/acpica/source/components/utilities/*.c)\
	$(MODULE)/osl.c\
	$(MODULE)/acpica.c

SRC += $(FILES)
