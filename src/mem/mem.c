#include "mem.h"
#include <mem/paging.h>
#include <stdint.h>
#include <libc.h>


void mem_init()
{
	paging_init();
}
