#include <complex>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>

#include "demos.h"
#include "util.h"

int map3d(Gfx * fb, int y)
{
  // map a normal y value to a 3d-like plane
  int startY = fb->getHeight()/2 + 10;
  int endY = fb->getHeight()-1;
  float percent = (float)y/fb->getHeight();

  // non-linearlly map the y to the plane
  // to simulate 3D distance

  //map(y, 0, endY

  return startY + endY*expf(percent);
}

struct line
{
  float y;
  float x;
  float dy;
  float dx;
  float startY;
  float startX;
  rgb_t color;
};

void demo_grid(Gfx * fb, int maxTime)
{
  Timer demoTime(maxTime*1000);
  demoTime.start();

  log_info("Demo grid");

  int skipAmt = 20;
  int numLines = (fb->getHeight() - fb->getHeight()/2 + 20)/skipAmt - 2;
  struct line * lines = new line[numLines];

  for(int i = 0; i < numLines; i++)
  {
    lines[i].startY = fb->getHeight()/2 + skipAmt;
    lines[i].y = lines[i].startY + skipAmt*i;
    lines[i].dy = 1.0 - (float)(fb->getHeight() - lines[i].y)
      /(fb->getHeight() - lines[i].y);
    lines[i].color = fb->randColor();
  }

  Timer ani(50);
  ani.start();

  while(1)
  {
    if(demoTime.tick())
    {
      delete lines;
      return;
    }

    //////////////////

    if(ani.tick())
    {
      // update pos
      for(int i = 0; i < numLines; i++)
      {
        lines[i].dy = 1.0 -(float)(fb->getHeight() - lines[i].y)
          /(fb->getHeight() -lines[i].startY);
        lines[i].y += 2.0*exp(lines[i].dy); 

        if(lines[i].y > fb->getHeight())
        {
          lines[i].y = lines[i].startY;
          lines[i].color = fb->randColor();
        }
      }

      ani.restart();
    }

    // draw horizontal lines towards the distance
    for(int i = 0; i < numLines; i++)
    {
      struct line * l = &lines[i];
      rgb_t color = l->color;
      rgb_t foldColor = {0, 0, 255};

      l->dy = 1.0 -(float)(fb->getHeight() - l->y)
        /(fb->getHeight() -l->startY);
      color.r *= max(l->dy, 0.01);
      color.g *= max(l->dy, 0.01);
      color.b *= max(l->dy, 0.01);

      int widthFar = skipAmt*2;
      int heightFar = skipAmt*2;

      int tbLeftCorner = fb->getWidth()/2 - widthFar/2;
      int tbRightCorner = fb->getWidth()/2 + widthFar/2;
      int lrLeftCorner = fb->getHeight()/2 - heightFar/2;
      int lrRightCorner = fb->getHeight()/2 + heightFar/2;
      int tbWidth = tbLeftCorner;
      int lrHeight = lrLeftCorner;

      ///// draw grids
      // left
      fb->drawLine(fb->getWidth()-l->y, lrRightCorner+lrHeight*l->dy, fb->getWidth()-l->y, lrLeftCorner-lrHeight*l->dy, color);
      // right
      fb->drawLine(l->y, lrRightCorner+lrHeight*l->dy, l->y, lrLeftCorner-lrHeight*l->dy, color);
      // bottom
      fb->drawLine(tbLeftCorner-tbWidth*l->dy, l->y, tbRightCorner+tbWidth*l->dy, l->y, color);
      //top
      fb->drawLine(tbLeftCorner-tbWidth*l->dy, fb->getHeight()-l->y, 
        tbRightCorner+tbWidth*l->dy, fb->getHeight() - l->y, color);

      //////// draw folds
      // bottom left
      fb->drawLineGrad(tbLeftCorner, lrRightCorner,
          tbLeftCorner-tbWidth, fb->getHeight()-1, COLOR_BLACK, foldColor); 
      // bottom right
      fb->drawLineGrad(tbRightCorner, lrRightCorner,
          tbRightCorner+tbWidth, fb->getHeight()-1, COLOR_BLACK, foldColor); 
      // top left
      fb->drawLineGrad(tbLeftCorner, lrLeftCorner, 
          tbLeftCorner-tbWidth, 0, COLOR_BLACK, foldColor);
      // top right
      fb->drawLineGrad(tbRightCorner, lrLeftCorner,
          tbRightCorner+tbWidth, 0, COLOR_BLACK, foldColor);

      //////// draw box
      // bottom
      /*fb->drawLine(tbLeftCorner, lrRightCorner, tbRightCorner, lrRightCorner, foldColor); 
      // top
      fb->drawLine(tbLeftCorner, lrLeftCorner, tbRightCorner, lrLeftCorner, foldColor); 
      // left
      fb->drawLine(tbLeftCorner, lrLeftCorner, tbLeftCorner, lrRightCorner, foldColor); 
      // right
      fb->drawLine(tbRightCorner, lrLeftCorner, tbRightCorner, lrRightCorner, foldColor); */
    }

    fb->fps();
    fb->flip();
    fb->drawClear(COLOR_BLACK);
  }
}
