#include "Gfx.h"
#include "util.h"

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

Gfx::Gfx(const char * fbPath)
{
  init(fbPath);
}

Gfx::~Gfx()
{
  drawClear(COLOR_BLACK);
  flip();
  detach();
}

bool Gfx::attach()
{
  if(m_attached)
    return true;

  int fd = open(m_fbPath, O_RDWR);

  if(fd < 0)
  {
    log_errno("Failed to attach to %s", m_fbPath);
    return false;
  }

  m_fbFd = fd; // hesitantly commit for now

  if(getVarScreenInfo() < 0)
  {
    log_errno("Failed to grab variable screen info on %s", m_fbPath);
    detach();

    return false;
  }

  if(getFixScreenInfo() < 0)
  {
    log_errno("Failed to grab fixed screen info on %s", m_fbPath);
    detach();

    return false;
  }

  // get and store the framebuffer size
  m_fbSize = m_fix.smem_len;

  void * fbMem = mmap(NULL, m_fbSize,
                      PROT_WRITE | PROT_READ,
                      MAP_SHARED, fd, 0);

  if(fbMem == MAP_FAILED)
  {
    log_errno("Failed to mmap framebuffer on %s", m_fbPath);
    detach();

    return false;
  }

  // everything went okay, commit
  m_mem = reinterpret_cast<uint32_t *>(fbMem);
  m_attached = true;

  log_debug("Got framebuffer at 0x%08x of size %u KiB", m_mem, m_fbSize/1024);

  // check for a single buffered display
  if(getHeight() == getVirtHeight())
  {
    m_doubleBuffered = false;
    m_bufYOffset = 0;
    m_frontBuf = m_backBuf = m_mem;

    log_info("WARNING: Got single buffered display of %dx%d. "
        "Attempting to get a double buffered one...", getWidth(), getHeight());

    if(setDoubleBuffered() < 0)
      log_error("Failed to get a double buffered display. Graphics performance "
          "will suffer");
  }

  if(getHeight()*2 >= getVirtHeight())
  {
    m_doubleBuffered = true;

    // making assumptions...
    m_bufYOffset = getHeight();
    m_frontBuf = m_mem;
    m_backBuf = m_frontBuf + m_bufYOffset*getWidth();
    
    // make sure we're panned to the front buffer (visible)
    if(panDisplay(0) < 0)
    {
      log_errno("Display %s doesn't seem to support panning", m_fbPath);

      detach();
      return false;
    }

    log_info("Got double buffered display of %dx%d", getWidth(), getHeight());
  }
  else
  {
    log_errno("Strange virtual screen height %u versus real height %u on %s",
        getHeight(), getVirtHeight(), m_fbPath);

    detach();
    return false;
  }

  return true;
}

void Gfx::detach()
{
  if(m_fbFd)
  {
    close(m_fbFd);
    m_fbFd = 0;
  }

  if(m_mem)
  {
    munmap(m_mem, m_fbSize);
    m_mem = NULL;
  }

  // clean slate
  init(m_fbPath);
}

int Gfx::getWidth()
{
  if(!m_attached)
    return 0;

  return m_var.xres;
}

int Gfx::getHeight()
{
  if(!m_attached)
    return 0;

  return m_var.yres;
}

int Gfx::getVirtWidth()
{
  if(!m_attached)
    return 0;

  return m_var.xres_virtual;
}

int Gfx::getVirtHeight()
{
  if(!m_attached)
    return 0;

  return m_var.yres_virtual;
}

void Gfx::drawClear(rgb_t color)
{
  if(!m_attached)
    return;

  uint32_t c = rgb2color(color);
  uint32_t * ptr = m_backBuf;
  uint32_t * ptrEnd = ptr + getHeight()*getWidth();

  while(ptr < ptrEnd)
    *ptr++ = c;
}

void Gfx::fillRect(int tlX, int tlY, int brX, int brY, rgb_t color)
{
  int tmp;

  if(tlX > brX)
  {
    tmp = tlX;
    tlX = brX;
    brX = tmp;
  }

  if(tlY > brY)
  {
    tmp = tlY;
    tlY = brY;
    brY = tmp;
  }

  // prevent overdraw
  tlX = min(max(tlX, 0), getWidth()-1);
  brX = min(max(brX, 0), getWidth()-1);
  tlY = min(max(tlY, 0), getHeight()-1);
  brY = min(max(brY, 0), getHeight()-1);

  uint32_t c = rgb2color(color);
  uint32_t * ptr = m_backBuf + tlY*getWidth() + tlX;

  for(int y = tlY; y < brY; y++)
  {
    uint32_t * line = ptr;

    for(int x = tlX; x < brX; x++)
      *line++ = c;

    // move down one line
    ptr += getWidth();
  }
}

void Gfx::drawPixel(int x, int y, rgb_t color)
{
  if(!m_attached)
    return;

  // perform some basic sanity checking
  if(x < 0 || x >= getWidth())
    return;

  if(y < 0 || y >= getHeight())
    return;

  // FIXME: assuming a 32 bpp screen
  uint32_t * ptr = m_backBuf + y*getWidth() + x;
  uint32_t c = rgb2color(color);

  *ptr = c;

  return;
}

void Gfx::drawPixelRaw(int x, int y, uint32_t color)
{
  rgb_t rgb = color2rgb(color);

  drawPixel(x, y, rgb);
}

void Gfx::drawLine(int x1, int y1, int x2, int y2, rgb_t color)
{
  int dx =  abs(x2-x1), sx = x1<x2 ? 1 : -1;
  int dy = -abs(y2-y1), sy = y1<y2 ? 1 : -1; 
  int err = dx+dy, e2; /* error value e_xy */

  // stolen from http://members.chello.at/~easyfilter/bresenham.html
  for(;;){  /* loop */
    drawPixel(x1,y1, color);
    if (x1==x2 && y1==y2) break;
    e2 = 2*err;
    if (e2 >= dy) { err += dy; x1 += sx; } /* e_xy+e_x > 0 */
    if (e2 <= dx) { err += dx; y1 += sy; } /* e_xy+e_y < 0 */
  }

  return;
}

void Gfx::drawLineGrad(int x1, int y1, int x2, int y2, rgb_t from, rgb_t to)
{
  int dx =  abs(x2-x1), sx = x1<x2 ? 1 : -1;
  int dy = -abs(y2-y1), sy = y1<y2 ? 1 : -1; 
  int err = dx+dy, e2; /* error value e_xy */
  // radient handling
  int sliderMax = max(-dy,dx);
  int slider = 0;
  bool xMax = dx > -dy && dx != 0;

  // stolen from http://members.chello.at/~easyfilter/bresenham.html
  // modified to do gradients
  for(;;) { 
    drawPixel(x1,y1, lerpRange(from, to, slider, sliderMax));
    if (x1==x2 && y1==y2) break;
    e2 = 2*err;
    if (e2 >= dy) 
    { 
      err += dy; 
      x1 += sx; 
      if(xMax)
        slider++;
    } /* e_xy+e_x > 0 */

    if (e2 <= dx) 
    { 
      err += dx;
      y1 += sy; 

      if(!xMax)
        slider++;
    } /* e_xy+e_y < 0 */
  }

  return;
}

void Gfx::drawDirty(bool set)
{
  if(set != m_drawDirty)
  {
    m_drawDirty = set;
    flip();
  }
}

void Gfx::flip()
{
  if(!m_doubleBuffered || !m_attached)
    return;

  if(m_drawDirty)
  {
    // present the back buffer (always)
    if(m_backBuf == m_mem)
      panDisplay(0);
    else
      panDisplay(m_bufYOffset);

    return;
  }

  // swap buffers
  uint32_t * tmp = m_frontBuf;
  m_frontBuf = m_backBuf;
  m_backBuf = tmp;

  // present the new front buffer
  if(m_frontBuf == m_mem)
    panDisplay(0);
  else
    panDisplay(m_bufYOffset);
}

rgb_t Gfx::randColor()
{
  uint32_t rC = rand();
  rgb_t c = color2rgb(rC);

  return c;
}

rgb_t Gfx::lerp(rgb_t from, rgb_t to, float slider)
{
  rgb_t newColor = COLOR_BLACK;

  if(slider < 0.0)
    return from;
  if(slider > 1.0)
    return to;

  int dr = (int)to.r - (int)from.r;
  int dg = (int)to.g - (int)from.g;
  int db = (int)to.b - (int)from.b;

  newColor.r = (char)from.r + (int)(dr*slider);
  newColor.g = (char)from.g + (int)(dg*slider);
  newColor.b = (char)from.b + (int)(db*slider);

  return newColor;
}

rgb_t Gfx::lerpRange(rgb_t from, rgb_t to, int slider, int sliderMax)
{
  rgb_t newColor = COLOR_BLACK;

  if(slider <= 0)
    return from;
  if(slider >= sliderMax)
    return to;

  int dr = (int)to.r - (int)from.r;
  int dg = (int)to.g - (int)from.g;
  int db = (int)to.b - (int)from.b;

  newColor.r = (char)from.r + (int)(dr*slider/sliderMax);
  newColor.g = (char)from.g + (int)(dg*slider/sliderMax);
  newColor.b = (char)from.b + (int)(db*slider/sliderMax);

  return newColor;
}

void Gfx::printInfo()
{
  log_error("%s: stub", __FUNCTION__);
}

void Gfx::fps()
{
  if(!m_fpsTimer.isStarted())
  {
    m_fpsTimer.start();
    return;
  }

  m_fpsTicks++;

  if(m_fpsTimer.tick())
  {
    log_info("FPS %d", m_fpsTicks);
    m_fpsTimer.restart();
    m_fpsTicks = 0;
  }
}

/* Private Methods
 */

int Gfx::getVarScreenInfo()
{
  return ioctl(m_fbFd, FBIOGET_VSCREENINFO, &m_var); 
}

int Gfx::setDoubleBuffered()
{
  if(getVarScreenInfo())
    return -1;

  m_var.yres_virtual = m_var.yres*2;
  
  if(ioctl(m_fbFd, FBIOPUT_VSCREENINFO, &m_var) < 0)
    return -1;

  if(getVarScreenInfo())
    return -1;

  if(m_var.yres_virtual != m_var.yres*2)
    return -1;
  else
    return 0;
}

int Gfx::getFixScreenInfo()
{
  return ioctl(m_fbFd, FBIOGET_FSCREENINFO, &m_fix); 
}

uint32_t Gfx::rgb2color(rgb_t rgb)
{
  return rgb2color(rgb.r, rgb.g, rgb.b);
}

uint32_t Gfx::rgb2color(uint8_t r, uint8_t g, uint8_t b)
{
  // assumed to be little endian
  return (r << 16) | (g << 8) | b;
}

rgb_t Gfx::color2rgb(uint32_t color)
{
  rgb_t rgb;

  rgb.r = color >> 16 & 0xff;
  rgb.g = color >> 8 & 0xff;
  rgb.b = color & 0xff;

  return rgb;
}

int Gfx::panDisplay(uint32_t yOffset)
{
  m_var.yoffset = yOffset;
  int retVal = ioctl(m_fbFd, FBIOPAN_DISPLAY, &m_var); 

  if(retVal < 0)
  {
    log_error("Failed to pan the display");
    return retVal;
  }

  // OMAPFB_WAITFORGO: found using strace on nlclient
  // See here: http://lxr.free-electrons.com/source/include/linux/omapfb.h?v=2.6.39#L57
  retVal = ioctl(m_fbFd, 0x4f3c, 0);

  if(retVal < 0)
    log_error("Failed to wait for the display to be ready");

  return retVal;
}

void Gfx::init(const char * fbPath)
{
  m_fbPath = fbPath;
  m_fbFd = 0;

  m_attached = false;
  clear_obj(&m_var);
  clear_obj(&m_fix);

  m_fbSize = 0;
  m_mem = NULL;
  m_drawDirty = false;
  m_frontBuf = NULL;
  m_backBuf = NULL;

  m_doubleBuffered = false;
  m_bufYOffset = 0;

  m_fpsTimer = Timer(1000);
  m_fpsTicks = 0;
}
