
# Set the module name
MODULE := lib

FILES := libc.c kprintf.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
