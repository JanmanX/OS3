#include "tss.h"
#include <cpu/gdt.h>
#include <libc.h>


tss_t tss[TSS_MAX_SEGMENTS];

void tss_reload(uint16_t selector)
{
	asm volatile ("ltr %0"
		      :
		      : "r"(selector)
		      :
		      );
}

void tss_init(void)
{
	/* Clear out memory */
	memset(tss, 0x00, sizeof(tss_t) * TSS_MAX_SEGMENTS);
}

void tss_install(uint8_t num)
{
	gdt_install_tss(GDT_TSS_INDEX, (uint64_t)&tss[num], sizeof(tss_t));

	/* LTR */
	tss_reload(GDT_TSS_OFFSET);
}
