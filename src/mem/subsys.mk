
# Set the module name
MODULE := mem

FILES := mem.c paging.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
