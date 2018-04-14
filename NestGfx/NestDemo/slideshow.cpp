#include <stdlib.h>
#include <unistd.h>
#include <memory.h>

#include <sys/time.h>
#include <sys/types.h>

#include "demos.h"
#include "util.h"
#include "slideshow/images.h"

#ifdef USE_IMAGES
struct image images[] = {
  {plum_width, plum_height, plum_data, NULL},
  {orchid_width, orchid_height, orchid_data, NULL},
  {bamboo_width, bamboo_height, bamboo_data, NULL},
  {chrysanthemum_width, chrysanthemum_height, chrysanthemum_data, NULL},
  {stone_width, stone_height, stone_data, NULL},
  {landscape_width, landscape_height, landscape_data, NULL},
  {dragon_width, dragon_height, dragon_data, NULL},
  {goldfish_width, goldfish_height, goldfish_data, NULL},
  {fan_width, fan_height, fan_data, NULL}
};

#define NUM_IMAGES 9
#else
struct image * images;
#endif

// transition functions
void transition_blocks(Gfx * fb, struct image * from, struct image * to, int speed);
void transition_fade(Gfx * fb, struct image * from, struct image * to, int speed);

void slideshow(Gfx * fb, int maxTime)
{
  Timer demoTime(maxTime*1000);
  demoTime.start();

  log_info("Slideshow");

#ifndef USE_IMAGES
  return;
#endif

  Timer ani(100);
  ani.start();

  // unpack all of the images in to memory
  for(int i = 0; i < NUM_IMAGES; i++)
  {
    unsigned int w = images[i].w;
    unsigned int h = images[i].h;

    images[i].pixels = new rgb_t*[h];

    if(!images[i].pixels)
    {
      log_error("Out of memory in slideshow (img %d)\n", i);
      die();
    }

    char * ptr = images[i].data;

    for(int y = 0; y < h; y++)
    {
      images[i].pixels[y] = new rgb_t[w];

      for(int x = 0; x < w; x++)
      {
        uint8_t pixels[3];
        rgb_t pixel;

        HEADER_PIXEL(ptr, pixels);
        pixel.r = pixels[0];
        pixel.g = pixels[1];
        pixel.b = pixels[2];

        images[i].pixels[y][x] = pixel;
      }
    }
  }

  int curImage = 0;
  int nextImage = 0;

  // render loop
  while(1)
  {
    rgb_t ** pixels = images[curImage].pixels;

    for(int y = 0; y < fb->getHeight(); y++)
      for(int x = 0; x < fb->getWidth(); x++)
        fb->drawPixel(x, y, pixels[y][x]);

    fb->fps();
    fb->flip();
    fb->drawClear(COLOR_BLACK);

    sleep(rand() % 3 + 1);

    nextImage = (curImage + 1) % NUM_IMAGES;

    if(rand() % 2)
      transition_blocks(fb, &images[curImage], &images[nextImage], 2000);
    else
      transition_fade(fb, &images[curImage], &images[nextImage], 2000);

    curImage = nextImage;

    if(demoTime.tick())
      break;
  }

  // free memory
  for(int i = 0; i < NUM_IMAGES; i++)
  {
    for(int y = 0; y < images[i].h; y++)
      delete [] images[i].pixels[y];

    delete [] images[i].pixels;
  }
}

void transition_blocks(Gfx * fb, struct image * from, struct image * to, int speed)
{
  int tilesPerRow = 4;
  int numTiles = tilesPerRow*tilesPerRow;
  int tileDim = fb->getWidth()/tilesPerRow;

  char * depth = new char[numTiles];
  bool * direction = new bool[numTiles];
  bool allDone = false;

  for(int tile = 0; tile < numTiles; tile++)
  {
    depth[tile] = 0;
    direction[tile] = false;
  }

  int sleepTime = speed*1000/(tileDim);

  while(!allDone)
  {
    allDone = true;

    for(int tile = 0; tile < numTiles; tile++)
    {
      int d = depth[tile];
      int tileW = tileDim - d;
      int tileCol = (tile % tilesPerRow)*tileDim;
      int tileRow = (tile/tilesPerRow)*tileDim;

      // map from the image to the screen
      for(int y = tileRow; y < tileRow + tileDim; y++)
      {
        if(!(y >= tileRow+d && y <= tileRow+tileDim-d))
          continue;

        for(int x = tileCol; x <  tileCol + tileDim; x++)
        {
          if(x >= tileCol+d && x <= tileCol+tileDim-d)
          {
            if(direction[tile])
              fb->drawPixel(x, y, to->pixels[y][x]);
            else
              fb->drawPixel(x, y, from->pixels[y][x]);
          }
        }
      }

      if(!direction[tile])
      {
        if(d < tileDim/2)
          depth[tile] = min(tileDim/2, depth[tile] + 1);
        else
          direction[tile] = true;
      }
      else
      {
        if(d > 0)
          depth[tile] = max(0, depth[tile] - 1);
      }

      if(!(direction[tile] && d == 0))
        allDone = false;

    }

    fb->fps();
    fb->flip();
    usleep(sleepTime);
    fb->drawClear(COLOR_BLACK);
  }

  delete [] depth;
  delete [] direction;
}

void transition_fade(Gfx * fb, struct image * from, struct image * to, int speed)
{
  int steps = 255;

  short int * delta = new short int[from->w*from->h*3];

  for(int j = 0; j < steps; j += 3)
  {
    for(int y = 0; y < fb->getHeight(); y++)
    {
      short int * dPtr = delta + y*fb->getWidth()*3;

      for(int x = 0; x < fb->getWidth(); x++)
      {
        if(j == 0)
        {
          dPtr[0] = to->pixels[y][x].r - from->pixels[y][x].r;
          dPtr[1] = to->pixels[y][x].g - from->pixels[y][x].g;
          dPtr[2] = to->pixels[y][x].b - from->pixels[y][x].b;
        }

        rgb_t pixel;

        pixel.r = from->pixels[y][x].r + dPtr[0]*j/steps;
        pixel.g = from->pixels[y][x].g + dPtr[1]*j/steps;
        pixel.b = from->pixels[y][x].b + dPtr[2]*j/steps;

        dPtr += 3;

        fb->drawPixel(x, y, pixel);
      }
    }

    fb->fps();
    fb->flip();
    fb->drawClear(COLOR_BLACK);
  }

  delete [] delta;
}
