#include "tss.h"
#include <cpu/gdt.h>

tss_t tss[TSS_MAX_SEGMENTS];

void tss_init(void)
{
	/* Clear out memory */
	memset(tss, 0x00, sizeof(tss_t) * TSS_MAX_SEGMENTS);

	/* Create TSS Entry */
	/*
	 * - Create TSS
	 * - LTR
	 */


}

void tss_install(uint8_t num, uint8_t aasd)
{
	gdt_install_tss(GDT_TSS_INDEX, (uint64_t)tss, sizeof(tss_t));



}
