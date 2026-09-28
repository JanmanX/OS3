# Compiler
CC              = gcc
DEFINES		= -D ACOS
# -MMD -MP: emit a .d file beside every object listing the headers it used, so
# that editing a header rebuilds what included it. Without this a header change
# is invisible to make and you silently link stale objects.
CFLAGS          = -std=gnu11 -O0 -Wall -Wextra -m64 -g \
			-ffreestanding\
			-fno-pie -fno-stack-protector\
			-mno-red-zone -nostdinc\
			-MMD -MP\
			$(DEFINES)

# Rust compiler
RUSTC		= rustc
RUSTFLAGS	=


# Assembler
AS              = nasm
ASFLAGS         = -f elf64

# Linker
LD              = ld
LDSCRIPT        = ./linker.ld
LDFLAGS         = --script=$(LDSCRIPT) -Map kernel.map -z max-page-size=0x1000 -z noexecstack\
			-nostdlib



MODULES         := init include lib drivers mem cpu kernel test acpi gui
INC             = -I./include -I. -I./lib -I./acpi/acpica/source/include
SRC             := main.c

include $(patsubst %, %/subsys.mk, $(MODULES))

OBJS    := $(patsubst %.asm,%.o,$(filter %.asm,$(SRC))) \
        $(patsubst %.S,%.o,$(filter %.S,$(SRC))) \
        $(patsubst %.c,%.o,$(filter %.c,$(SRC)))

# One per C object. The assembler ones never appear, which -include tolerates.
DEPS    := $(patsubst %.c,%.d,$(filter %.c,$(SRC)))

.PHONY: clean all tags


# $<  --- Input file
# $@  --- Input filename
%.o: %.asm
	$(AS) $(INC) $(AS_INC) $(ASFLAGS) $< -o $@

%.o: %.S
	$(GAS) -o $@ $(GASFLAGS) -c $<

%.o: %.c
	$(CC) $(INC) $(CFLAGS) -c $< -o $@

%.o: %.rs
	$(RUSTC) $(RUSTFLAGS) -o $@ $<


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
	rm -f $(DEPS)
	rm -f *.map
	rm -f *.objdump


# Pulled in last: each .d turns into rules saying which headers an object
# depends on. Missing on the first build, which is exactly when it does not
# matter, because everything is being compiled anyway.
-include $(DEPS)




