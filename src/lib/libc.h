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

/* String functions */



/* Misc */
uint8_t checksum_zero(uint8_t *, uint64_t count);


/* Assembly  functions */
uint8_t inb(uint16_t port);
uint32_t ind(uint16_t port);

void outb(uint16_t port, uint8_t val);
void outd(uint16_t port, uint32_t val);

void io_wait(void);


/* Pointer helper macros */
/* Returns the appropriate pointer type to p + offet in bytes */
#define GET_UINT8(p, offset) (*((uint8_t*)p + offset))
#define GET_UINT16(p, offset) (*(uint16_t*)((uint8_t*)p + offset))
#define GET_UINT32(p, offset) (*(uint32_t*)((uint8_t*)p + offset))
#define GET_UINT64(p, offset) (*(uint64_t*)((uint8_t*)p + offset))



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
#define ASSERT(a, m) if(!a) {\
			kprintf("%s", m);\
			HALT;\
		}
#define ASSERTF(a, m, ...) if(!a) {\
			kprintf("[ASSERTION %s():%d]: ", __func__, __LINE__);\
			kprintf(m, __VA_ARGS__);\
		}


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
