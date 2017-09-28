#pragma once

#include <stdint.h>
#include <types.h>

#define KERNEL_MEM_START	(0x100000)
#define KERNEL_HEAP_END		(GiB/2)
#define MALLOC_MIN_SIZE		(0x04)
#define MCB_FREE		(0x00)
#define MCB_USED		(0x01)

typedef struct memory_control_block {
	uint8_t used;
	uint64_t size;
	struct memory_control_block *next;
	/*	struct memory_control_block *prev; /* Not included. lets keep it
	 *						simple for now */

} __attribute__((packed))  memory_control_block_t;
typedef memory_control_block_t mcb_t;

void mem_init(void);

uintptr_t *malloc(uint64_t size);
void free(uintptr_t* ptr);

/* Miscellaneous functions */
uint8_t* mem_under_mib(uint64_t length);
