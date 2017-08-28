
# Set the module name
MODULE := cpu

FILES := gdt.c idt.c interrupt.c _interrupt.asm

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
