#include <libc.h>


int main(void* mb_info)
{

	kprintf("Hello world!");





	asm __volatile__("hlt");
	return 0;
}
