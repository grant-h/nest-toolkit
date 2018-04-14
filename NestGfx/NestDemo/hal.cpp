#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include <sys/time.h>
#include <sys/types.h>

#include "demos.h"
#include "util.h"

void hello_dave(Gfx * fb, int maxTime)
{
  Timer demoTime(maxTime*1000);
  demoTime.start();

  log_info("Hello Dave");

  Timer ani(100);
  ani.start();

  int maxRad = 160;
  uint8_t *** grad = NULL;

  grad = new uint8_t**[maxRad];

  // generate all possible quarter gradients
  for(int rad = 0; rad < maxRad; rad++)
  {
    grad[rad] = new uint8_t*[rad];
    memset(grad[rad], 0, sizeof(uint8_t*)*rad);

    for(int i = 0; i < rad; i++)
    {
      grad[rad][i] = new uint8_t[rad];
      memset(grad[rad][i], 0, sizeof(uint8_t)*rad);
    }

    for(int y = 0; y < rad; y++)
    {
      for(int x = 0; x < rad; x++)
      {
        int realX = x - rad;
        int realY = y - rad;
        float dist = sqrtf((realX*realX) + (realY*realY));
        grad[rad][y][x] = 255.0*((rad - dist)/rad);
      }
    }
  }

  int rad = 0;
  int mod = 0;
  int fsm = 0;
  int time = 0;
  int blink = 0;

  while(1) 
  {
    if(ani.tick())
    {
      if(fsm == 0)
      {
        rad++;
      }
      else if(fsm == 1)
      {
        time++;
        rad =  10.0*sinf(time*3.1415926f/24.0) + maxRad-20;
      }
      else if(fsm == 2)
      {
        time += 4;
        mod = rad*sinf(time*(3.1415926*2/100.0));
      }

      if(fsm == 0 && rad+1 == maxRad-20)
        fsm = 1;
      else if(fsm == 1 && time > 100)
      {
        fsm = 2;
        time = 0;
      }
      else if(fsm == 2 && time >= 100)
      {
        if(blink == 1)
          break;

        blink++;
        fsm = 1;
      }
    }

    if(rad > 0)
    {
      for(int y = 0; y < rad; y++)
      {
        for(int x = 0; x < rad; x++)
        {
          unsigned c = grad[rad][y][x] << 16;

          int newY = min(max(y, y+mod), rad);
          fb->drawPixelRaw(x+160-rad, newY+160-rad, c); //tl
          fb->drawPixelRaw(160+rad-x-1, newY+160-rad, c); //tr
          fb->drawPixelRaw(x+160-rad, 160+rad-newY-1, c); //bl
          fb->drawPixelRaw(160+rad-x-1, 160+rad-newY-1, c); //tr
        }
      }
    }

    fb->fps();
    fb->flip();
    fb->drawClear(COLOR_BLACK);
  }

  sleep(2);
  log_info("\'hello dave!\'\n");

  // free that 1MB of memory
  for(int i = 0; i < maxRad; i++)
  {
    for(int j = 0; j < i; j++)
      delete [] grad[i][j];
    delete [] grad[i];
  }

  delete [] grad;
}
