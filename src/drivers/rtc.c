#include "rtc.h"
#include <libc.h>

/* XXX: Do we just assume RTC exists? */
void rtc_init(void)
{
	uint8_t b = (1 << 7) /* Enable updates*/
		| (1 << 2) /* Binary values */
		| (1 << 1) /* 24 hour mode */
		| 0x00; /* No daylight savings time */

	rtc_write(RTC_REGISTER_B, b);

	if(b != rtc_read(RTC_REGISTER_B)) {
		LOG("Something is not set in RTC.");
	}
}

uint8_t rtc_read(const uint8_t reg)
{
	outb(RTC_ADDRESS_PORT, reg);
	return inb(RTC_DATA_PORT);
}
uint8_t rtc_write(const uint8_t reg, const uint8_t val)
{
	outb(RTC_ADDRESS_PORT, reg);
	outb(RTC_DATA_PORT, val);
}

uint8_t rtc_disable_updates(void)
{
	uint8_t b = rtc_read(RTC_REGISTER_B);
	b &= ~(1 << 7); /* Disable bit 7*/
	rtc_write(RTC_REGISTER_B, b);
}

uint8_t rtc_enable_updates(void)
{
	uint8_t b = rtc_read(RTC_REGISTER_B);
	b |= (1 << 7); /* enable bit 7*/
	rtc_write(RTC_REGISTER_B, b);
}

uint8_t rtc_get_seconds(void)
{
	uint8_t ret = 0x00;
	rtc_disable_updates();
	ret = rtc_read(RTC_REGISTER_SECONDS);
	rtc_enable_updates();
	return ret;

}

uint8_t rtc_get_minutes(void)
{
	uint8_t ret = 0x00;
	rtc_disable_updates();
	ret = rtc_read(RTC_REGISTER_MINUTES);
	rtc_enable_updates();
	return ret;

}
uint8_t rtc_get_hours(void)
{
	uint8_t ret = 0x00;
	rtc_disable_updates();
	ret = rtc_read(RTC_REGISTER_HOURS);
	rtc_enable_updates();
	return ret;

}
uint8_t rtc_get_day(void)
{
	uint8_t ret = 0x00;
	rtc_disable_updates();
	ret = rtc_read(RTC_REGISTER_DAY);
	rtc_enable_updates();
	return ret;

}
uint8_t rtc_get_month(void)
{
	uint8_t ret = 0x00;
	rtc_disable_updates();
	ret = rtc_read(RTC_REGISTER_MONTH);
	rtc_enable_updates();
	return ret;

}
uint8_t rtc_get_year(void)
{
	uint8_t ret = 0x00;
	rtc_disable_updates();
	ret = rtc_read(RTC_REGISTER_YEAR);
	rtc_enable_updates();
	return ret;

}


