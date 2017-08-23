#ifndef KLIB_H
#define KLIB_H

#include <types.h>
#include <stdint.h>
#include <kprintf.h>

void itoa(char *buf, int base, int64_t n);

extern void kpanic();


void *memcpy(void *dst, void *src, uint64_t count);
uint8_t memcmp(const uint8_t *s1, const uint8_t *s2, uint64_t n);
void memset(uint8_t *s, uint8_t c, uint64_t n);


/* DEBUG FUNCTIONS
 * Should not be used in production */
#define BOCHS_DEBUG do {\
			asm volatile("xchg %bx, %bx");\
		} while(0);


#define HALT asm volatile("hlt")

#define LOG(m) kprintf("[LOG %s():%d]: %s\n", __func__, __LINE__, m)

#define DEBUG(m) kprintf("[DEBUG %s():%d]: %s\n", __func__, __LINE__, m)
#define ERROR(m) do {\
		kprintf("[ERROR %s():%d]: %s\n", __func__, __LINE__, m);\
		HALT;\
		} while(0);


#endif /* KLIB_H */
