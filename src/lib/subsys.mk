
# Set the module name
MODULE := lib

FILES := libc.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
