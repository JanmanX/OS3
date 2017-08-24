
# Set the module name
MODULE := lib

FILES := libc.c kprintf.c multiboot_parser.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
