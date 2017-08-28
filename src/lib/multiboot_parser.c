#include "multiboot_parser.h"
#include <multiboot2.h>
#include <libc.h>

static struct multiboot_tag *mb_first_tag = NULL;
static uint32_t mb_info_size = 0;


void multiboot_parser_init(uint8_t* mb_info_struct_addr)
{
	/* Get the size */
	mb_info_size = *(uint32_t*)mb_info_struct_addr;

	/* Get the pointer to the first tag */
	mb_first_tag = (struct multiboot_tag *) (mb_info_struct_addr + 8);
}

struct multiboot_tag *multiboot_parser_get_tag(uint32_t tag)
{
	struct multiboot_tag *mb_tag;
	for (mb_tag = mb_first_tag;
	     mb_tag->type != MULTIBOOT_TAG_TYPE_END;
	     mb_tag = (struct multiboot_tag *) ((multiboot_uint8_t *) mb_tag
					     + ((mb_tag->size + 7) & ~7)))
	{
		/* Check tag */
		if(mb_tag->type == tag) {
			return mb_tag;
		}
	}
#if 0

		switch (tag->type)
		{
		case MULTIBOOT_TAG_TYPE_ACPI_NEW:
			kprintf("ACPI_NEW = %s\n",
				((struct
				  multiboot_tag_string*)tag)->string);
			break;
		case MULTIBOOT_TAG_TYPE_ACPI_OLD:
			kprintf("ACPI_OLD = %s\n",
				((struct
				  multiboot_tag_string*)tag)->string);
			break;


		case MULTIBOOT_TAG_TYPE_CMDLINE:
			kprintf ("Command line = %s\n",
				 ((struct multiboot_tag_string *) tag)->string);
			break;
		case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
			kprintf ("Boot loader name = %s\n",
				 ((struct multiboot_tag_string *) tag)->string);
			break;
		case MULTIBOOT_TAG_TYPE_MODULE:
			kprintf ("Module at 0x%x-0x%x. Command line %s\n",
				 ((struct multiboot_tag_module *) tag)->mod_start,
				 ((struct multiboot_tag_module *) tag)->mod_end,
				 ((struct multiboot_tag_module *) tag)->cmdline);
			HALT;
			break;
		case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
			kprintf ("mem_lower = %uKB, mem_upper = %uKB\n",
				 ((struct multiboot_tag_basic_meminfo *) tag)->mem_lower,
				 ((struct multiboot_tag_basic_meminfo *) tag)->mem_upper);
			break;
		case MULTIBOOT_TAG_TYPE_BOOTDEV:
			kprintf ("Boot device 0x%x,%u,%u\n",
				 ((struct multiboot_tag_bootdev *) tag)->biosdev,
				 ((struct multiboot_tag_bootdev *) tag)->slice,
				 ((struct multiboot_tag_bootdev *) tag)->part);
			break;
		case MULTIBOOT_TAG_TYPE_MMAP: {
			multiboot_memory_map_t *mmap;

			kprintf ("mmap\n");

			for (mmap = ((struct multiboot_tag_mmap *) tag)->entries;
			     (multiboot_uint8_t *) mmap
			     < (multiboot_uint8_t *) tag + tag->size;
			     mmap = (multiboot_memory_map_t *)
			     ((unsigned long) mmap
			      + ((struct multiboot_tag_mmap *) tag)->entry_size)) {
				/* Print split in two, because kprintf cannot
				 * handle this many format strings */
				kprintf (" base_addr = 0x%x%x,"
					 " length = 0x%x%x ",
					 (unsigned) (mmap->addr >> 32),
					 (unsigned) (mmap->addr & 0xffffffff),
					 (unsigned) (mmap->len >> 32),
					 (unsigned) (mmap->len & 0xffffffff));
				if((unsigned)mmap->type < 4) {
					kprintf("type = 0x%x = %s\n",
						(unsigned) mmap->type,
						mmap_type_to_string[mmap->type]);
				} else {
					kprintf("type = 0x%x\n",
						(unsigned) mmap->type);

				}
			}

		}
			break;
		case MULTIBOOT_TAG_TYPE_VBE: {
			struct multiboot_tag_vbe *tag_vbe
				= (struct multiboot_tag_vbe *) tag;
			uint16_t vbe_mode = *(uint16_t*)(tag_vbe + 8);
			kprintf("VBE TYPE: 0x%x\n", vbe_mode);
			break;
		}

		case MULTIBOOT_TAG_TYPE_FRAMEBUFFER: {
			struct multiboot_tag_framebuffer *tag_fb
				= (struct multiboot_tag_framebuffer *) tag;

			screen_init(tag_fb);

			break;
		}
		}

	}

#endif

	/* Nothing found */
	return NULL;
}


