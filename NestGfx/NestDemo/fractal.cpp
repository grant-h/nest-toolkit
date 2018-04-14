#include <complex>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "demos.h"
#include "util.h"

struct fracPt
{
  int escape;
};

void demo_fractal(Gfx * fb, int maxTime)
{
  Timer demoTime(maxTime*1000);
  demoTime.start();

  rgb_t color = COLOR_RED;
  int maxIter = 20;
  double zoom = 0.5;

  // memoize the fractal
  struct fracPt * pts = new fracPt[fb->getHeight()*fb->getWidth()];

  if(!pts)
  {
    log_error("Could not allocate fractal memory!");
    return;
  }

  log_info("Demo fractal");

  std::complex<double> z, c;

  double sY = 2*zoom;
  double sX = -2.5*zoom;
  double eY = -2*zoom;
  double eX = 1.5*zoom;

  fb->drawDirty(true);
  
  for(double y = sY; y >= eY; y -= 0.01*zoom) {
    for(double x = sX; x <= eX; x += 0.01*zoom) {
      z = std::complex<double>(0, 0);
      c = std::complex<double>(x, y);
      int escapeIn = 0;
      for(int i = 0; i < maxIter; i++) {
        z = z*z + c;
        escapeIn++;
        if(std::abs(z) >= 2) break;
      }

      int screenY = min(fb->getHeight()-((int)(y*80.0/zoom) + 159), fb->getHeight()-1);
      int screenX = min((int)(x*80.0/zoom) + 200, fb->getHeight()-1);

      pts[screenY*fb->getWidth()+screenX].escape = escapeIn;

      if(std::abs(z) < 2)
      {
        fb->drawPixel(screenX, screenY, COLOR_BLACK);
      }
      else
      {
        rgb_t c;
        c.b = 255*((float)escapeIn/maxIter);
        c.r = 255 - c.b;
        c.g = 0;

        fb->drawPixel(screenX, screenY, c);
      }

      fb->fps();
      if(demoTime.tick())
      {
        delete pts;
        return;
      }
    }
  }

  int ticks = 0;
  int colorIter = 0;

  Timer ani(20);
  ani.start();

  rgb_t colors[3] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE};
  int NUMCOLS = sizeof(colors)/sizeof(colors[0]);

  rgb_t color1 = colors[colorIter];
  rgb_t color2 = colors[(colorIter + 1) % NUMCOLS];

  fb->drawDirty(true);

  while(1) 
  {
    if(demoTime.tick())
    {
      delete pts;
      return;
    }

    if(ani.tick()) {

      ticks++;

      if(ticks > 100)
      {
        ticks = 0;

        // move on to the next color
        colorIter++;
        color1 = color2;
        color2 = colors[colorIter % NUMCOLS];
      }

      ani.restart();
    }

    int iter = 0;
    struct fracPt * pt = pts;
    int h = fb->getHeight();
    int w = fb->getWidth();
    rgb_t lerped = fb->lerp(color1, color2, (float)ticks/100);

    for(int y = 0; y < h; y++) {
      for(int x = 0; x < w; x++) {

        if(pt->escape < maxIter) {
          rgb_t finalColor = lerped;

          // gate the colors based on escape time
          finalColor.r = finalColor.r*pt->escape/maxIter;
          finalColor.g = finalColor.g*pt->escape/maxIter;
          finalColor.b = finalColor.b*pt->escape/maxIter;

          fb->drawPixel(x, y, finalColor);
        }
        else
          fb->drawPixel(x, y, COLOR_BLACK);

        pt++;
      }
    }

    fb->fps();
    // dont clear because we redraw the whole screen
  }
}
