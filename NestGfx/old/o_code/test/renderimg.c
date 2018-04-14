#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "../fb.h"

struct rgba {
	uint8_t alpha;
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

int main(int argc, char** argv) {
	FILE* img;
	uint32_t x, y;
	uint32_t color;

	if(argc != 2) {
		fprintf(stderr, "usage: %s <rawimg>\n", argv[0]);
		return EXIT_FAILURE;
	}

	if(fb_init("/dev/fb0")) {
		fprintf(stderr, "unable to acquire framebuffer\n");
		return EXIT_FAILURE;
	}

	if(!(img = fopen(argv[1], "r"))) {
		fprintf(stderr, "unable to open file %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	for(y = 0; y < 320; y++) {
		for(x = 0; x < 320; x++) {
			if(fread(&color, sizeof(color), 1, img) < 1) {
				perror("fread");
			}
			/* wizardry */
			fb_draw_pixel(x, 319 - y, color >> 8);
		}
	}

	return EXIT_SUCCESS;
}	
