
# Set the module name
MODULE := cpu

FILES := gdt.c idt.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
