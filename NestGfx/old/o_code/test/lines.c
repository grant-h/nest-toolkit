#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>

#include "../fb.h"

int main(void) {
	uint32_t x, y;
	uint32_t i;
	fb_init("/dev/fb0");

	/* clear fb */
	for(x = 0; x < 320; x++)
		for(y = 0; y < 320; y++)
			fb_draw_pixel(x, y, 0);
	
	for(x = 0; x < 320; x += 10) {
		i = 0;
		for(y = 0; y <= 320; y++)
			fb_draw_pixel(x, y, 0xffff0000);

		while(i < 1000000) i++;
	}

	for(y = 0; y < 320; y += 10) {
		i = 0;
		for(x = 0; x <= 320; x++)
			fb_draw_pixel(x, y, 0xff00ff00);

		while(i < 1000000) i++;
	}
	
	return EXIT_SUCCESS;
}
