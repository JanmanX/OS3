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

void *memcpy(void *dst, const void *src, size_t count)
{
	char* _dst = (char*)dst;
	char* _src = (char*)src;

	while (count--) {
		*_dst++ = *_src++;
	}
	return dst;
}

int memcmp(void *_s1, void *_s2, size_t n)
{
	uint8_t* s1 = (uint8_t*)_s1;
	uint8_t* s2 = (uint8_t*)_s2;


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

void*  memset(void *_s, int c, size_t n)
{
	uint8_t* s = _s;
	while(n--) {
		*s++ = c;
	}

	return _s;
}

/* String functions */
uint64_t strlen(const char *s)
{
	size_t len = 0x00;
	while(*s != (char)0x00) {
		s++;
		len++;
	}

	return len;
}

char* strcat(char* dst, const char *src)
{
	char *str = dst;

	/* Get to end of string */
	while(*str)
	{
		str++;
	}

	/* Concat */
	while(*str++ = *src++)
	{ ; }


	/* Return */
	return dst;
}

int strcmp(const char *s1, const char *s2)
{
	while(*s1 == *s2) {
		if(*s1 == (char)0) {
			return 0;
		}

		s1++;
		s2++;
	}
	return (unsigned char)*s1 - (unsigned char)*s2;

}

int strncmp(const char *s1, const char *s2, size_t n)
{
	while(*s1 == *s2 && n--) {
		if(*s1 == (char)0) {
			return 0;
		}

		s1++;
		s2++;
	}

	return (unsigned char)*s1 - (unsigned char)*s2;
}

char *strcpy(char *dst, const char *src)
{
	char *str = dst;

	while(*src) {
		*str++ = *src++;
	}

	*str = (char)0x00;

	return dst;
}

char *strncpy(char *dst, const char *src, size_t n)
{
	char *str = dst;

	while(*src && n--) {
		*str++ = *src++;
	}

	if(n) {
		*str = (char)0x00;
	}
	return dst;
}

int isupper(int c)
{
	return (c >= (int)'A' && c <= (int)'Z');
}

int toupper(int c)
{
	return islower(c) ? c-0x20 : c;
}

int islower(int c)
{
	return (c >= (int)'a' && c <= (int)'z');
}

int tolower(int c)
{
	return isupper(c)? c+0x20 : c;
}

int isprint(int c) {
	return c >= 0x20 && c <= 0x7E;
}

int isxdigit(int c)
{
	return (isdigit(c) || (c >= 0x61 && c <= 0x66));
}

int isdigit(int c)
{
	return c >= 0x30 && c <= 0x39;
}

int isspace(int c)
{
	return (c == (int)' '
		|| c == (int)'\f'
		|| c == (int)'\n'
		|| c == (int)'\r'
		|| c == (int)'\t'
		|| c == (int)'\v' );
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

inline uint16_t inw(uint16_t port)
{
	uint16_t ret;
	asm volatile ("inw %1, %0"
		      : "=a"(ret)
		      : "Nd"(port)
		     );
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

inline void outw(uint16_t port, uint16_t val)
{
	asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}

inline void outd(uint16_t port, uint32_t val)
{
	asm volatile ( "out %0, %1" : : "a"(val), "Nd"(port) );
}


/* CPU-speed independant.
 * From osdev.org */
void io_wait(void)
{
	/* Port 0x80 is used for 'checkpoints' during POST. */
	/* The Linux kernel seems to think it is free for use :-/ */
	asm volatile ( "outb %%al, $0x80" : : "a"(0) );
}

void _pause(void)
{
	asm volatile ( "pause" );
}


