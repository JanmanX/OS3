#include "mem.h"
#include <mem/paging.h>
#include <multiboot2.h>
#include <multiboot_parser.h>
#include <stdint.h>
#include <libc.h>

/* Pointer to the MB MMAP TAG */
static struct multiboot_tag_mmap *mb_tag_mmap = NULL;
static struct multiboot_tag_basic_meminfo *mb_tag_meminfo = NULL;


void mem_init(void)
{
	/* Identity-map, so we can use printing routines */
	paging_init();

	/* Retrieve tags from bootloader */
	mb_tag_meminfo = (struct multiboot_tag_basic_meminfo*)
			multiboot_parser_get_tag(MULTIBOOT_TAG_TYPE_BASIC_MEMINFO);

	mb_tag_mmap = (struct multiboot_mmap_entry*)
			multiboot_parser_get_tag(MULTIBOOT_TAG_TYPE_MMAP);


	if(mb_tag_meminfo == NULL) {
		ERROR("No Multiboot Meminfo. Cannot continue!");
		return;
	}
	if(mb_tag_mmap == NULL) {
		ERROR("No Multiboot MMAP. Cannot continue!");
		return;
	}

	LOGF("MB_TAG_MEMINFO->MEM_UPPER: 0x%x\n", mb_tag_meminfo->mem_upper);
	LOGF("MB_TAG_MMAP @ 0x%x\n", (uint64_t)mb_tag_mmap);

	/* Parse the map */
	multiboot_memory_map_t *mmap = NULL;
	for (mmap = mb_tag_mmap->entries;
	     (uint8_t *) mmap < (uint8_t *) mb_tag_mmap + mb_tag_mmap->size;
	     mmap = (multiboot_memory_map_t*)((uint64_t)mmap+mb_tag_mmap->entry_size)) {

		/* Print split in two, because kprintf cannot
		 * handle this many format strings */
		kprintf (" base_addr = 0x%x%x,"
			 " length = 0x%x%x ",
			 (unsigned) (mmap->addr >> 32),
			 (unsigned) (mmap->addr & 0xffffffff),
			 (unsigned) (mmap->len >> 32),
			 (unsigned) (mmap->len & 0xffffffff));
		if((unsigned)mmap->type < 4) {
			kprintf("type = 0x%x\n", (unsigned) mmap->type);
		}
	}

}
