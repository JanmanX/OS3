
# Set the module name
MODULE := drivers

FILES := screen.c font.c ps2keyboard.c ps2.c ps2mouse.c pci.c sata.c rtc.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
