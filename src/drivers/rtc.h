#pragma once

#include <stdint.h>

#define RTC_ADDRESS_PORT		(0x70)
#define RTC_DATA_PORT			(0x71)

#define RTC_REGISTER_SECONDS		(0x00)
#define RTC_REGISTER_MINUTES		(0x02)
#define RTC_REGISTER_HOURS		(0x04)
#define RTC_REGISTER_DAY		(0x07)
#define RTC_REGISTER_MONTH		(0x08)
#define RTC_REGISTER_YEAR		(0x09)
#define RTC_REGISTER_A			(0x0A)
#define RTC_REGISTER_B			(0x0B)
#define RTC_REGISTER_C			(0x0C)
#define RTC_REGISTER_D			(0x0D)









void rtc_init(void);

uint8_t rtc_read(const uint8_t reg);
uint8_t rtc_write(const uint8_t reg, const uint8_t val);
uint8_t rtc_get_seconds(void);
uint8_t rtc_get_minutes(void);
uint8_t rtc_get_hours(void);
uint8_t rtc_get_day(void);
uint8_t rtc_get_month(void);
uint8_t rtc_get_year(void);



