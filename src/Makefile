# Compiler
CC              = x86_64-elf-gcc
DEFINES		=
CFLAGS          = -std=gnu11 -ffreestanding -O0 -Wall -Wextra -m64 -g \
			-mno-red-zone -nostdinc -H\

# Assembler
AS              = nasm
ASFLAGS         = -f elf64

# Linker
LD              = ld
LDSCRIPT        = ./linker.ld
LDFLAGS         = --script=$(LDSCRIPT) -Map kernel.map -z max-page-size=0x1000


MODULES         := init include lib drivers mem cpu kernel fs test acpica
INC             = -I./include -I. -I./lib -I./acpica/include
SRC             := main.c

include $(patsubst %, %/subsys.mk, $(MODULES))

OBJS    := $(patsubst %.asm,%.o,$(filter %.asm,$(SRC))) \
        $(patsubst %.S,%.o,$(filter %.S,$(SRC))) \
        $(patsubst %.c,%.o,$(filter %.c,$(SRC)))

.PHONY: clean all tags


# $<  --- Input file
# $@  --- Input filename
%.o: %.asm
	$(AS) $(INC) $(AS_INC) $(ASFLAGS) $< -o $@

%.o: %.S
	$(GAS) -o $@ $(GASFLAGS) -c $<

%.o: %.c
	$(CC) $(INC) $(CFLAGS) -c $< -o $@

all: $(OBJS)
	echo $(OBJS)
	$(LD) -o kernel.bin $(LDFLAGS) $^
	objdump -DMintel kernel.bin > kernel.objdump

print_objs: $(OBJS)
	echo $(FILES)
	echo $(OBJS)

tags:
	rm -f tags
	find . | xargs ctags

clean:
	rm -f $(OBJS)
	rm -f *.map
	rm -f *.objdump




