#pragma once

#include <types.h>
#include <stdint.h>
#include <kprintf.h>

void itoa(char *buf, int base, int64_t n);

extern void kpanic();


void *memcpy(void *dst, const void *src, size_t count);
int memcmp(void *s1, void *s2, size_t n);
void* memset(void *s, int c, size_t n);

/* String functions */
size_t strlen(const char *s);
char* strcat(char* dst, const char *src);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, size_t n);

int toupper(int c);
int tolower(int c);
int isupper(int c);
int islower(int c);
int isprint(int c);
int isxdigit(int c);
int isdigit(int c);

/* Misc */
uint8_t checksum_zero(uint8_t *, uint64_t count);


/* Assembly  functions */
uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t ind(uint16_t port);

void outb(uint16_t port, uint8_t val);
void outw(uint16_t port, uint16_t val);
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
		while(1) {\
			HALT;\
		}\
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
			while(1) {HALT;}\
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


