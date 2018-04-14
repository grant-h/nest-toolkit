#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>
#include <unistd.h>

#include "../fb.h"

int main(int argc, char** argv) {
	uint32_t xres, yres, x, y;
	if(argc == 1) return EXIT_FAILURE;

	assert(fb_init(argv[1]) == 0);
	printf("got framebuffer of size %ux%u\n", xres = fb_get_x_res(), yres = fb_get_y_res());
	for(x = 0; x < xres; x++)
		for(y = 0; y < yres; y++)
			if(((x + y) & 0x3) == 0x03)
				fb_draw_pixel(x, y, 0xffffffff);
			else
				fb_draw_pixel(x, y, 0x00000000);
	
	sleep(10);
	fb_end();

	return EXIT_SUCCESS;
}
