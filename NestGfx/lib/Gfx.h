#ifndef GFX_H
#define GFX_H

#include <linux/fb.h>

#include "common.h"
#include "Timer.h"

// a nice generic color structure
struct rgb_t { unsigned char r, g, b; };
struct point3 { int x, y, z; };
struct point2 { int x, y; };

static const rgb_t COLOR_BLACK = {0, 0, 0};
static const rgb_t COLOR_WHITE = {255, 255, 255};
static const rgb_t COLOR_RED = {255, 0, 0};
static const rgb_t COLOR_GREEN = {0, 255, 0};
static const rgb_t COLOR_BLUE = {0, 0, 255};

class Gfx
{
public:
  Gfx(const char * fbPath);
  ~Gfx();

  // screen resource management
  bool attach();
  void detach();

  // screen information
  int getWidth();
  int getHeight();
  int getVirtWidth();
  int getVirtHeight();

  // drawing
  void drawClear(rgb_t color);
  void fillRect(int tlX, int tlY, int brX, int brY, rgb_t color);
  void drawPixel(int x, int y, rgb_t color);
  void drawPixelRaw(int x, int y, unsigned color);
  void drawLine(int x1, int y1, int x2, int y2, rgb_t color);
  void drawLineGrad(int x1, int y1, int x2, int y2, rgb_t from, rgb_t to);
  void drawDirty(bool set);

  // flips the screen buffers, if we have more than one
  void flip();

  // misc coloring
  rgb_t randColor();
  // linear interpolation
  rgb_t lerp(rgb_t from, rgb_t to, float slider);
  // non-floating point version
  rgb_t lerpRange(rgb_t from, rgb_t to, int slider, int sliderMax);

  // display information
  void printInfo();
  void fps();
private:
  int getVarScreenInfo();
  int setDoubleBuffered();
  int getFixScreenInfo();
  // color packing/unpacking (TODO: public?)
  uint32_t rgb2color(rgb_t rgb);
  uint32_t rgb2color(uint8_t r, uint8_t g, uint8_t b);
  rgb_t color2rgb(uint32_t color);
  int panDisplay(uint32_t yOffset);
  void init(const char * fbPath);

  const char * m_fbPath;
  int m_fbFd;

  bool m_attached;

  struct fb_var_screeninfo m_var;
  struct fb_fix_screeninfo m_fix;
  uint32_t m_fbSize;

  // drawing should never use this directly
  uint32_t * m_mem;
  bool m_drawDirty; // draw directly to the front buffer

  // framebuffer pointers
  uint32_t * m_frontBuf;
  uint32_t * m_backBuf;

  bool m_doubleBuffered;
  unsigned m_bufYOffset; // offset to the other buffer

  // fps counter
  Timer m_fpsTimer;
  int m_fpsTicks;
};

#endif
