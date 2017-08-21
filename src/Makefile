# Compiler
CC              = x86_64-elf-gcc
CFLAGS          = -std=gnu99 -ffreestanding -O0 -Wall -Wextra -m64 -g -mno-red-zone -nostdinc -H

# Assembler
AS              = nasm
ASFLAGS         = -f elf64

# Linker
LD              = ld
LDSCRIPT        = ./linker.ld
LDFLAGS         = --script=$(LDSCRIPT) -Map kernel.map -z max-page-size=0x1000


MODULES         := init lib
INC             = -I. -I./include -I./lib
SRC             := main.c

include $(patsubst %, %/subsys.mk, $(MODULES))

OBJS    := $(patsubst %.asm,%.o,$(filter %.asm,$(SRC))) \
        $(patsubst %.S,%.o,$(filter %.S,$(SRC))) \
        $(patsubst %.c,%.o,$(filter %.c,$(SRC)))




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



clean:
	rm -f $(OBJS)
	rm -f *.map
