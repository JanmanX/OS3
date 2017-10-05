#include "time.h"
#include <drivers/rtc.h>
#include <cpu/hpet.h>


void time_init(void)
{
	rtc_init();
	hpet_init();
}


void sleep(uint64_t ms)
{
	uint64_t start = hpet_get_ms_elapsed();

	while(hpet_get_ms_elapsed() < start + ms)
		;
}
