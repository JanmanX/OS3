
# Set the module name
MODULE := test

FILES := test.c tree_test.c list_test.c

SRC += $(patsubst %, $(MODULE)/%, $(FILES))
