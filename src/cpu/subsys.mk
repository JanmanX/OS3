
# Set the module name
MODULE := cpu

FILES := gdt.c idt.c apic.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
