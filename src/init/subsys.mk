
# Set the module name
MODULE := init

FILES := boot.asm multiboot_header.asm

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
