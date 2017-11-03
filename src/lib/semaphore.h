#pragma once
#include <stdint.h>
#include <lib/spinlock.h>

typedef struct semaphore {
	uint64_t n;
	spinlock_t spinlock;
} semaphore_t;

semaphore_t* semaphore_create(void);
void semaphore_signal(semaphore_t* s);
void semaphore_wait(semaphore_t* s);
