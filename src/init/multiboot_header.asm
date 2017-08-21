; How many bytes from the start of the file we search for the header.
%define MULTIBOOT_SEARCH			32768
%define MULTIBOOT_HEADER_ALIGN			8

; The magic field should contain this.
%define MULTIBOOT2_HEADER_MAGIC			0xe85250d6

; This should be in %eax.
%define MULTIBOOT2_BOOTLOADER_MAGIC		0x36d76289

%define MULTIBOOT_ARCHITECTURE_I386  0
%define MULTIBOOT_ARCHITECTURE_MIPS32  4
%define MULTIBOOT_HEADER_TAG_OPTIONAL 1

[BITS 64]
section .multiboot_header
header_start:
	multiboot_magic 	dd MULTIBOOT2_HEADER_MAGIC
	multiboot_arch		dd MULTIBOOT_ARCHITECTURE_I386
	multiboot_header_len	dd header_end - header_start
	multiboot_header_chksm	dd 0x100000000 - (MULTIBOOT2_HEADER_MAGIC \
						+ MULTIBOOT_ARCHITECTURE_I386 \
						+ (header_end - header_start))

	; other tags here

	; End tag
	dw 0x00	; type
	dw 0x00 ; flags
	dd 0x08	; size

header_end:
