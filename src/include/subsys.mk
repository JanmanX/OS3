
# Set the module name
MODULE := include

FILES := list.c tree.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
