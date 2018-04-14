#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#include <signal.h>

#include "demos.h"
#include "util.h"

static int readTurn(int fd)
{
  if(fd < 0)
  {
    log_error("Bad device");
    return 0;
  }

  //  seconds    u sec   unk  ---   diff
  // 051e 53ba f2b2 0005 0002 0000 0001 0000
  char buf[32];
  int res = read(fd, buf, 32);

  if(res != 32 || res < 0)
  {
    if(errno != EAGAIN)
      log_error("read dev");

    return 0;
  }

  int * diff = reinterpret_cast<int *>(buf + 12);

  return *diff;
}

static int readPress(int fd)
{
  if(fd < 0)
  {
    log_error("Bad device");
    return -1;
  }

  //  seconds    u sec   unk  ---   diff
  // 051e 53ba f2b2 0005 0002 0000 0001 0000
  char buf[32];
  int res = read(fd, buf, 32);

  if(res != 32 || res < 0)
  {
    if(errno != EAGAIN)
      log_error("read dev");

    return -1;
  }

  int * diff = reinterpret_cast<int *>(buf + 12);

  return *diff;
}

void demo_julia(Gfx * fb, int maxTime)
{
  Timer demoTime(maxTime*1000);
  demoTime.start();

  int pid = fork();

  if(pid == 0) // child
  {
    system("./run-julia.sh");
    exit(0);
  }
  else if(pid > 0)
  {
    while(!demoTime.tick())
      usleep(1000);

    kill(pid, SIGTERM);

    return;
  }
  else
  {
    log_error("Failed to fork() for julia");
    return;
  }



  int fd = open("/dev/input/event1", O_RDONLY);
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);

  int fd2 = open("/dev/input/event2", O_RDONLY);
  flags = fcntl(fd2, F_GETFL, 0);
  fcntl(fd2, F_SETFL, flags | O_NONBLOCK);

  Timer ani(2);
  ani.start();

#define MAX_ITER 50

  while(1)
  {
    //int rot = readTurn(fd);
    uint32_t yres, xres, x, y, R, G, B;
    float X, Y, Xold, Yold, dx, dy, Cx, Cy, Zx, Zy, t;
    uint8_t iter;
    
    yres = fb->getHeight();
    xres = fb->getWidth();

    dy = ((float)4) / yres;
    dx = ((float)4) / xres;
    
    for(t = 0; t < 18 * M_PI; t += M_PI / 100) {
      /*Cx = t*cos(t/8)/(9 * M_PI);
      Cy = t*sin(t/2)/(9 * M_PI);*/
      /*Cx = cos(t/4)*(1 + cos(t));
      Cy = 2*cos(2*t);*/
      Cx = -2 *exp(t/(18*M_PI)) * cos(2*t)/M_E;
      Cy = exp(t/(18*M_PI)) * sin(3*t)/M_E;

      for(Zy = 0; Zy <= 2; Zy += dy) {
        for(Zx = -2; Zx <= 2; Zx += dx) {
          X = Zx;
          Y = Zy;
          G = B = 0;
          R = 0xff;
          for(iter = 0; iter < MAX_ITER; iter++) {
            Xold = X;
            Yold = Y;
            X = Xold * Xold - Yold * Yold + Cx;
            Y = Xold * Yold + Xold * Yold + Cy;
            R -= 0x0f;
            G += 0x0f;
            if(X * X + Y * Y >= 4) break;
          }
          x = xres * Zx / 4 + 160;
          y = yres * Zy / 4 + 160;
          
          fb->drawPixelRaw(x, y,
              (iter == MAX_ITER) ? 0xff000000 : 0xff000000
                + ((R & 0xff) << 16)
                + ((G & 0xff) << 8)
                + ((B & 0xff)));
          fb->drawPixelRaw(xres -x, yres - y,
              (iter == MAX_ITER) ? 0xff000000 : 0xff000000 + 
              ((R & 0xff) << 16) +
              ((G & 0xff) << 8) +
              ((B & 0xff)));
              /* (iter == MAX_ITER) ? 0xffffffff : 0x00000000);*/
        }
      }
      fb->fps();
      fb->flip();
      fb->drawClear(COLOR_BLACK);

      if(demoTime.tick())
        return;
    }

  }
}
