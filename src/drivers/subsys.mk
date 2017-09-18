
# Set the module name
MODULE := drivers

FILES := screen.c font.c keyboard.c ps2.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
