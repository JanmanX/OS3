
# Set the module name
MODULE := gui

FILES := gfx.c cursor.c compositor.c


SRC += $(patsubst %, $(MODULE)/%, $(FILES))
