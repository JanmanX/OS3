#pragma once

#include <stdint.h>

/* Number of GiBs to identity-map */
#define NUM_GIB		64

/* Number of entries in each table */
#define PML4T_SIZE	512
#define PDPT_SIZE	512


/* Page table bits */
#define PAGE_PRESENT    (1UL << 0)
#define PAGE_RW         (1UL << 1)
#define PAGE_US         (1UL << 2)
#define PAGE_PWT        (1UL << 3)
#define PAGE_PCD        (1UL << 4)
#define PAGE_A          (1UL << 5)
#define PAGE_D          (1UL << 6)
#define PAGE_PS         (1UL << 7)
#define PAGE_G          (1UL << 8)
#define PAGE_NX         (1UL << 63)



/* Basic Units */
#define KiB                (1024UL)
#define MiB                (1024UL*1024UL)
#define GiB                (1024UL*1024UL*1024UL)


uint64_t pml4t[PML4T_SIZE] __attribute__((aligned(4096))); ;
uint64_t pdpt[PDPT_SIZE] __attribute__((aligned(4096))); ;

void paging_init();


