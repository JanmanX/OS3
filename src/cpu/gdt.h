#pragma once

#include <stdint.h>

#define GDT_MAX_DESCRIPTORS 32

/* Descriptor indexes */
#define GDT_NULL_INDEX			(0x00)
#define GDT_KERNEL_CODE_INDEX		(0x01)
#define GDT_KERNEL_DATA_INDEX		(0x02)
#define GDT_USER_CODE_INDEX		(0x03)
#define GDT_USER_DATA_INDEX		(0x04)
#define GDT_TSS_INDEX			(0x05)


/* Descriptor Offsets */
#define GDT_NULL_OFFSET			(GDT_NULL_INDEX << 3)
#define GDT_KERNEL_CODE_OFFSET		(GDT_KERNEL_CODE_INDEX << 3)
#define GDT_KERNEL_DATA_OFFSET		(GDT_KERNEL_CODE_INDEX << 3)
#define GDT_USER_CODE_OFFSET		(GDT_USER_CODE_INDEX << 3)
#define GDT_USER_DATA_OFFSET		(GDT_USER_DATA_INDEX << 3)
#define GDT_TSS_OFFSET			(GDT_TSS_INDEX	<< 3)


typedef struct gdt_entry {
	uint16_t limit0;
	uint16_t base0;
	uint8_t base1;
	uint8_t access;
	uint8_t limit1:4;
	uint8_t flags:4;
	uint8_t base2;
} __attribute__((packed)) gdt_entry_t;


typedef struct gdt_system_entry {
	uint16_t limit0;
	uint16_t base0;

	uint8_t base1;
	uint8_t type0;

	uint8_t limit1;
	uint8_t base2;

	uint32_t base3;
	uint32_t reserved;
} __attribute__((packed)) gdt_system_entry_t;


typedef struct {
	uint16_t limit;
	uint64_t base;
} __attribute__((packed)) gdt_pointer_t;


void gdt_load(uint64_t new_gdt);
void gdt_init();
void gdt_install_descriptor(uint8_t num, uint32_t base, uint32_t limit,
			    uint8_t access, uint8_t flags);
void gdt_install_tss(uint8_t num, uint64_t base, uint64_t limit);

