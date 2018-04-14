#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <fbdraw.h>

#include "patched.h"

int main(int argc, char** argv) {
	size_t i;
	uint8_t rgb[3];
	uint32_t color;

	if(fb_init("/dev/fb0")) {
		fprintf(stderr, "can not open framebuffer device\n");
		return EXIT_FAILURE;
	}

	for(i = 0; i < height * width; i++) {
		HEADER_PIXEL(header_data, rgb);
		color = (rgb[0] << 16) | (rgb[1] << 8) | rgb[2];
		fb_draw_pixel(i % 320, i / 320, color);
		/* fb_draw_pixel(i / 320, i % 320, color); */
	}

	return EXIT_SUCCESS;
}

