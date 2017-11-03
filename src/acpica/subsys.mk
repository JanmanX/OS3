# Set the module name
MODULE := acpica


FILES := $(wildcard acpica/components/*/*.c) acpica/osl.c

SRC += FILES
