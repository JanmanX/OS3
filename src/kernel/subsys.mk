# Set the module name
MODULE := kernel

FILES := interrupt.c _interrupt.asm exception.c acpi.c time.c
SRC += $(patsubst %, $(MODULE)/%, $(FILES))

