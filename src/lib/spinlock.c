#include "spinlock.h"
#include <stdint.h>



void spinlock_acquire(spinlock_t* spinlock)
{
	/*
.retry:
    lock bts [lock],0		Read bit 0 from [lock] into CF
    jc .retry			If CF set, retry
	*/

	__asm__ ("1: lock bts $0, %[s]; jc 1b;"
		 : [s] "+m"(*spinlock)
		 :
		 : "memory"
		 );
}


void spinlock_release(spinlock_t* spinlock)
{
	asm volatile ("lock btr $0, %[s]"
		      : [s] "+m"(*spinlock)
		      :
		      : "memory");
}

