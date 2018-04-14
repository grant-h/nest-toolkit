#ifndef FONTHANDLER_H__
#define FONTHANDLER_H__

#include <inttypes.h>

typedef struct textfield_ {
	char* caption;
	uint32_t x;
	uint32_t y;
	uint32_t height;
	uint32_t width;
	uint32_t color;
} textfield;

int ft_init(void);
int ft_print(textfield* t);

#endif
