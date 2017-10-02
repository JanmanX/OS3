#pragma once

#include <stdint.h>


#define HPET_REGISTER_ID			(0x000)
#define HPET_REGISTER_CONFIG			(0x010)
#define HPET_REGISTER_INTERRUPT_STATUS		(0x020)
#define HPET_REGISTER_MAIN_COUNTER_VALUE	(0x0F0)

#define FEMTOSECOND			(0x38D7EA4C68000) /* 10^15 */

#define HPET_TIMER_OFFSET(x)			(0x100 + (x * 0x20))
#define HPET_TIMER_CONFIG			(0x00)
#define HPET_TIMER_COMPARATOR			(0x08)
#define HPET_TIMER_FSB_IRR			(0x10)


void hpet_init(void);

void hpet_enable(void);
void hpet_disable(void);
uint8_t hpet_num_timers(void);
uint32_t hpet_freq(void);