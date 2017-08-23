
# Set the module name
MODULE := mem

FILES := mem.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
