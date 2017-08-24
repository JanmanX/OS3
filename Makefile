TARGET=os.iso

QEMU=qemu-system-x86_64
QEMU_FLAGS=-m 10G -s -vga std # -cpu host -enable-kvm

GRUB=grub2
BOCHS=bochs
DEBUG_DIR=./debug

SRC	:= src
KERNEL	:= $(SRC)/kernel.bin



all:
	make -C $(SRC)

iso: all
	cp $(KERNEL) cd/boot/
	$(GRUB)-mkrescue -o $(TARGET) cd/

run: iso
	$(QEMU) $(GRUB_KERNEL) $(QEMU_FLAGS) -cdrom  $(TARGET)

debug: iso
	$(QEMU) $(GRUB_KERNEL) $(QEMU_FLAGS) $(QEMU_DRIVE)  -S -cdrom  $(TARGET)



bochs: iso
	$(BOCHS) -f $(DEBUG_DIR)/bochs/bochs.conf -q

tags:
	find os/ | xargs ctags

clean:
	rm -fv bx_enh_dbg.ini
	rm -fv $(TARGET)
	make -C $(SRC) clean
