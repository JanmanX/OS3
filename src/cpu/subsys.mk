
# Set the module name
MODULE := cpu

FILES := gdt.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
