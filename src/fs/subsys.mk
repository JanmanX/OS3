# Set the module name
MODULE := fs

FILES := fat.c


SRC += $(patsubst %, $(MODULE)/%, $(FILES))
