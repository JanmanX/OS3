#pragma once

#include <stdint.h>


#define TSS_MAX_SEGMENTS	(16)

typedef struct tss {
	uint32_t reserved0;

	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;

	uint64_t reserved1;

	uint64_t ist[7];

	uint64_t reserved2;
	uint16_t reserved3;

	uint16_t io_map;

} __attribute__((packed)) tss_t;

void tss_init(void);
