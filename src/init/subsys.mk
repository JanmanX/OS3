
# Set the module name
MODULE := init

FILES := boot.asm multiboot_header.asm gdt.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
