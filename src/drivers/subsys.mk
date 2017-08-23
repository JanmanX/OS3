
# Set the module name
MODULE := drivers

FILES := screen.c font.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
