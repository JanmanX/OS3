#include "libc.h"
#include <stdint.h>
#include <kprintf.h>


void itoa(char *buf, int base, int64_t n)
{
	char *p = buf;
	char *p1, *p2;
	uint64_t un = n;
	int div = 10;

	if(base == 'd' && n < 0) {
		*p++ = '-';
		un = -n;
		buf++;
	} else if (base == 'x') {
		div = 16;
	}

	do {
		/* calculate remainder */
		int r = un % div;

		*p++ = (r < 10) ? r + '0' : r + 'a' - 10;
	} while(un /= div);

	/* Terminate string */
	*p = 0;

	/* Reverse the string */
	p1 = buf;
	p2 = p - 1;


	while(p1 < p2) {
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;

	}
}

void *memcpy(void *dst, void *src, uint64_t count)
{
	char* _dst = (char*)dst;
	char* _src = (char*)src;

	while (count--) {
		*_dst++ = *_src++;
	}
	return dst;
}

uint8_t memcmp(const uint8_t *s1, const uint8_t *s2, uint64_t n)
{
	/* While bytes to compare */
	while(n--) {
		/* Compare and store result in diff */
		uint8_t diff = *s1++ - *s2++;

		/* If non-zero, return diff */
		if(diff != 0) {
			return diff;
		}
	}

	return 0;
}

void memset(uint8_t *s, uint8_t c, uint64_t n)
{
	while(n--) {
		*s++ = c;
	}
}

uint8_t checksum_zero(uint8_t *b, uint64_t count)
{
	uint8_t sum = 0;
	while(count--) {
		sum += *b++;
	}

	return sum;
}


inline uint8_t inb(uint16_t port)
{
	uint8_t ret;
	asm volatile ( "inb %1, %0"
		       : "=a"(ret)
		       :
		       "Nd"(port)
		     );
	return ret;
}

inline uint32_t ind(uint16_t port)
{
	uint32_t ret;
	asm volatile ( "in %1, %0"
		       : "=a"(ret)
		       :
		       "Nd"(port)
		     );
	return ret;
}



inline void outb(uint16_t port, uint8_t val)
{
	asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

inline void outd(uint16_t port, uint32_t val)
{
	asm volatile ( "out %0, %1" : : "a"(val), "Nd"(port) );
}

/* CPU-speed independant.
 * From osdev.org */
static inline void io_wait(void)
{
    /* Port 0x80 is used for 'checkpoints' during POST. */
    /* The Linux kernel seems to think it is free for use :-/ */
    asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}

void _pause(void)
{
	asm volatile ( "pause" );
}


