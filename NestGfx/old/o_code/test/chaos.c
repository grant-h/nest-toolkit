#include <stdlib.h>
#include <inttypes.h>

#include "../fb.h"

#define MAX_ITER 10000

int main(void) {
	int i;
	uint32_t x, y, color;

	fb_init("/dev/fb0");

	for(x = 0; x < 320; x++)
		for(y = 0; y < 320; y++)
			fb_draw_pixel(x, y, 0);

	x = rand() % 320;
	y = rand() % 320;


	for(i = 0; i < MAX_ITER; i++) {
		switch(rand() % 6) {
			case 0:
			case 1:
				x = x/2;
				y = y/2;
				color = 0xffff0000;
				break;
			case 2:
			case 3:
				x = (320 + x)/2;
				y = y/2;
				color = 0xff00ff00;
				break;
			default:
				x = (160 + x)/2;
				y = (320 + y)/2;
				color = 0xff0000ff;
		}
		fb_draw_pixel(x, y, color);
	}

	return EXIT_SUCCESS;
}

