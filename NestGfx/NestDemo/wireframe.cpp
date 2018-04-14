#include <unistd.h>

#include "demos.h"
#include "util.h"

static struct point2 to2D(Gfx * fb, struct point3 pt3)
{
  struct point2 pt;
  float scaleRatio = (float)300/(300+pt3.z);

  pt.x = pt3.x*scaleRatio;
  pt.y = pt3.y*scaleRatio;

  pt.x += fb->getWidth()/2;
  pt.y = fb->getHeight()/2 - pt.y;

  return pt;
}

void demo_3d(Gfx * fb, int maxTime)
{
  Timer demoTime(maxTime*1000);
  demoTime.start();

  int dim = 150;
  struct point3 box[] = {{dim/2, dim/2, dim/2},
                         {dim/2, -dim/2, dim/2},
                         {-dim/2,-dim/2, dim/2},
                         {-dim/2, dim/2, dim/2},
                         {dim/2, dim/2, -dim/2},
                         {dim/2, -dim/2, -dim/2},
                         {-dim/2, -dim/2, -dim/2},
                         {-dim/2, dim/2, -dim/2}};

  int numPts = sizeof(box)/sizeof(box[0]);

  fb->drawDirty(false);
  fb->drawClear(COLOR_BLACK);


  enum cube_dir
  {
    DIR_LEFT = 0,
    DIR_BACK,
    DIR_RIGHT,
    DIR_FORWARD
  } dir;

  dir = DIR_LEFT;

  int xMod = 0;
  int zMod = 0;
  int gridMod = 0;

  Timer ani(10);
  ani.start();

  while(1)
  {
    if(demoTime.tick())
      return;

    if(ani.tick())
    {
      if(dir == DIR_LEFT)
      {
        xMod--;

        if(xMod < -100)
          dir = DIR_BACK;
      }
      if(dir == DIR_BACK)
      {
        zMod++;

        if(zMod > 300)
          dir = DIR_RIGHT;
      }
      if(dir == DIR_RIGHT)
      {
        xMod++;

        if(xMod > 100)
          dir = DIR_FORWARD;
      }
      if(dir == DIR_FORWARD)
      {
        zMod--;

        if(zMod < -100)
          dir = DIR_LEFT;
      }

      gridMod = (gridMod + 1) % 40;

      ani.restart();
    }

    // draw lines extending in to the distance
    for(int x = -fb->getWidth(); x < fb->getWidth(); x +=20)
    {
      struct point3 here, there;
      here.x = x;
      here.y = -(dim/2 -10);
      here.z = -200;

      there.x = x;
      there.y = -(dim/2 -10);
      there.z = 400;

      struct point2 a, b;
      a = to2D(fb, here);
      b = to2D(fb, there);

      fb->drawLineGrad(a.x, a.y,
          b.x, b.y, COLOR_BLUE, COLOR_BLACK);
    }

    // draw horizontal grid lines
    for(int z = -200 - gridMod; z < 400; z +=40)
    {
      struct point3 here, there;
      struct rgb_t color = fb->lerpRange(COLOR_GREEN, COLOR_BLACK, z+400, 700);
      here.x = -fb->getWidth();
      here.y = -(dim/2 -10);
      here.z = z;

      there.x = fb->getWidth()-1;
      there.y = -(dim/2 -10);
      there.z = z;

      struct point2 a, b;
      a = to2D(fb, here);
      b = to2D(fb, there);

      fb->drawLine(a.x, a.y, 
          b.x, b.y, color);
    }

    // we are drawing 4 lines per square
    // with 3 operations to draw a cube
    for(int i = 0; i <= 2; i++)
    {
      for(int j = 0; j < 4; j++)
      {
        struct point2 a, b;
        struct point3 c, d;
        int idx = j;

        if(i == 1)
          c = box[j+4];
        else
          c = box[j];

        // what is this I dont even
        if(i == 0)
          d = box[(j+1) % 4];
        else if(i == 1)
          d = box[(j+1) % 4 + 4];
        else
          d = box[j + 4];

        c.x += xMod;
        d.x += xMod;
        c.z += zMod;
        d.z += zMod;

        a = to2D(fb, c);
        b = to2D(fb, d);

        if(i == 0) // back face
          fb->drawLine(a.x, a.y, 
              b.x, b.y, COLOR_BLUE);
        else if(i == 1) // front face
          fb->drawLine(a.x, a.y, 
              b.x, b.y, COLOR_RED);
        else if(i == 2) // connecting lines
          fb->drawLine(a.x, a.y, 
              b.x, b.y, COLOR_GREEN);
      }
    }

    fb->fps();
    fb->flip();
    usleep(3000);
    fb->drawClear(COLOR_BLACK);
  }
}
