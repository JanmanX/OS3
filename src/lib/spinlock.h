#pragma once
#include <stdint.h>

typedef uint8_t spinlock_t;

void spinlock_acquire(spinlock_t* spinlock);
void spinlock_release(spinlock_t* spinlock);
