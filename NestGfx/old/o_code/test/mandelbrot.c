#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "../fb.h"

#define MAX_ITER 100

int main(void) {
	uint32_t yres, xres, x, y;
	float X, Y, Xold, Yold, dx, dy, Cx, Cy;
	uint8_t iter;
	
	if(fb_init("/dev/fb0")) {
		fprintf(stderr, "can not initialize framebuffer.\n");
		return EXIT_FAILURE;
	}

	yres = fb_get_y_res();
	xres = fb_get_x_res();

	dy = ((float)4) / yres;
	dx = ((float)4) / xres;

	printf("got framebuffer of size %ux%u (%f, %f).\n", xres, yres, dx, dy);

	for(Cy = 0; Cy <= 2; Cy += dy) {
		for(Cx = -2; Cx <= 2; Cx += dx) {
			X = Cx;
			Y = Cy;
			for(iter = 0; iter < MAX_ITER; iter++) {
				Xold = X;
				Yold = Y;
				X = Xold * Xold - Yold * Yold + Cx;
				Y = Xold * Yold + Xold * Yold + Cy;
				if(X * X + Y * Y >= 4) break;
			}
			x = xres * Cx / 4 + 160;
			y = yres * Cy / 4 + 160;
			
			fb_draw_pixel(x, y,
					(iter == MAX_ITER) ? 0xffffffff : 0x00000000);
			fb_draw_pixel(x, yres - y,
					(iter == MAX_ITER) ? 0xffffffff : 0x00000000);
		}
	}

	return EXIT_SUCCESS;
}
