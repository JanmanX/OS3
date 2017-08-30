
# Set the module name
MODULE := kernel

FILES := interrupt.c _interrupt.asm exception.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
