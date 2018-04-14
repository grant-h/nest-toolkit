#ifndef SLIDESHOW_IMAGE_H
#define SLIDESHOW_IMAGE_H

#define HEADER_PIXEL(data,pixel) {\
pixel[0] = (((data[0] - 33) << 2) | ((data[1] - 33) >> 4)); \
pixel[1] = ((((data[1] - 33) & 0xF) << 4) | ((data[2] - 33) >> 2)); \
pixel[2] = ((((data[2] - 33) & 0x3) << 6) | ((data[3] - 33))); \
data += 4; \
}

struct image
{
  unsigned int w, h;
  char * data;
  rgb_t ** pixels;
};

#define USE_IMAGES

#ifdef USE_IMAGES
#include "../slideshow/goldfish.h"
#include "../slideshow/landscape.h"
#include "../slideshow/bamboo.h"
#include "../slideshow/chrysanthemum.h"
#include "../slideshow/dragon.h"
#include "../slideshow/fan.h"
#include "../slideshow/orchid.h"
#include "../slideshow/plum.h"
#include "../slideshow/stone.h"
#else
static char * goldfish_data = "";
static char * landscape_data = "";
static char * bamboo_data = "";
static char * chrysanthemum_data = "";
static char * dragon_data = "";
static char * fan_data = "";
static char * orchid_data = "";
static char * plum_data = "";
static char * stone_data = "";
#endif

#endif
