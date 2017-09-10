#pragma once

#include <stdint.h>

#define KERNEL_MEM_START 0x100000

void mem_init(void);

void *malloc(uint64_t size);
void free(void* ptr);
