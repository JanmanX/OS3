#include "hpet.h"
#include <kernel/acpi.h>
#include <libc.h>
#include <kernel/interrupt.h>
#include <cpu/apic.h>


hpett_t* hpet = NULL;
uint64_t* hpet_base = NULL;
uint16_t hpet_min_tick = 0x00;

void hpet_enable(void)
{
	ASSERT(hpet_base != NULL, "HPET not initialized");

	uint64_t config = GET_UINT64(hpet_base, HPET_REGISTER_CONFIG);
	config |= 0x01; /* Set bit 0 to 1 */
	GET_UINT64(hpet_base, HPET_REGISTER_CONFIG) = config;
}

void hpet_disable(void)
{
	ASSERT(hpet_base != NULL, "HPET not initialized");

	uint64_t config = GET_UINT64(hpet_base, HPET_REGISTER_CONFIG);
	config &= ~((uint64_t)0x01); /* Reset bit 0 */
	GET_UINT64(hpet_base, HPET_REGISTER_CONFIG) = config;
}

uint8_t hpet_timer_handler(pt_regs_t* regs)
{
	LOG("HPET Timer interrupt");
	lapic_eoi();
}

uint8_t hpet_timer_handler2(pt_regs_t* regs)
{
	LOG("HPET Timer 2 interrupt");
	lapic_eoi();
}


void hpet_timer_setup(uint8_t timer,
		      interrupt_handler_t timer_handler,
		      uint8_t periodic,
		      uint64_t hz) {
	uint8_t i = 0x00;

	/* Get the upper 4 bytes of CONFIG register (Interrupt Routing
	 * capability) */
	uint32_t available_irqs = GET_UINT32(hpet_base, (HPET_TIMER_OFFSET(timer)
					     + HPET_TIMER_CONFIG + 4));
	/* Get a free interrupt vector */
	uint32_t vector = interrupt_get_free_vector();


	/* Iterate over available*/
	uint64_t irq = 0x00;
	for(i = 0; i < 32; i++) {
		if((available_irqs >> i) & 1) {
			if(ioapic_is_irq_free(i)) {
				/* IRQ free, we can install it */
				irq = i;
			}
		}
	}

	ASSERT(i > 31, "HPET Timer invalid IRQ");

	/* Setup timer */
	uint64_t main_counter = GET_UINT64(hpet_base,
					   HPET_REGISTER_MAIN_COUNTER_VALUE);
	uint64_t period = FEMTOSECOND / hpet_freq() / hz;

	GET_UINT64(hpet_base, HPET_TIMER_OFFSET(timer) + HPET_TIMER_CONFIG)
		= (1 << 8)		/* 64 bit timer */
		| (irq << 9)		/* Interrupt */
		| (1 << 6)		/* next write to accumulator only */
		| ((uint64_t)periodic << 3)	/* periodic */
		| (1 << 2);		/* generate interrupts */

	/* Set the accumulator (because we wrote bit 6 above */
	GET_UINT64(hpet_base, HPET_TIMER_OFFSET(timer) + HPET_TIMER_COMPARATOR)
		= period;

	/* Set the comparator */
	GET_UINT64(hpet_base, HPET_TIMER_OFFSET(timer) + HPET_TIMER_COMPARATOR)
		= period + main_counter;

	/* Enable timer interrupt in ISR */
	uint64_t isr = GET_UINT64(hpet_base, HPET_REGISTER_INTERRUPT_STATUS);
	isr |= (1 << irq); /* Set the n'th IRQ bit */
	GET_UINT64(hpet_base, HPET_REGISTER_INTERRUPT_STATUS) = isr;


	interrupt_install(vector, timer_handler);

	ioapic_set_irq(irq, LAPIC_BSP, vector);

}

uint8_t hpet_num_timers(void)
{
	ASSERT(hpet_base != NULL, "hpet_base not set!");
	return (GET_UINT32(hpet_base, HPET_REGISTER_ID) >> 0x08) & 0x1F;
}

uint32_t hpet_freq(void)
{
	ASSERT(hpet_base != NULL, "hpet_base not set!");
	return FEMTOSECOND / GET_UINT32(hpet_base, HPET_REGISTER_ID + 4);
}


void hpet_init(void)
{
	hpet = acpi_get_table(ACPI_SIGNATURE_HPET);
	ASSERT(hpet != NULL, "Could not find HPET");

	/* If lower byte is non-zero, HPET is not memory mapped */
	if(hpet->base_address0 & 0xFF != 0x00) {
		ERROR("HPET is not memory mapped!");
		return;
	}

	/* Read HPET INFO */
	hpet_base = (uint64_t*)((uint64_t)hpet->base_address2 << 32
				| (uint64_t)hpet->base_address1);

	/* Reset Main counter */
	hpet_disable(); /* Enable at end of this function */
	GET_UINT64(hpet_base, HPET_REGISTER_MAIN_COUNTER_VALUE) = (uint64_t)0;

	/* Disable legacy routing */
	uint64_t config = GET_UINT32(hpet_base, HPET_REGISTER_CONFIG);
	config &= ~((uint64_t)0x02); /* Disable legacy routing */
	GET_UINT32(hpet_base, HPET_REGISTER_CONFIG) = config;

	/* Disable all timers  interrupts */
	uint32_t general_interrupt = GET_UINT64(hpet_base,
						HPET_REGISTER_INTERRUPT_STATUS);
	general_interrupt &= ~(uint64_t)0xFFFFFFFF; /* Reset buttom 32 bits */
	GET_UINT64(hpet_base, HPET_REGISTER_INTERRUPT_STATUS) =
		general_interrupt;


	/* Minimum tick */
	hpet_min_tick = hpet->minimum_tick;

	/* Counter size */
	uint8_t counter_size = ((GET_UINT32(hpet_base, HPET_REGISTER_ID) >> 0x0D)
				& 0x01);
	if(counter_size == 0x00) {
		ERROR("HPET counter 32bit only!");
	}


	/* Setup timers */
	hpet_timer_setup(0, hpet_timer_handler, 10, 10000);
	hpet_timer_setup(1, hpet_timer_handler2, 0, 100);


	LOGF("FREQ: 0x%x\n", hpet_freq());

	/* go go go */
	hpet_enable();
}


