
# Set the module name
MODULE := drivers

FILES := screen.c font.c ps2keyboard.c ps2.c ps2mouse.c pci.c rtc.c ahci.c


SRC += $(patsubst %, $(MODULE)/%, $(FILES))
