#include "mem.h"
#include <mem/paging.h>
#include <multiboot2.h>
#include <multiboot_parser.h>
#include <stdint.h>
#include <libc.h>
#include <types.h>


/* Pointer to the MB MMAP TAG */
static struct multiboot_tag_mmap *mb_tag_mmap = NULL;
static struct multiboot_tag_basic_meminfo *mb_tag_meminfo = NULL;

/* End of kernel. Defined in linker script */
extern uint64_t kernel_end_addr;
static uint8_t mem_heap_start = 0x00;

/* Heap units */
struct memory_control_block *heap = NULL;

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
		kprintf (" base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n",
			 (unsigned) (mmap->addr >> 32),
			 (unsigned) (mmap->addr & 0xffffffff),
			 (unsigned) (mmap->len >> 32),
			 (unsigned) (mmap->len & 0xffffffff),
			 (unsigned) mmap->type);
	}


	/* Initialize dynamic memory */
	heap_init((uint64_t*)&kernel_end_addr,
		  KERNEL_HEAP_END - (uint64_t)(&kernel_end_addr));
}


void heap_init(uint64_t *start, uint64_t size)
{
	LOGF("kernel_end_addr: 0x%x\n", &kernel_end_addr);
	LOGF("size of heap: 0x%x\n", size);

	heap = (mcb_t*)start;
	heap->used = 0x00;
	heap->size = size;
	heap->next = NULL;

	LOGF("heap @ 0x%x\theap->used = 0x%x\theap->size = 0x%x\n",
	     heap, heap->used, heap->size);

}

/* Collapse consecutive free blocks into one block */
void mem_heap_collapse(void)
{
	mcb_t* node = NULL;

	for(node = heap; node != NULL; node = node->next) {
		/* No next block = no more to collapse. Exit */
		if(node->next == NULL) {
			break;
		}

		/* If both next blocks are free, collapse them into 1 */
		while(node->used == 0x00 && node->next->used == 0x00) {
			node->size += sizeof(mcb_t) + node->next->size;
			node->next = node->next->next;
		}
	}
}


uintptr_t* malloc(uint64_t size)
{
	mcb_t* node = NULL;
	uint8_t retry = 0x01;

RETRY:
	for(node = heap; node != NULL; node = node->next) {
		if(node->used == 0x00 && node->size >= size) {
			/*
			LOGF("node @ 0x%x\tnode->used = 0x%x\tnode->size = 0x%x\n",
			     node, node->used, node->size);
			*/


			/* If next = NULL, create a new one */
			if(node->next == NULL) {
				node->next = (mcb_t*)(((uint64_t)node)
						      + sizeof(mcb_t)
						      + size);
				node->next->used = 0x00;
				node->next->next = NULL;
				node->next->size = node->size
					- sizeof(mcb_t)
					- size;

			}

			node->used = 0x01;
			node->size = size;
			return (uintptr_t*)(((uint64_t)node) + sizeof(mcb_t));
		}
	}


	/* We have reached this point without any valid pointer.
	 * Collapse memory and try again. */
	if(retry) {
		retry = 0x00;
		mem_heap_collapse();
		goto RETRY;
	}

	return NULL;
}



void free(uintptr_t *ptr)
{
	/* Sanity check */
	if((uint64_t)ptr < (uint64_t)&kernel_end_addr ||
	   (uint64_t)ptr > KERNEL_HEAP_END) {
		LOG("Cannot free outside heap bounds!");
		return;
	}

	/* Get the address of the MCB */
	mcb_t* node = (mcb_t*)(((uint64_t)ptr) - sizeof(mcb_t));
	node->used = 0x00;
}

uint8_t* mem_under_mib(uint64_t length)
{
	ASSERT(mb_tag_mmap != NULL, "mb_tag_mmap == NULL");

	/* Parse the map */
	multiboot_memory_map_t *mmap = NULL;
	for (mmap = mb_tag_mmap->entries;
	     (uint8_t *) mmap < (uint8_t *) mb_tag_mmap + mb_tag_mmap->size;
	     mmap = (multiboot_memory_map_t*)((uint64_t)mmap+mb_tag_mmap->entry_size)) {

		if(mmap->addr < MiB
		   && mmap->len >= length) {
			return (uint8_t*)mmap->addr;
		}

	}
}
