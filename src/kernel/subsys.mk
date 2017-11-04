# Set the module name
MODULE := kernel

FILES := interrupt.c _interrupt.asm exception.c time.c
SRC += $(patsubst %, $(MODULE)/%, $(FILES))

