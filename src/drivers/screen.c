#include "screen.h"
#include <stdint.h>
#include <multiboot2.h>


uint8_t screen_init(struct multiboot_tag_framebuffer* mb_fb)
{


	uint32_t *a = (uint32_t*)0xA0000;

a[0] = 0xFF00FF00;
a[1] = 0xFF00FF00;
a[2] = 0xFF00FF00;
a[3] = 0xFF00FF00;
a[4] = 0xFFFFFFFF;
a[5] = 0xFFFF0000;
a[6] = 0xFFFF0000;




	if(mb_fb->common.framebuffer_type != MULTIBOOT_FRAMEBUFFER_TYPE_RGB) {
		/* TODO: Log and fail*/
		return 1;
	}

	width = mb_fb->common.framebuffer_width;
	height = mb_fb->common.framebuffer_height;
	bpp = mb_fb->common.framebuffer_bpp;
	buffer = (uint32_t*)(uint32_t)mb_fb->common.framebuffer_addr;


	if(buffer == a) {
a[10] = 0xFF00FF00;
a[11] = 0xFF00FF00;
a[12] = 0xFF00FF00;
a[13] = 0xFF00FF00;
a[14] = 0xFFFFFFFF;
a[15] = 0xFFFF0000;
a[16] = 0xFFFF0000;
	}





	put_pixel(0,0, 0xFF00FF00);
put_pixel(1,1, 0xFFFFFF00);
put_pixel(1,0, 0xFFFFFF00);
put_pixel(0,1, 0xFFFFFF00);
put_pixel(100,0, 0xFF00FF00);
put_pixel(100,1, 0xFFFFFF00);


}


static void put_pixel(uint32_t x, uint32_t y, uint32_t argb)
{
	uint32_t location = x + (y * width);

	/* Check overflow */
	if(location > width * height)  {
		/* TODO: LOG */
		return;
	}

	//buffer[location] = argb;
}

