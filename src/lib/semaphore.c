#include "semaphore.h"

#include <libc.h>
#include <mem/mem.h>
#include <stdint.h>
#include <kernel/time.h>

semaphore_t* semaphore_create(void)
{
	semaphore_t *sem = malloc(sizeof(semaphore_t));
	ASSERT(sem != NULL, "Could not allocate semaphore!");

	/* Init */
	sem->spinlock = 0;
	sem->n = 0;
	return sem;
}

void semaphore_signal(semaphore_t* sem)
{
	spinlock_acquire(&(sem->spinlock));
	sem->n++;
	spinlock_release(&(sem->spinlock));
}

void semaphore_wait(semaphore_t* sem)
{
	do {
		spinlock_acquire(&(sem->spinlock));
		if(sem->n > 0) {
			sem->n--;
			spinlock_release(&(sem->spinlock));
			return;
		}
		spinlock_release(&(sem->spinlock));
		sleep(5);
	} while(1);

}
