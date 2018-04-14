#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <math.h>
#include <unistd.h>

#include "../fb.h"

#define MAX_ITER 50

int main(void) {
	uint32_t yres, xres, x, y, R, G, B;
	float X, Y, Xold, Yold, dx, dy, Cx, Cy, Zx, Zy, t;
	uint8_t iter;
	
	if(fb_init("/dev/fb0")) {
		fprintf(stderr, "can not initialize framebuffer.\n");
		return EXIT_FAILURE;
	}

	yres = fb_get_y_res();
	xres = fb_get_x_res();

	dy = ((float)4) / yres;
	dx = ((float)4) / xres;
	
	for(t = 0; t < 18 * M_PI; t += M_PI / 100) {
		/*Cx = t*cos(t/8)/(9 * M_PI);
		Cy = t*sin(t/2)/(9 * M_PI);*/
		/*Cx = cos(t/4)*(1 + cos(t));
		Cy = 2*cos(2*t);*/
		Cx = -2 *exp(t/(18*M_PI)) * cos(2*t)/M_E;
		Cy = exp(t/(18*M_PI)) * sin(3*t)/M_E;

		for(Zy = 0; Zy <= 2; Zy += dy) {
			for(Zx = -2; Zx <= 2; Zx += dx) {
				X = Zx;
				Y = Zy;
				G = B = 0;
				R = 0xff;
				for(iter = 0; iter < MAX_ITER; iter++) {
					Xold = X;
					Yold = Y;
					X = Xold * Xold - Yold * Yold + Cx;
					Y = Xold * Yold + Xold * Yold + Cy;
					R -= 0x0f;
					G += 0x0f;
					if(X * X + Y * Y >= 4) break;
				}
				x = xres * Zx / 4 + 160;
				y = yres * Zy / 4 + 160;
				
				fb_draw_pixel(x, y,
						(iter == MAX_ITER) ? 0xff000000 : 0xff000000
							+ ((R & 0xff) << 16)
							+ ((G & 0xff) << 8)
							+ ((B & 0xff)));
				fb_draw_pixel(xres -x, yres - y,
						(iter == MAX_ITER) ? 0xff000000 : 0xff000000
							+ ((R & 0xff) << 16)
							+ ((G & 0xff) << 8)
							+ ((B & 0xff)));
						/* (iter == MAX_ITER) ? 0xffffffff : 0x00000000);*/
			}
		}
		usleep(100);
	}

	return EXIT_SUCCESS;
}
