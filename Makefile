TARGET=os.iso

QEMU=qemu-system-x86_64
QEMU_FLAGS=-m 6G -s -vga std -boot d\
		-cpu host -enable-kvm\
		-drive id=disk,file=$(DISK),if=none,format=raw\
		-device ahci,id=ahci\
		-device ide-hd,drive=disk,bus=ahci.0\
		-machine q35


# The disk the file explorer browses: an MBR with one FAT32 partition
DISK=disk.img

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

# A build product: regenerate rather than edit. Needs dosfstools, mtools and
# util-linux.
$(DISK): tools/mkdisk.sh
	tools/mkdisk.sh $@

disk: $(DISK)

run: iso $(DISK)
	$(QEMU) $(GRUB_KERNEL) $(QEMU_FLAGS) -cdrom  $(TARGET)

debug: iso $(DISK)
	$(QEMU) $(GRUB_KERNEL) $(QEMU_FLAGS)  -S -cdrom  $(TARGET)
debug2: iso
	gdb $(KERNEL) -ex "target remote :1234"\
			-ex "hbreak main"\
			-ex "layout src"\
			-ex "continue"


bochs: iso
	$(BOCHS) -f $(DEBUG_DIR)/bochs/bochs.conf -q

tags:
	find src/ | xargs ctags

clean:
	rm -fv tags
	rm -fv bx_enh_dbg.ini
	rm -fv $(TARGET) $(DISK)
	make -C $(SRC) clean
