
# Set the module name
MODULE := drivers

FILES := screen.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
