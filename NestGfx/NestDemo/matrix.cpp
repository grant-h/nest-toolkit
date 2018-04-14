#include <complex>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>

#include "demos.h"
#include "util.h"

struct particle
{
  rgb_t color;
  float bright;
  int x, y;
  int speed;
};

void demo_matrix(Gfx * fb, int maxTime)
{
  Timer demoTime(maxTime*1000);
  demoTime.start();

  log_info("Demo matrix");

  Timer ani(50);
  ani.start();

  int tailH = 70;
  int numPart = 60;
  int pW = fb->getWidth()/numPart;
  struct particle * parts = new particle[numPart];

  for(int i = 0; i < numPart; i++)
  {
    parts[i].x = randInt(0, fb->getWidth());
    parts[i].x -= parts[i].x % pW;
    parts[i].y = randInt(-fb->getHeight(), 0);
    parts[i].bright = 1.0;
    parts[i].color = COLOR_GREEN;
    parts[i].speed = randInt(3, 8);
  }

  while(1)
  {
    if(demoTime.tick())
      return;

    if(ani.tick())
    {
      for(int i = 0; i < numPart; i++)
      {
        parts[i].y += parts[i].speed;

        if(parts[i].y-tailH> fb->getHeight())
        {
          parts[i].x = randInt(0, fb->getWidth());
          parts[i].x -= parts[i].x % pW;

          parts[i].y = -pW;
          parts[i].speed = randInt(3, 8);
        }
      }

      ani.restart();
    }

    for(int i = 0; i < numPart; i++)
    {
      for(int j = 0; j < pW; j++)
        fb->drawLineGrad(parts[i].x+j, parts[i].y-tailH, parts[i].x+j, parts[i].y+pW, COLOR_BLACK,parts[i].color);
      //fb->fillRect(i*pW, parts[i].y-j*pW, (i+1)*pW, parts[i].y+pW-j*pW, parts[i].color);
    }

    fb->fps();
    fb->flip();
    fb->drawClear(COLOR_BLACK);
  }
}
