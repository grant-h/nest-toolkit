#include <unistd.h>
#include <math.h>
#include <stdlib.h>

#include "Gfx.h"
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

static struct point2 to2D(Gfx * fb, int x, int y, int z)
{
  struct point2 pt;
  float scaleRatio = (float)300/(300+z);

  pt.x = x*scaleRatio;
  pt.y = y*scaleRatio;

  pt.x += fb->getWidth()/2;
  pt.y = fb->getHeight()/2 - y;

  return pt;
}

#include <fcntl.h>
#include <errno.h>
int readTurn(int fd)
{
  if(fd < 0)
  {
    log_error("Bad device");
    return 0;
  }

  //  seconds    u sec   unk  ---   diff
  // 051e 53ba f2b2 0005 0002 0000 0001 0000
  char buf[16];

  if(read(fd, buf, 16) != 16)
  {
    if(errno != EAGAIN)
      log_error("read dev");
    return 0;
  }

  int * diff = reinterpret_cast<int *>(buf + 12);

  /*if(*diff)
    log_info("diff %d", *diff);*/

  return *diff;
}

#define PI 3.1415926535f

void demo_twist(Gfx * fb, int maxTime)
{
  Timer demoTime(maxTime*1000);
  demoTime.start();

  int spacing = 1;
  int size = 20;

  rgb_t lineColors[] = {COLOR_WHITE, COLOR_RED, COLOR_GREEN, COLOR_BLUE};

  Timer ani(10);
  Timer ani2(15000);
  ani2.start();
  ani.start();
  bool doSpace = false;

  float angle = 0;
  float rotAngle = 0;

  // HACKS ON HACKS ON HACKS ON HACKS
  // fake draw ordering LOL
  // wow Im going to hell for this one
  int drawOrder[4] = {0, 1, 3, 2};
  float cosRot = cos(rotAngle + PI/2);
  float sinRot = sin(rotAngle);

  // determines the rotation limits for each of the columns
  float rotAmt = (PI/2)*(0.5*cos(angle/2) + 1.5);

  int fd = open("/dev/input/event1", O_RDONLY);
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  while(1)
  {
    // animate our twister!
    if(ani.tick())
    {
      angle += 0.05;
      //rotAngle += 0.1;
      ani.restart();

      cosRot = cos(rotAngle + PI/2);
      sinRot = sin(rotAngle);
      rotAmt = (PI/2)*(0.5*cos(angle/2) + 1.5);
    }

    int amt = readTurn(fd);
    if(amt)
    {
      if(amt <0)
        amt = -1;
      else 
        amt = 1;

      //angle += amt*0.01;

      if(spacing + amt < 1)
        spacing = 1;
      else if(spacing + amt > 60)
        spacing = 60;
      else
        spacing += amt;

      //spacing = 15 + 10*(0.5*sin(angle)-0.5) - 4;
    }

    if(ani2.tick())
      doSpace = true;

    if(demoTime.tick())
    {
      return;
    }

    for(int j = 0; j < 4; j++)
    {
      int col = drawOrder[j]; // choose which column to draw
      int nextCol = (col+1) % 4; // next column?

      /* Determines which corner of a circle we're drawing
       * Imagine a unit circle from above. We can place each column
       * on to a point of this unit circle. Depending on the column,
       * our X and Y mods will differ by a 'phase shift'. This shift
       * to match the reference angle of which quadrant the column
       * currently lies
       */
      float colMod = (PI/2)*col + PI/4;
      float nextColMod = (PI/2)*nextCol + PI/4;

      // size of our twister
      float dist = 60.0;
      bool exitNext = false;

      for(int y = -fb->getHeight()/2; ;y += spacing )
      {
        float angle1 = rotAmt*sin(angle + 0.002*y);
        float xRot = y*cosRot;
        float yRot = y*sinRot;

        int newX = dist*cosf(angle1 + colMod)+xRot;
        int newZ = dist*sinf(angle1 + colMod);

        struct point2 pt = to2D(fb, newX, y+yRot, newZ);

        // draw lines
        int calcX = dist*cosf(angle1 + nextColMod)+xRot;
        int calcZ = dist*sinf(angle1 + nextColMod);

        // fade some colors
        rgb_t color = fb->lerpRange(lineColors[col], COLOR_BLACK, pt.y, fb->getHeight());
        rgb_t color2 = fb->lerpRange(COLOR_WHITE, COLOR_BLACK, pt.y, fb->getHeight());

        // fill in the space between columns
        struct point2 connect = to2D(fb, calcX, y+yRot, calcZ); 
        fb->drawLine(pt.x, pt.y, connect.x, connect.y, color);

        // finally, draw the vertical line borders
        if(spacing == 1)
        {
          fb->drawPixel(pt.x, pt.y, color2);
        }
        else
        {
          int nextY = y+spacing;
          float angle2 = rotAmt*sin(angle + 0.002*(nextY));
          int newX2 = dist*cosf(angle2 + colMod)+(nextY)*cosRot;
          int newZ2 = dist*sinf(angle2 + colMod);

          struct point2 ptNext = to2D(fb, newX2, (nextY)+(nextY)*sinRot, newZ2);
          fb->drawLine(pt.x, pt.y, ptNext.x, ptNext.y, color2);
        }

        if(exitNext)
          break;

        if(y > (fb->getHeight()/2))
          exitNext = true;
      }
    }

    fb->flip();
    //usleep(1000);
    fb->drawClear(COLOR_BLACK);
    fb->fps();
  }
}
