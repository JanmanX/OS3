#include "hpet.h"
#include <kernel/acpi.h>
#include <libc.h>
#include <kernel/interrupt.h>
#include <cpu/apic.h>


hpett_t* hpet = NULL;
uint64_t* hpet_base = NULL;
uint16_t hpet_min_tick = 0x00;
uint64_t ms_elapsed = 0x00;

uint64_t hpet_get_ms_elapsed(void)
{
	return ms_elapsed;
}

void hpet_enable(void)
{
	ASSERT(hpet_base != NULL, "HPET not initialized");

	uint64_t config = GET_UINT64(hpet_base, HPET_REGISTER_CONFIG);
	config |= 0x01; /* Set bit 0 */

	/* If not capable to re-route legacy timers, support legacy routing */
	if(hpet_legacy_replacement_route_capable() == 0x00) {
		config |= (0x01 << 1);
	}

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
	ms_elapsed++;
	lapic_eoi();
}

uint8_t hpet_timer_handler2(pt_regs_t* regs)
{
	//	LOGF("2",NULL);
	lapic_eoi();
}


void hpet_timer_setup(uint8_t timer,
		      interrupt_handler_t timer_handler,
		      uint8_t periodic,
		      uint64_t hz) {
	if(timer >= hpet_num_timers()) {
		LOGF("Timer 0x%x does not exist!\n", timer);
		return;
	}

	/* Check if timer supports the operations */
	uint64_t timer_config =
		GET_UINT64(hpet_base, HPET_TIMER_OFFSET(timer) + HPET_TIMER_CONFIG);

	/* Check for periodic support */
	if(periodic && (timer_config >> 4) & 1 == 0) {
		ERROR("Timer does not support periodic interrupt!");
	}

	uint8_t i = 0x00;

	/* Get a free interrupt vector */
	uint32_t vector = interrupt_get_free_vector();

	/* Get the IRQ
	 * If legacy, use ISA IRQs */
	uint64_t irq;
	if(timer < 2 && hpet_legacy_replacement_route_capable() == 0) {
		LOG("ISA Timer");
		irq = timer == 0
			? HPET_TIMER0_IRQ
			: HPET_TIMER1_IRQ;

		/* In case of rewiring */
		irq = ioapic_get_iso(irq);

		/* Check if actually free */
		if(ioapic_is_irq_free(irq) == FALSE) {
			ERROR("HPET ISA IRQ taken!");
			return;
		}
	} else {
		/* Get the upper 4 bytes of CONFIG register (Interrupt Routing
		 * capability) */
		uint32_t available_irqs = GET_UINT32(hpet_base, (HPET_TIMER_OFFSET(timer)
								 + HPET_TIMER_CONFIG + 4));

		LOGF("avail_irq: 0x%x\n", available_irqs);

		/* Iterate over available*/
		irq = 0xFF;
		for(i = 0; i < 32; i++) {
			if((available_irqs >> i) & 1) {
				if(ioapic_is_irq_free(ioapic_get_iso(i))) {
					/* IRQ free, we can install it */
					irq = ioapic_get_iso(i);
				}
			}
		}


		ASSERT(irq > 31, "HPET Timer invalid IRQ");

		/* No free IRQ found. Bail out */
		if(irq == 0xFF) {
			LOG("No more IRQs");
			return;
		}
	}

	/* Setup timer */
	uint64_t main_counter = GET_UINT64(hpet_base,
					   HPET_REGISTER_MAIN_COUNTER_VALUE);
	uint64_t period = FEMTOSECOND / hpet_freq() / hz;

	GET_UINT64(hpet_base, HPET_TIMER_OFFSET(timer) + HPET_TIMER_CONFIG)
		= (1 << 8)		/* 64 bit timer */
		| (irq << 9)		/* Interrupt (ignored on ISA)  */
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

	LOGF("Installing timer 0x%x on irq 0x%x to vector 0x%x\n", timer, irq,
	     vector);

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


uint8_t hpet_legacy_replacement_route_capable(void)
{
	return GET_UINT32(hpet_base, HPET_REGISTER_ID) >> 15;
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

	/* Disable all timers  interrupts */
	uint32_t general_interrupt = 0x00; /* Reset buttom 32 bits */
	GET_UINT32(hpet_base, HPET_REGISTER_INTERRUPT_STATUS) = general_interrupt;
	/* Minimum tick */
	hpet_min_tick = hpet->minimum_tick;

	/* Counter size */
	uint8_t counter_size = ((GET_UINT32(hpet_base, HPET_REGISTER_ID) >> 0x0D)
				& 0x01);
	if(counter_size == 0x00) {
		ERROR("HPET counter 32bit only!");
	}

	/* Setup timers */
	/* Timer 0: 1000hz timer */
	hpet_timer_setup(0, hpet_timer_handler, 1, 1000);

	/* Timer 1: ... */
	//	hpet_timer_setup(1, hpet_timer_handler2, 1, 100);


	/* go go go */
	hpet_enable();

}


