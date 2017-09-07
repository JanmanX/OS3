#include "paging.h"
#include <stdint.h>
#include <libc.h>


static void paging_reload_pml4t(uint64_t pml4t_start_phys)
{
	asm volatile("mov %0, %%rax;\
		     mov %%rax, %%cr3;"
		     :
		     : "r"(pml4t_start_phys)
		     : "%rax"
		    );
}

void paging_init()
{
	/* Clear out the tables */
	memset((uint8_t*)pml4t, 0x00, sizeof(uint64_t) * PML4T_SIZE);
	memset((uint8_t*)pdpt, 0x00, sizeof(uint64_t) * PML4T_SIZE);

	uint64_t i = 0;
	/* Setup PML4T */
	pml4t[0] = ((uint64_t)pdpt) | PAGE_PRESENT | PAGE_RW;

	/* Setup PDPT*/
	for(i = 0; i < PDPT_SIZE; i++) {
		pdpt[i] = (i * GiB) | PAGE_PRESENT | PAGE_RW | PAGE_PS;
	}

	/* reload tables */
	paging_reload_pml4t((uint64_t)pml4t);
}

