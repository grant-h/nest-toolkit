#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <fcntl.h>

#include <linux/kd.h>
#include <sys/ioctl.h>
#include <math.h>

#include "util.h"
#include "Gfx.h"
#include "demos.h"
#include "Timer.h"

struct args
{
  bool daemon;
  const char * pidFile;
};

struct scene
{
  void (*demo)(Gfx *, int);
  int time;
};

Gfx * g_fb = NULL;

void exitFunc(int sig)
{
  if(g_fb)
  {
    delete g_fb;
    g_fb = NULL;
  }

  exit(0);
}

int main(int argc, char * argv[])
{
  const char * fbPath = getenv("FRAMEBUFFER");
  const char * pidFile = "/tmp/killme.pid";

  signal(SIGINT, exitFunc);
  signal(SIGTERM, exitFunc);

  if(!fbPath)
    fbPath = "/dev/fb0";

  if(argc >= 2 && strcmp(argv[1], "debug") == 0)
    log_info("Starting in interactive mode");
  else
    background();

  if(!write_pid_file(pidFile))
  {
    log_error("Failed to write our PID file %s", pidFile);
    die();
  }

  Gfx * fb = new Gfx(fbPath);

  srand(time(NULL));

  if(!fb->attach())
    die();

  // for cleanup
  g_fb = fb;

  int sound = open("/dev/tty0", O_RDONLY);
  float t = 0.0;

/*for(int i = 0; i < 2; i++)
  {
    for(int j = 0; j < sizeof(okay); j++)
    {
      int sample = okay[j];
      ioctl(sound, KIOCSOUND, sample);
      usleep(125);
    }
    //t += 0.01;
    //ioctl(sound, KIOCSOUND, 0);
  }
  
  ioctl(sound, KIOCSOUND, 0);*/

  // clean off the crud
  fb->drawDirty(false);
  fb->drawClear(COLOR_BLACK);
  fb->flip();

  struct scene demos[] = {
    //{demo_julia, 15},
    {demo_matrix, 10},
    {demo_twist, 10},
    {demo_3d, 15},
    //{demo_fractal, 10},
    {demo_grid, 10},
    {demo_starfield, 30}
  };
#define NUM_DEMOS (sizeof(demos)/sizeof(*demos))

  // --------- DEMOS ---------
  // play these once
  //slideshow(fb, 40);
  //hello_dave(fb, 20);

  while(1)
  {
    for(int i = 0; i < NUM_DEMOS; i++)
    {
      demos[i].demo(fb, demos[i].time);
      fb->drawDirty(false);
    }
  }

  delete fb;

  return 0;
}
