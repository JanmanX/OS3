
# Set the module name
MODULE := cpu

FILES := gdt.c idt.c apic.c tss.c pic.c hpet.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
