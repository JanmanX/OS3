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

uint8_t checksum_zero(uint8_t *, uint64_t count);


#define LOG(m) kprintf("[LOG %s():%d]: %s\n", __func__, __LINE__, m);\

#define DEBUG(m) kprintf("[DEBUG %s():%d]: %s\n", __func__, __LINE__, m);\

#define ERROR(m) do {\
		kprintf("[ERROR %s():%d]: %s\n", __func__, __LINE__, m);\
		HALT;\
		} while(0);

#define LOGF(m, ...) do {\
		kprintf("[LOG %s():%d]: ", __func__, __LINE__);\
		kprintf(m, __VA_ARGS__);\
		} while(0);

#define DEBUGF(m, ...) do {\
		kprintf("[DEBUG %s():%d]: ", __func__, __LINE__);\
		kprintf(m, __VA_ARGS__);\
		} while(0);

#define ERRORF(m, ...) do {\
		kprintf("[ERROR %s():%d]: ", __func__, __LINE__);\
		kprintf(m, __VA_ARGS__);\
		HALT;\
		} while(0);

/* Assembly instructions */
#define CLI asm volatile ("cli")
#define STI asm volatile ("sti")
#define HALT asm volatile("hlt")

/* DEBUG FUNCTIONS
* Should not be used in production */
#define BOCHS_DEBUG do {\
			asm volatile("xchg %bx, %bx");\
	       } while(0);



#endif /* KLIB_H */
