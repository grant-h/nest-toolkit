#ifndef FB_H_
#define FB_H_

#include <inttypes.h>

int fb_init(const char* fbPath);
uint32_t fb_get_x_res(void);
uint32_t fb_get_y_res(void);
void fb_draw_pixel(uint32_t x, uint32_t y, uint32_t color);
void fb_clear(void);
void fb_end(void);

#endif
